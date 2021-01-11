#include <sys/irq.h>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    irq_wait(0x01);

    for (;;) {}

    return 0;
}
