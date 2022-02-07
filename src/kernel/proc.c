#include <kernel/proc.h>
#include <kernel/kernel.h>
#include <kernel/common/array.h>
#include <libboot/libboot.h>
#include <elf.h>

static Process gProcessList[1024];
static size_t  gProcessListSize;
static Array   gProcessListFree;

static Array    gScheduledProcs[2];
static int      gScheduledProcsFront;
static int      gScheduledProcsIndex;

void proc_init(void)
{
    gProcessListSize = 0;

    ARRAY_INIT(&gProcessListFree, size_t, 16);

    ARRAY_INIT(&gScheduledProcs[0], Process*, 16);
    ARRAY_INIT(&gScheduledProcs[1], Process*, 16);

    gScheduledProcsFront = 0;
    gScheduledProcsIndex = 0;
}

static void* proc_alloc(Process* proc, void* addr, uint64_t size, int prot)
{
    ASM("mov %0, %%cr3\r\n" :: "rax"(proc->addr_space.cr3) : "memory");
    kmapanon(addr, size, KPROT_USER | prot);
    return addr;
}

static void load_elf(Process* proc, const char* image)
{
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;
    size_t rambase = 0x10000000;

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
    size_t stack_size = 0x10000;
    proc_alloc(proc, stack, stack_size, KPROT_READ | KPROT_WRITE);
    memset(stack, 0, stack_size);

    proc->regs[X86_REG_SP] = (uint64_t)(stack + stack_size);
    proc->regs[X86_REG_FLAGS] = getflags() | X86_FLAG_IF;
    proc->regs[X86_REG_IP] = ehdr->e_entry + rambase;

    BREAKPOINT;
}

static uint64_t create_cr3()
{
    uint64_t    cr3_kernel;
    uint64_t    cr3_user;
    uint64_t*   cr3_kernel_ptr;
    uint64_t*   cr3_user_ptr;

    ASM("mov %%cr3, %0\r\n" : "=a"(cr3_kernel) :: "memory");
    cr3_user = alloc_phys(PAGESIZE);

    cr3_kernel_ptr = (uint64_t*)physical_to_virtual(cr3_kernel);
    cr3_user_ptr = (uint64_t*)physical_to_virtual(cr3_user);

    memset(cr3_user_ptr, 0, 256 * 8);
    memcpy(cr3_user_ptr + 256, cr3_kernel_ptr + 256, 256 * 8);

    return cr3_user;
}

Process* proc_create(void)
{
    Process*        proc;
    size_t          proc_id;

    if (ARRAY_EMPTY(&gProcessListFree))
    {
        proc_id = gProcessListSize++;
    }
    else
    {
        proc_id = *ARRAY_BACK(&gProcessListFree, size_t);
        ARRAY_POP(&gProcessListFree);
    }
    proc = gProcessList + proc_id;

    /* Create a blank process */
    memset(proc, 0, sizeof(*proc));

    /* Setup proper context switching */
    proc->run = &proc_run;
    proc->addr_space.cr3 = create_cr3();

    return proc;
}

Process* proc_create_initram(const char* path)
{
    Process* proc;
    const char* image;

    image = initram_lookup(path);
    if (image)
    {
        proc = proc_create();
        load_elf(proc, image);
        return proc;
    }
    return NULL;
}

void proc_schedule(Process* proc)
{
    *ARRAY_EMPLACE(&gScheduledProcs[1 - gScheduledProcsFront], Process*) = proc;
}

void proc_reschedule(void)
{
    proc_schedule(gKernel.threads[0].proc);
}

static Process* next_proc(void)
{
    /* If there are still processes in the front buffer, then run the next one */
    if (gScheduledProcsIndex < gScheduledProcs[gScheduledProcsFront].size)
        return *ARRAY_AT(&gScheduledProcs[gScheduledProcsFront], Process*, gScheduledProcsIndex++);

    /* The front buffer is fully processed. We need to swap buffers */
    gScheduledProcs[gScheduledProcsFront].size = 0;
    gScheduledProcsFront = 1 - gScheduledProcsFront;
    gScheduledProcsIndex = 0;

    /* Were there processes in the backbuffer? */
    if (!(ARRAY_EMPTY(&gScheduledProcs[gScheduledProcsFront])))
        return *ARRAY_AT(&gScheduledProcs[gScheduledProcsFront], Process*, gScheduledProcsIndex++);

    /* Every process is blocked */
    return NULL;
}

static void _NORETURN sched_exec(Process* proc)
{
    ASM("mov %0, %%cr3\r\n" :: "rax"(proc->addr_space.cr3) : "memory");
    gKernel.threads[0].proc = proc;
    timer_ns(100000);
    proc->run(proc);
    _UNREACHABLE();
}

static void _NORETURN sched_wait(void)
{
    gKernel.threads[0].proc = NULL;
    ASM (
        "sti\r\n"
        "1: hlt\r\n"
        "jmp 1b\r\n"
    );
    _UNREACHABLE();
}

void _NORETURN kern_schedule(void)
{
    Process* proc;

    proc = next_proc();
    if (proc != NULL)
        sched_exec(proc);
    else
        sched_wait();
}
