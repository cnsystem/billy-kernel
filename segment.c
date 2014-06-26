#include "kassert.h"
#include "string.h"
#include "tss.h"
#include "segment.h"

static void SetSizeAndBasePages(
    struct SegmentDescriptor* desc,
    unsigned long baseAddr,
    unsigned long numPages
)
{   
    KASSERT( numPages > 0 );
    numPages -= 1;
    desc->sizeLow     = numPages & 0xFFFF;
    desc->sizeHigh    = (numPages >> 16) & 0x0F;
    desc->baseLow     = baseAddr & 0xFFFFFF;
    desc->baseHigh    = (baseAddr >> 24) & 0xFF;
    desc->granularity = 1; // size in pages
}

static inline void SetSizeAndBaseBytes(
    struct SegmentDescriptor* desc,
    unsigned long baseAddr,
    unsigned long numBytes
)
{
    desc->sizeLow     = numBytes & 0xFFFF;
    desc->sizeHigh    = (numBytes >> 16) & 0x0F;
    desc->baseLow     = baseAddr & 0xFFFFFF;
    desc->baseHigh    = (baseAddr >> 24) & 0xFF;
    desc->granularity = 0; // size in bytes
}

void InitNullSegmentDescriptor( struct SegmentDescriptor* desc )
{
    memset( desc, '\0', sizeof( *desc ) );
}


void InitCodeSegmentDescriptor(
    struct SegmentDescriptor* desc,
    unsigned long baseAddr,
    unsigned long numPages,
    int privilegeLevel
)
{
    KASSERT( privilegeLevel >= 0 && privilegeLevel <= 3 );

    SetSizeAndBasePages( desc, baseAddr, numPages );
    desc->type     = 0x0A;  // 1010b: code, !conforming, readable, !accessed
    desc->system   = 1;
    desc->dpl      = privilegeLevel;
    desc->present  = 1;
    desc->reserved = 0;
    desc->dbBit    = 1; // 32 bit code segment
}

void InitDataSegmentDescriptor(
    struct SegmentDescriptor* desc,
    unsigned long baseAddr,
    unsigned long numPages,
    int privilegeLevel
)
{
    KASSERT( privilegeLevel >= 0 && privilegeLevel <= 3 );

    SetSizeAndBasePages( desc, baseAddr, numPages );
    desc->type     = 0x02; // 0010b: data, expand-up, writable, !accessed
    desc->system   = 1;
    desc->dpl      = privilegeLevel;
    desc->present  = 1;
    desc->reserved = 0;
    desc->dbBit    = 1; // 32 bit operands
}

void InitTSSDescriptor( struct SegmentDescriptor* desc, struct TSS* theTSS )
{
    SetSizeAndBaseBytes( desc, (unsigned long) theTSS, sizeof(struct TSS) );
    desc->type     = 0x09; // 1001b: 32 bit, !busy
    desc->system   = 0;
    desc->dpl      = 0;
    desc->present  = 1;
    desc->reserved = 0;
    desc->dbBit    = 0; // must be 0 in TSS
}


void InitLDTDescriptor(
    struct SegmentDescriptor* desc,
    struct SegmentDescriptor theLDT[],
    int numEntries
)
{
    SetSizeAndBaseBytes(
	desc, (unsigned long) theLDT, sizeof(struct SegmentDescriptor) * numEntries );

    desc->type     = 0x02; // 0010b
    desc->system   = 0;
    desc->dpl      = 0;
    desc->present  = 1;
    desc->reserved = 0;
    desc->dbBit    = 0;
}
