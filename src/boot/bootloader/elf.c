#include <boot/bootloader/boot.h>
#include <elf.h>

/* elf */
uint64_t elf_load(const char* name)
{
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;
    char*       base = initram_lookup(name);
    char*       tmp;

    print("Found image ");
    print(name);
    putchar('\n');

    ehdr = (Elf64_Ehdr*)base;
    phdr = (Elf64_Phdr*)(base + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        tmp = memory_alloc(phdr[i].p_memsz);
        memset(tmp, 0, phdr[i].p_memsz);
        memcpy(tmp, base + phdr[i].p_offset, phdr[i].p_filesz);
        mmap64(tmp, phdr[i].p_vaddr, phdr[i].p_memsz);
    }

    return ehdr->e_entry;
}
