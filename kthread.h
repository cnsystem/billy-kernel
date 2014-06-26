#ifndef KTHREADH
#define KTHREADH

#include "ktypes.h"

struct UserContext;
struct InterruptState;
struct KernelThread {
    unsigned long esp;                            //堆栈指针		
    volatile unsigned long numTicks;              //	
    int priority;                                 //优先级
    DEFINELINK( ThreadQueue, KernelThread );     //队列
    void* stackPage;
    struct UserContext* userContext;
    struct KernelThread* owner;
    int refCount;   
    Boolean alive;
    struct Mutex joinLock;
    struct Condition joinCond;
};
typedef void (*ThreadStartFunc)( unsigned long arg );

#define PRIORITYIDLE    0
#define PRIORITYUSER    1
#define PRIORITYLOW     2
#define PRIORITYNORMAL  5
#define PRIORITYHIGH   10
//初始化调度器
void InitScheduler( void );
//创建内核线程
struct KernelThread* StartKernelThread(ThreadStartFunc startFunc, unsigned long arg, int priority, Boolean detached);
//创建用户线程
struct KernelThread* StartUserThread(struct UserContext* userContext, unsigned long entryAddr, Boolean detached);
//
void MakeRunnable( struct KernelThread* kthread );
//
void MakeRunnableAtomic( struct KernelThread* kthread );
//
struct KernelThread* GetCurrent( void );
//
struct KernelThread* GetNextRunnable( void );
//调度
void Schedule( void );
void Yield( void );
void Exit( void );
void KillThread( struct KernelThread* kthread );
void DetachThread( struct KernelThread* kthread );
void Join( struct KernelThread* kthread );

void SwitchToThread( struct KernelThread* );
void RestoreThread( void );

void WakeUp( struct ThreadQueue* waitQueue );
void WakeUpOne( struct ThreadQueue* waitQueue );


extern struct KernelThread* gcurrentThread;

extern Boolean gneedReschedule;

extern Boolean gkillCurrentThread;

#endif // KTHREADH
