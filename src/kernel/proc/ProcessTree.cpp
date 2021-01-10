#include <kernel/proc/ProcessTree.h>

Process& ProcessTree::create()
{
    _procs.push(Process());
    return _procs.back();
}
