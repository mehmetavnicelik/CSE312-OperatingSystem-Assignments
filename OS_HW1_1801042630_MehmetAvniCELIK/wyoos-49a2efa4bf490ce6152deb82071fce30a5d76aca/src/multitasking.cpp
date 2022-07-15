
#include <multitasking.h>

using namespace myos;
using namespace myos::common;


Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */
    
    // cpustate -> error = 0;    
   
    // cpustate -> esp = ;
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;
    isEnded=false;
    isOvertime=false;
    counter=0;
}

Task::~Task()
{
}

        
TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}
bool TaskManager::isAvailable=true;
bool TaskManager::CreateThread(Task* task)
{
    if(numTasks >= 256)
        return false;
    if(isAvailable==true){          //if there is not any process in the queue,thread will be created.
        tasks[numTasks++] = task;
        task->counter++;
    }
    return true;
}
bool TaskManager::TerminateThread(Task* task){
    if(numTasks < 0)
        return false;
    for(int i=0;i<numTasks;i++){
        if(tasks[i]==task){
            task->isEnded=true;
            TaskManager:: isAvailable=true;     //to make sure the thread is ended and some other thread can work
            while(i!=numTasks-1){
                tasks[i]=tasks[i+1];
                i++;
            }
            numTasks--;
            break;
        }
    }
    return true;
}
bool TaskManager::YieldThread(Task* task){ // thread that works for a long time pushed to the end of the queue without terminating
        tasks[numTasks]=task;
        int i=0;
        for(int i=0;i<numTasks;i++){
            if(tasks[i]==task){
                while(i!=numTasks){
                    tasks[i]=tasks[i+1];
                    i++;
                }
                break;
            }
        }
        task->isOvertime=false;
        return true;
}

bool TaskManager::JoinThread(Task* task,int joinValue){ // make sure it is terminated.If it is not terminated,next threads wont be exectued.
    if(task->isEnded==false)
        TaskManager:: isAvailable=false;

    else if(task->isEnded==true)
        TaskManager:: isAvailable=true;

    else isAvailable=false;

    return isAvailable;
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if(numTasks <= 0)
        return cpustate;
    
    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;
    
    if(++currentTask >= numTasks)
        currentTask %= numTasks;
    return tasks[currentTask]->cpustate;
}

    