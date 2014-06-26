#include "kassert.h"
#include "screen.h"
#include "string.h"
#include "kthread.h"

unsigned long GetCurrentEFLAGS( void );
static struct ThreadQueue srunQueue = THREADQUEUEINITIALIZER;
struct KernelThread* gcurrentThread;
Boolean gneedReschedule;
Boolean gkillCurrentThread;
static struct ThreadQueue sgraveyardQueue = THREADQUEUEINITIALIZER;
static struct ThreadQueue sreaperWaitQueue = THREADQUEUEINITIALIZER;
static void InitThread( struct KernelThread* kthread, void* stackPage,
	int priority, Boolean detached )
{
    struct KernelThread* owner = detached ? 0 : gcurrentThread;
    memset( kthread, '\0', sizeof(*kthread) );
    kthread->stackPage = stackPage;
    kthread->esp = ((unsigned long) kthread->stackPage) + PAGESIZE;
    kthread->numTicks = 0;
    kthread->priority = priority;
    kthread->userContext = 0;
    kthread->owner = owner;  
    kthread->refCount = detached ? 1 : 2;
    kthread->alive = TRUE;
    MutexInit( &kthread->joinLock );
    CondInit( &kthread->joinCond );
}


static struct KernelThread* CreateThread( int priority, Boolean detached )
{
    struct KernelThread* kthread;
    void* stackPage = 0;
    DisableInterrupts();
    kthread = AllocPage();
    if ( kthread != 0 )
        stackPage = AllocPage(); 
    EnableInterrupts();
    if ( kthread == 0 )
		return 0;
    if ( stackPage == 0 ) {
		FreePageAtomic( kthread );
		return 0;
    }
    InitThread( kthread, stackPage, priority, detached );
    return kthread;
}


static void Push( struct KernelThread* kthread, unsigned long value )
{
    kthread->esp -= 4;
    *((unsigned long *) kthread->esp) = value;
}

static void DestroyThread( struct KernelThread* kthread )
{
    DetachUserContext( kthread );
    DisableInterrupts();
    FreePage( kthread->stackPage );
    FreePage( kthread );
    EnableInterrupts();
}
static void ReapThread( struct KernelThread* kthread )
{
    EnqueueThread( &sgraveyardQueue, kthread );
    WakeUp( &sreaperWaitQueue );
}
static void LaunchThread( void )
{
    EnableInterrupts();
}
static void ShutdownThread( void )
{
    struct KernelThread* runnable;
    struct KernelThread* current = gcurrentThread;
    KillThread( current );
    DisableInterrupts();
    runnable = GetNextRunnable();
    DetachThread( current ); 
    asm volatile (
	"jmp " S(RestoreThread)
	:
	: "a" (runnable) 
    );
}
static void PushGeneralRegisters( struct KernelThread* kthread )
{
   
    Push( kthread, 0 ); // eax
    Push( kthread, 0 ); // ebx
    Push( kthread, 0 ); // edx
    Push( kthread, 0 ); // edx
    Push( kthread, 0 ); // esi
    Push( kthread, 0 ); // edi
    Push( kthread, 0 ); // ebp
}

static void SetupKernelThread(
    struct KernelThread* kthread,
    ThreadStartFunc startFunc,
    unsigned long arg )
{
    unsigned long eflags;
    Push( kthread, arg );
    Push( kthread, (unsigned long) &ShutdownThread );
    Push( kthread, (unsigned long) startFunc );
    eflags = GetCurrentEFLAGS();
    eflags &= ~(EFLAGSIF);
    Push( kthread, eflags );
    Push( kthread, KERNELCS );
    Push( kthread, (unsigned long) &LaunchThread );
    Push( kthread, 0 );
    Push( kthread, 0 );
    PushGeneralRegisters( kthread );
    Push( kthread, KERNELDS ); // ds
    Push( kthread, KERNELDS ); // es
    Push( kthread, 0 ); // fs
    Push( kthread, 0 ); // gs
}

static void SetupUserThread(
    struct KernelThread* kthread, struct UserContext* userContext,
    unsigned long entryAddr )
{
    unsigned long eflags = GetCurrentEFLAGS() | EFLAGSIF;
    unsigned csSelector = userContext->csSelector;
    unsigned dsSelector = userContext->dsSelector;
    AttachUserContext( kthread, userContext );
    Push( kthread, dsSelector );
    Push( kthread, userContext->size );
    Push( kthread, eflags );
    Push( kthread, csSelector );
    Push( kthread, entryAddr );
    Push( kthread, 0 );
    Push( kthread, 0 );
    PushGeneralRegisters( kthread );
    Push( kthread, dsSelector ); // ds
    Push( kthread, dsSelector ); // es
    Push( kthread, dsSelector ); // fs
    Push( kthread, dsSelector ); // gs
}

static void Idle( unsigned long arg )
{
    while ( TRUE )
	Yield();
}

static void Reaper( unsigned long arg )
{
    struct KernelThread *kthread;
    DisableInterrupts();
    while ( TRUE ) {
	if ( (kthread = sgraveyardQueue.head) == 0 ) {
	    Wait( &sreaperWaitQueue );
	}
	else {	 
	    ClearThreadQueue( &sgraveyardQueue );
	    EnableInterrupts();
	    Yield();  
	    while ( kthread != 0 ) {
		struct KernelThread* next = NODENEXT(ThreadQueue, kthread);
#if 0
		Print( "Reaper: disposing of thread @ %x, stack @ %x\n",
		    kthread, kthread->stackPage );
#endif
		DestroyThread( kthread );
		kthread = next;
	    }	 
	    DisableInterrupts();
	}
    }
}

static inline struct KernelThread* FindBest( struct ThreadQueue* queue )
{
    struct KernelThread *kthread = queue->head, *best = 0;
    while ( kthread != 0 ) {
	if ( best == 0 || kthread->priority > best->priority )
	    best = kthread;
	kthread = NODENEXT(ThreadQueue, kthread);
    }
    return best;
}

void InitScheduler( void )
{
    struct KernelThread* mainThread = (struct KernelThread *) KERNTHREADOBJ;
    KASSERT( IsThreadQueueEmpty( &srunQueue ) );
    InitThread( mainThread, (void *) KERNSTACK, PRIORITYNORMAL, TRUE );
    gcurrentThread = mainThread;
    StartKernelThread( Idle, 0, PRIORITYIDLE, TRUE );
    StartKernelThread( Reaper, 0, PRIORITYNORMAL, TRUE );
}

struct KernelThread* StartKernelThread(
    ThreadStartFunc startFunc,
    unsigned long arg,
    int priority,
    Boolean detached
)
{
    struct KernelThread* kthread = CreateThread( priority, detached );
    if ( kthread != 0 ) {
	SetupKernelThread( kthread, startFunc, arg );
	MakeRunnableAtomic( kthread );
    }
    return kthread;
}

struct KernelThread* StartUserThread(
    struct UserContext* userContext,
    unsigned long entryAddr,
    Boolean detached
)
{
    struct KernelThread* kthread = CreateThread( PRIORITYUSER, detached );
    if ( kthread != 0 ) {
	SetupUserThread( kthread, userContext, entryAddr );
	MakeRunnableAtomic( kthread );
    }
    return kthread;
}

void MakeRunnable( struct KernelThread* kthread )
{
    KASSERT( !InterruptsEnabled() );
    EnqueueThread( &srunQueue, kthread );
}

void MakeRunnableAtomic( struct KernelThread* kthread )
{
    DisableInterrupts();
    MakeRunnable( kthread );
    EnableInterrupts();
}

struct KernelThread* GetCurrent( void )
{
    return gcurrentThread;
}

struct KernelThread* GetNextRunnable( void )
{
    struct KernelThread* best = FindBest( &srunQueue );
    KASSERT( best != 0 );
    RemoveThread( &srunQueue, best );
    return best;
}


void Schedule( void )
{
    struct KernelThread* runnable;
    KASSERT( !InterruptsEnabled() );
    runnable = GetNextRunnable();
    SwitchToThread( runnable );
}

void Yield( void )
{
    DisableInterrupts();
    MakeRunnable( gcurrentThread );
    Schedule();
    EnableInterrupts();
}

void Exit( void )
{
    ShutdownThread();
}


void KillThread( struct KernelThread* kthread )
{
    KASSERT( InterruptsEnabled() );

    if ( kthread->owner != 0 ) {
	MutexLock( &kthread->joinLock );
	kthread->alive = FALSE;
	CondBroadcast( &kthread->joinCond );
	MutexUnlock( &kthread->joinLock );
    }
}

void DetachThread( struct KernelThread* kthread )
{
    KASSERT( !InterruptsEnabled() );
    KASSERT( kthread->refCount > 0 );

    --kthread->refCount;
    if ( kthread->refCount == 0 ) {
	ReapThread( kthread );
    }
}

void Join( struct KernelThread* kthread )
{
    KASSERT( kthread->owner == gcurrentThread );

    MutexLock( &kthread->joinLock );
    while ( kthread->alive ) {
	CondWait( &kthread->joinCond, &kthread->joinLock );
    }
    MutexUnlock( &kthread->joinLock );
    DisableInterrupts();
    DetachThread( kthread );
    EnableInterrupts();
}

void Wait( struct ThreadQueue* waitQueue )
{
    struct KernelThread* current = gcurrentThread;
    KASSERT( !InterruptsEnabled() );
    EnqueueThread( waitQueue, current );
    Schedule();
}

void WakeUp( struct ThreadQueue* waitQueue )
{
    struct KernelThread *kthread = waitQueue->head, *next;
    KASSERT( !InterruptsEnabled() );
    while ( kthread != 0 ) {
	next = NODENEXT(ThreadQueue, kthread);
	MakeRunnable( kthread );
	kthread = next;
    }

    ClearThreadQueue( waitQueue );
}

void WakeUpOne( struct ThreadQueue* waitQueue )
{
    struct KernelThread* best;
    KASSERT( !InterruptsEnabled() );
    best = FindBest( waitQueue );
    if ( best != 0 ) {
	RemoveThread( waitQueue, best );
	MakeRunnable( best );
    }
}
