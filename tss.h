#ifndef TSS_H
#define TSS_H
#define ulong unsigned long
#define ushort unsigned short
#define uint unsigned int
struct TSS {
   
    ushort link;
    ushort reserved1;
    ulong esp0;
    ushort ss0;
    ushort reserved2;
    ulong esp1;
    ushort ss1;
    ushort reserved3;
    ulong esp2;
    ushort ss2;
    ushort reserved4;
    ulong cr3;
    ulong eip;
    ulong eflags;
    ulong eax;
    ulong ecx;
    ulong edx;
    ulong ebx;
    ulong esp;
    ulong ebp;
    ulong esi;
    ulong edi;
    ushort es;
    ushort reserved5;
    ushort cs;
    ushort reserved6;
    ushort ss;
    ushort reserved7;
    ushort ds;
    ushort reserved8;
    ushort fs;
    ushort reserved9;
    ushort gs;
    ushort reserved10;
    ushort ldt;
    ushort reserved11;
    uint debugTrap : 1;
    uint reserved12 : 15;
    ushort ioMapBase;
};
extern struct TSS g_theTSS;
void Init_TSS( void );
#endif // TSS_H
