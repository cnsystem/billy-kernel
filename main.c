#include "bootinfo.h"
#include "string.h"
#include "screen.h"
#include "tss.h"
#include "kthread.h"
#include "keyboard.h"

static void InitBSS( void );
void Main( struct BootInfo* bootInfo )
{
    InitBSS();
    InitScreen();
    InitMem( bootInfo );
    InitTSS();
    InitInterrupts();
    InitScheduler();
    InitTraps();
    InitTimer();
    InitKeyboard();
    SetCurrentAttr( ATTRIB(BLACK, CYAN|BRIGHT) );
    Print( "Welcome to billynix!\n" );    
}
static void InitBSS( void )
{
    char *bssStart, *bssEnd;
#if defined(GNUWIN32)
    extern char bssstart;
    extern char bssend;
    bssStart = &bssstart;
    bssEnd = &bssend;
#else
    extern char bssstart;
    extern char end;
    bssStart = &bssstart;
    bssEnd = &end;
#endif
    memset( bssStart, '\0', bssEnd - bssStart );
}
