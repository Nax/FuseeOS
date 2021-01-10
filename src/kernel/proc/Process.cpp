#include <kernel/proc/Process.h>
#include <kernel/proc/ProcessTree.h>
#include <kernel/kernel.h>
#include <libboot/libboot.h>
#include <elf.h>

static void load_elf(Process& proc, const char* image)
{
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;

    kprintf("Found process: 0x%lx\n", (uint64_t)image);

    ehdr = (Elf64_Ehdr*)image;
    phdr = (Elf64_Phdr*)(image + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        char* base = (char*)((i + 1) * 0x10000000ull);
        kmmap(base, 0, phdr[i].p_memsz, KPROT_USER | KPROT_READ | KPROT_WRITE | KPROT_EXECUTE, KMAP_ANONYMOUS | KMAP_FIXED);
        memset(base, 0, phdr[i].p_memsz);
        memcpy(base, image + phdr[i].p_offset, phdr[i].p_filesz);
    }

    proc.regs[X86_REG_IP] = ehdr->e_entry;
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
