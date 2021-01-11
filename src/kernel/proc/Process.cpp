#include <kernel/proc/Process.h>
#include <kernel/kernel.h>
#include <libboot/libboot.h>
#include <elf.h>
#include <new>

static void load_elf(Process* proc, const char* image)
{
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;
    std::size_t rambase = 0x10000000;

    kprintf("Found process: 0x%lx\n", (uint64_t)image);

    ehdr = (Elf64_Ehdr*)image;
    phdr = (Elf64_Phdr*)(image + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        char* base = (char*)(phdr[i].p_vaddr + rambase);
        proc_alloc(proc, base, phdr[i].p_memsz, KPROT_READ | KPROT_WRITE | KPROT_EXECUTE);
        memset(base, 0, phdr[i].p_memsz);
        memcpy(base, image + phdr[i].p_offset, phdr[i].p_filesz);
    }

    char* stack = (char*)(0x200000ull);
    std::size_t stack_size = 0x10000;
    proc_alloc(proc, stack, stack_size, KPROT_READ | KPROT_WRITE);
    memset(stack, 0, stack_size);

    proc->regs[X86_REG_SP] = (uint64_t)(stack + stack_size);
    proc->regs[X86_REG_FLAGS] = getflags() | X86_FLAG_IF;
    proc->regs[X86_REG_IP] = ehdr->e_entry + rambase;
}

static uint64_t create_cr3()
{
    uint64_t    cr3_kernel;
    uint64_t    cr3_user;
    uint64_t*   cr3_kernel_ptr;
    uint64_t*   cr3_user_ptr;

    ASM("mov %%cr3, %0\r\n" : "=a"(cr3_kernel));
    cr3_user = alloc_phys(PAGESIZE);

    cr3_kernel_ptr = (uint64_t*)physical_to_virtual(cr3_kernel);
    cr3_user_ptr = (uint64_t*)physical_to_virtual(cr3_user);

    memset(cr3_user_ptr, 0, 256 * 8);
    memcpy(cr3_user_ptr + 256, cr3_kernel_ptr + 256, 256 * 8);

    return cr3_user;
}

Process* proc_create(void)
{
    Process*    proc;

    /* Create a blank process */
    proc = (Process*)kmalloc(sizeof(Process));
    memset(proc, 0, sizeof(proc));
    new (proc) Process;

    /* Setup proper context switching */
    proc->run = &proc_run;
    proc->addr_space.cr3 = create_cr3();

    return proc;
}

Process* proc_create_initram(const char* path)
{
    Process* proc{};
    const char* image;

    image = initram_lookup(path);
    if (image)
    {
        proc = proc_create();
        load_elf(proc, image);
    }
    return proc;
}

void* proc_alloc(Process* proc, void* addr, uint64_t size, int prot)
{
    ASM("mov %0, %%cr3\r\n" :: "rax"(proc->addr_space.cr3));
    kmmap(addr, 0, size, KPROT_USER | prot, KMAP_FIXED | KMAP_ANONYMOUS);
    return addr;
}

void proc_exec(Process* proc)
{
    ASM("mov %0, %%cr3\r\n" :: "rax"(proc->addr_space.cr3));
    gKernel.threads[0].proc = proc;
    timer_ns(100000);
    proc->run(proc);
}
