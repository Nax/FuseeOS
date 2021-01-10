#ifndef KERNEL_PROC_PROCESS_TREE_H
#define KERNEL_PROC_PROCESS_TREE_H 1

#include <kernel/collections/Array.h>
#include <kernel/proc/Process.h>

class ProcessTree
{
public:
    Process& create();
    Process& next();

private:
    Array<Process>  _procs;
};

#endif
