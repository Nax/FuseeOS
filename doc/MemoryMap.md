# Kernel memory map

## Virtual Address Space

0x0000000000000000 - Userland
0xffff800000000000 - Physical Map
0xffffa00000000000 - Kernel Memory (Backed)
0xffffc00000000000 - Kernel Memory (IO)
0xffffffff00000000 - Kernel InitRD
0xffffffff80000000 - Kernel Image

Since virtual memory is orders and orders of magnitude larger
than physical memory will ever be, we can simply map all of it
at a fixed address for the kernel to use.

## Physical Address Space

For the bootloader:
  * <16 MiB:  Temporary space, free to use. Kernel can trash it upon boot, no need to keep track of it.
  * >= 16MiB: Permanent space. Need to keep track of it. Actual kernel pages need to come from this.
