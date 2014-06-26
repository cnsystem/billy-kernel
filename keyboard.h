#ifndef KEYBOARDH
#define KEYBOARDH

#include "ktypes.h"
#define KBIRQ 1
#define KBCMD 0x64
#define KBDATA 0x60
#define KBOUTPUTFULL 0x01
#define KBKEYRELEASE 0x80
typedef unsigned short Keycode;
#define KEYSPECIALFLAG 0x0100
#define KEYKEYPADFLAG  0x0200
#define KEYSHIFTFLAG   0x1000
#define KEYALTFLAG     0x2000
#define KEYCTRLFLAG    0x4000
#define KEYRELEASEFLAG 0x8000
#define SPECIAL(num) (KEYSPECIALFLAG | (num))
#define KEYUNKNOWN SPECIAL(0)
#define KEYF1      SPECIAL(1)
#define KEYF2      SPECIAL(2)
#define KEYF3      SPECIAL(3)
#define KEYF4      SPECIAL(4)
#define KEYF5      SPECIAL(5)
#define KEYF6      SPECIAL(6)
#define KEYF7      SPECIAL(7)
#define KEYF8      SPECIAL(8)
#define KEYF9      SPECIAL(9)
#define KEYF10     SPECIAL(10)
#define KEYF11     SPECIAL(11)
#define KEYF12     SPECIAL(12)
#define KEYLCTRL   SPECIAL(13)
#define KEYRCTRL   SPECIAL(14)
#define KEYLSHIFT  SPECIAL(15)
#define KEYRSHIFT  SPECIAL(16)
#define KEYLALT    SPECIAL(17)
#define KEYRALT    SPECIAL(18)
#define KEYPRINTSCRN SPECIAL(19)
#define KEYCAPSLOCK SPECIAL(20)
#define KEYNUMLOCK SPECIAL(21)
#define KEYSCRLOCK SPECIAL(22)
#define KEYSYSREQ  SPECIAL(23)


#define KEYPADSTART 128
#define KEYPAD(num) (KEYKEYPADFLAG | KEYSPECIALFLAG | (num+KEYPADSTART))
#define KEYKPHOME  KEYPAD(0)
#define KEYKPUP    KEYPAD(1)
#define KEYKPPGUP  KEYPAD(2)
#define KEYKPMINUS KEYPAD(3)
#define KEYKPLEFT  KEYPAD(4)
#define KEYKPCENTER KEYPAD(5)
#define KEYKPRIGHT KEYPAD(6)
#define KEYKPPLUS  KEYPAD(7)
#define KEYKPEND   KEYPAD(8)
#define KEYKPDOWN  KEYPAD(9)
#define KEYKPPGDN  KEYPAD(10)
#define KEYKPINSERT KEYPAD(11)
#define KEYKPDEL   KEYPAD(12)

#ifdef GEEKOS


#define ASCIIESC 0x1B
#define ASCIIBS  0x08


void InitKeyboard();
Boolean ReadKey( Keycode* key);

Keycode WaitForKey();
Keycode WaitForKeyAtomic();



#endif // KEYBOARDH
