

#include "kthread.h"
#include "screen.h"
#include "irq.h"
#include "io.h"
#include "keyboard.h"

#define LEFTSHIFT  0x01
#define RIGHTSHIFT 0x02
#define LEFTCTRL   0x04
#define RIGHTCTRL  0x08
#define LEFTALT    0x10
#define RIGHTALT   0x20
#define SHIFTMASK  (LEFTSHIFT | RIGHTSHIFT)
#define CTRLMASK   (LEFTCTRL | RIGHTCTRL)
#define ALTMASK    (LEFTALT | RIGHTALT)
static unsigned sshiftState = 0;


#define KBSIZE 256
#define NEXT(x) (((x) + 1) % KBSIZE)
typedef struct keyboardcache{
	Keycode* head;
	Keycode* tail;
	int count;
	Keycode buf[KBSIZE];
}kbcache;
kbcache kb;
static struct ThreadQueue swaitQueue = THREADQUEUEINITIALIZER;
static const Keycode sscanTableNoShift[] = {
    KEYUNKNOWN, ASCIIESC, '1', '2',  // 0x00 - 0x03
    '3', '4', '5', '6',                // 0x04 - 0x07
    '7', '8', '9', '0',                // 0x08 - 0x0B
    '-', '=', ASCIIBS, '\t',          // 0x0C - 0x0F
    'q', 'w', 'e', 'r',                // 0x10 - 0x13
    't', 'y', 'u', 'i',                // 0x14 - 0x17
    'o', 'p', '[', ']',                // 0x18 - 0x1B
    '\r', KEYLCTRL, 'a', 's',         // 0x1C - 0x1F
    'd', 'f', 'g', 'h',                // 0x20 - 0x23
    'j', 'k', 'l', ';',                // 0x24 - 0x27
    '\'', '`', KEYLSHIFT, '\\',       // 0x28 - 0x2B
    'z', 'x', 'c', 'v',                // 0x2C - 0x2F
    'b', 'n', 'm', ',',                // 0x30 - 0x33
    '.', '/', KEYRSHIFT, KEYPRINTSCRN,// 0x34 - 0x37
    KEYLALT, ' ', KEYCAPSLOCK, KEYF1,// 0x38 - 0x3B
    KEYF2, KEYF3, KEYF4, KEYF5,    // 0x3C - 0x3F
    KEYF6, KEYF7, KEYF8, KEYF9,    // 0x40 - 0x43
    KEYF10, KEYNUMLOCK, KEYSCRLOCK, KEYKPHOME, // 0x44 - 0x47
    KEYKPUP, KEYKPPGUP, KEYKPMINUS, KEYKPLEFT, // 0x48 - 0x4B
    KEYKPCENTER, KEYKPRIGHT, KEYKPPLUS, KEYKPEND, // 0x4C - 0x4F
    KEYKPDOWN, KEYKPPGDN, KEYKPINSERT, KEYKPDEL, // 0x50 - 0x53
    KEYSYSREQ, KEYUNKNOWN, KEYUNKNOWN, KEYUNKNOWN, // 0x54 - 0x57
};
#define SCANTABLESIZE (sizeof(sscanTableNoShift) / sizeof(Keycode))
static const Keycode sscanTableWithShift[] = {
    KEYUNKNOWN, ASCIIESC, '!', '@',  // 0x00 - 0x03
    '#', '$', '%', '^',                // 0x04 - 0x07
    '&', '*', '(', ')',                // 0x08 - 0x0B
    '', '+', ASCIIBS, '\t',          // 0x0C - 0x0F
    'Q', 'W', 'E', 'R',                // 0x10 - 0x13
    'T', 'Y', 'U', 'I',                // 0x14 - 0x17
    'O', 'P', '{', '}',                // 0x18 - 0x1B
    '\r', KEYLCTRL, 'A', 'S',         // 0x1C - 0x1F
    'D', 'F', 'G', 'H',                // 0x20 - 0x23
    'J', 'K', 'L', ':',                // 0x24 - 0x27
    '"', '~', KEYLSHIFT, '|',         // 0x28 - 0x2B
    'Z', 'X', 'C', 'V',                // 0x2C - 0x2F
    'B', 'N', 'M', '<',                // 0x30 - 0x33
    '>', '?', KEYRSHIFT, KEYPRINTSCRN,// 0x34 - 0x37
    KEYLALT, ' ', KEYCAPSLOCK, KEYF1,// 0x38 - 0x3B
    KEYF2, KEYF3, KEYF4, KEYF5,    // 0x3C - 0x3F
    KEYF6, KEYF7, KEYF8, KEYF9,    // 0x40 - 0x43
    KEYF10, KEYNUMLOCK, KEYSCRLOCK, KEYKPHOME, // 0x44 - 0x47
    KEYKPUP, KEYKPPGUP, KEYKPMINUS, KEYKPLEFT, // 0x48 - 0x4B
    KEYKPCENTER, KEYKPRIGHT, KEYKPPLUS, KEYKPEND, // 0x4C - 0x4F
    KEYKPDOWN, KEYKPPGDN, KEYKPINSERT, KEYKPDEL, // 0x50 - 0x53
    KEYSYSREQ, KEYUNKNOWN, KEYUNKNOWN, KEYUNKNOWN, // 0x54 - 0x57
};

static Boolean IsQueueEmpty( void )
{
    return kb.head==kb.tail;
}

static Boolean IsQueueFull( void )
{
    return NEXT(kb.tail) == kb.head;
}

static void EnqueueKeycode( Keycode keycode )
{
    if ( !IsQueueFull() ) {
	kb.buf[ kb.tail ] = keycode;
	kb.tail = NEXT(kb.tail);
    }
}

static Keycode DequeueKeycode()
{
    Keycode key;
    KASSERT( !IsQueueEmpty() );
    key = kb.buf[kb.head];
    kb.head = NEXT(kb.head);
    return key;
}
static void kb_Handler(struct InterruptState* state)
{
    uchart status, scanCode;
    unsigned flag = 0;
    bool release = false, shift;
    Keycode keycode;
    BeginIRQ(state);	
    status = InByte(KBCMD);
    IODelay();
    if ((status & KBOUTPUTFULL) != 0) {
		scanCode = InByte(KBDATA);
		IODelay();
		if (scanCode & KBKEYRELEASE) {
			release = true;
			scanCode &= ~(KBKEYRELEASE);
		}
		if (scanCode >= SCANTABLESIZE) {
			Print("Î´ÖªÉ¨ÃèÂë: %x\n", scanCode);
			EndIRQ(state);
			return ;
		}
		shift = ((sshiftState & SHIFTMASK) != 0);
		keycode = shift ? sscanTableWithShift[scanCode] : sscanTableNoShift[scanCode];
		switch (keycode) {
		case KEYLSHIFT:
			flag = LEFTSHIFT;
			break;
		case KEYRSHIFT:
			flag = RIGHTSHIFT;
			break;
		case KEYLCTRL:
			flag = LEFTCTRL;
			break;
		case KEYRCTRL:
			flag = RIGHTCTRL;
			break;
		case KEYLALT:
			flag = LEFTALT;
			break;
		case KEYRALT:
			flag = RIGHTALT;
			break;
		default:
			break;
		}
		if(flag!=0)
		{
			if (release)
				sshiftState &= ~(flag);
			else
				sshiftState |= flag;
		}
		else{
			if (shift)
				keycode |= KEYSHIFTFLAG;
			if ((sshiftState & CTRLMASK) != 0)
				keycode |= KEYCTRLFLAG;
			if ((sshiftState & ALTMASK) != 0)
				keycode |= KEYALTFLAG;
			if (release)
				keycode |= KEYRELEASEFLAG;		
			EnqueueKeycode(keycode);
			WakeUp(&swaitQueue);
			gneedReschedule = true;
		}
    }
    EndIRQ(state);
}

void InitKeyboard()
{
    unsigned short irqMask;

    Print( "³õÊ¼»¯¼üÅÌ...\n" );
    sshiftState = 0;
    kb.head = kb.tail = 0;
    InstallIRQ( KBIRQ, kb_Handler );
    irqMask = GetIRQMask();
    irqMask &= ~(1 << KBIRQ);
    SetIRQMask( irqMask );
}

Boolean ReadKey( Keycode* key )
{
    Boolean result;
    DisableInterrupts();
    result = !IsQueueEmpty();
    if ( result ) {
	*key = DequeueKeycode();
    }
    EnableInterrupts();
    return result;
}
Keycode WaitForKeyAtomic()
{
    Keycode key;
    DisableInterrupts();
    key = WaitForKey();
    EnableInterrupts();
    return key;
}
Keycode WaitForKey()
{
    Boolean gotKey = FALSE;
    Keycode key = KEYUNKNOWN;
    do {
	gotKey = !IsQueueEmpty();
	if ( gotKey )
	    key = DequeueKeycode();
	else
	    Wait( &swaitQueue );
    }
    while ( !gotKey );
    return key;
}
