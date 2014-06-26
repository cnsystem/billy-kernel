#include "segment.h"
#include "tss.h"
#include "gdt.h"
extern void LoadGDTR( unsigned short* limitAndBase );

#define NUMGDTENTRIES 16

static struct SegmentDescriptor sGDT[ NUMGDTENTRIES ];

static int snumAllocated = 0;

struct SegmentDescriptor* AllocateSegmentDescriptor( void )
{
    struct SegmentDescriptor* desc = 0;
    int i;

    for ( i = 1; i < NUMGDTENTRIES; ++i ) {
	desc = &sGDT[ i ];
	if ( desc->avail ) {
	    ++snumAllocated;
	    desc->avail = 0;
	    break;
	}
    }

    return desc;
}

void FreeSegmentDescriptor( struct SegmentDescriptor* desc )
{
    InitNullSegmentDescriptor( desc );
    desc->avail = 1;
    --snumAllocated;
}

int GetDescriptorIndex( struct SegmentDescriptor* desc )
{
    return (int) (desc - sGDT);
}

void InitGDT( void )
{
    unsigned short limitAndBase[3];
    unsigned long gdtBaseAddr = (unsigned long) sGDT;
    struct SegmentDescriptor* desc;
    int i;

    for ( i = 0; i < NUMGDTENTRIES; ++i ) {
	desc = &sGDT[ i ];
	InitNullSegmentDescriptor( desc );
	desc->avail = 1;
    }

    desc = AllocateSegmentDescriptor();
    InitCodeSegmentDescriptor(
	desc,
	0,		// base address
	0x100000,	// num pages (== 2^20)
	0		// privilege level (0 == kernel)
    );

    desc = AllocateSegmentDescriptor();
    InitDataSegmentDescriptor(
	desc,
	0,		// base address
	0x100000,	// num pages (== 2^20)
	0		// privilege level (0 == kernel)
    );


    desc = AllocateSegmentDescriptor();
    InitTSSDescriptor( desc, &gtheTSS );

    limitAndBase[0] = sizeof( struct SegmentDescriptor ) * NUMGDTENTRIES;
    limitAndBase[1] = gdtBaseAddr & 0xffff;
    limitAndBase[2] = gdtBaseAddr >> 16;
    LoadGDTR( limitAndBase );
}
