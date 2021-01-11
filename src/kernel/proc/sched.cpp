#ifndef KERNEL_PROC_SCHED_H
#define KERNEL_PROC_SCHED_H 1

#include <kernel/collections/Array.h>
#include <kernel/proc/Process.h>

static Array<Process*> gScheduledProcs;
static Array<Process*> gScheduledProcsIO;

void proc_schedule(Process* proc)
{
    gScheduledProcs.push_front(proc);
}

void proc_schedule_io(Process* proc)
{
    gScheduledProcsIO.push_front(proc);
}

static Process* next_proc(Array<Process*>& procs)
{
    Process* p;

    if (procs.empty())
        return nullptr;
    p = procs.back();
    procs.pop_back();
    return p;
}

void proc_run_next()
{
    Process* p;

    p = next_proc(gScheduledProcsIO);
    if (!p)
        p = next_proc(gScheduledProcs);
    proc_exec(p);
}

#endif
