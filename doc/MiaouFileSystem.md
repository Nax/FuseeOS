# The Miaou File System

FuseeOS uses the Miaou File System.

MFS works with pages.
A page is a block of 4096 bytes.
The first 64 pages of a disk are reserved: the first 63 pages are reserved for the VBR, and the last reserved page is there to store filesystem metadata.

