.section .text
.extern main

.global _start
_start:
    call main
    ud2
