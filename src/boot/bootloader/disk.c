#include <boot/bootloader/boot.h>

void disk_read_raw(char* dst, uint64_t lba, uint32_t sectors)
{
    BiosArgs         args;
    DiskAccessPacket dap;
    char             buffer[4096];
    uint32_t         nsectors;

    dap.size        = sizeof(DiskAccessPacket);
    dap.zero        = 0;
    dap.dst_offset  = (uintptr_t)buffer;
    dap.dst_segment = 0;

    lba += gBootParams.mbr_partition.lba_start;

    while (sectors)
    {
        dap.lba_lo        = lba & 0xffffffff;
        dap.lba_hi        = (lba >> 32);
        nsectors          = (sectors > 8) ? 8 : sectors;
        dap.transfer_size = nsectors;

        args.eax = 0x4200;
        args.edx = gBootParams.boot_drive;
        args.esi = (uintptr_t)&dap;
        bios_call(0x13, &args);

        memcpy(dst, buffer, nsectors * 512);
        dst += nsectors * 512;
        lba += nsectors;
        sectors -= nsectors;
    }
}
