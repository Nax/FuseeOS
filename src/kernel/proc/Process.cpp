#include <kernel/proc/Process.h>
#include <kernel/proc/ProcessTree.h>
#include <kernel/kernel.h>
#include <libboot/libboot.h>
#include <elf.h>

extern "C" void _exec_proc(Process* proc);

static void load_elf(Process& proc, const char* image)
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
        kmmap(base, 0, phdr[i].p_memsz, KPROT_USER | KPROT_READ | KPROT_WRITE | KPROT_EXECUTE, KMAP_ANONYMOUS | KMAP_FIXED);
        memset(base, 0, phdr[i].p_memsz);
        memcpy(base, image + phdr[i].p_offset, phdr[i].p_filesz);
    }

    char* stack = (char*)(0x200000ull);
    std::size_t stack_size = 0x10000;
    kmmap(stack, 0, stack_size, KPROT_USER | KPROT_READ | KPROT_WRITE, KMAP_ANONYMOUS | KMAP_FIXED);

    proc.regs[X86_REG_SP] = (uint64_t)(stack + stack_size);
    proc.regs[X86_REG_FLAGS] = getflags() | X86_FLAG_IF;
    proc.regs[X86_REG_IP] = ehdr->e_entry + rambase;

    exec_proc(proc);
}

void load_proc_initram(const char* path)
{
    const char* image;

    image = initram_lookup(path);
    if (image)
    {
        Process& proc = gKernel.procs.create();
        load_elf(proc, image);
    }
}

void exec_proc(Process& proc)
{
    gKernel.threads[0].proc = &proc;
    timer_ns(100000);
    proc.run(&proc);
}
