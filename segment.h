
#ifndef SEGMENT_H
#define SEGMENT_H
#include "ktypes.h"
struct TSS;.
struct SegmentDescriptor {
    unsigned short sizeLow        __attribute__((packed)) ;
    unsigned int baseLow     : 24 __attribute__((packed)) ;
    unsigned int type        : 4  __attribute__((packed)) ;
    unsigned int system      : 1  __attribute__((packed)) ;
    unsigned int dpl         : 2  __attribute__((packed)) ;
    unsigned int present     : 1  __attribute__((packed)) ;
    unsigned int sizeHigh    : 4  __attribute__((packed)) ;
    unsigned int avail       : 1  __attribute__((packed)) ;
    unsigned int reserved    : 1  __attribute__((packed)) ; // set to zero
    unsigned int dbBit       : 1  __attribute__((packed)) ;
    unsigned int granularity : 1  __attribute__((packed)) ;
    unsigned char baseHigh        __attribute__((packed)) ;
};

static unsigned short Selector( int rpl, Boolean isGDT, int index )
{
    unsigned short selector = 0;
    selector = (rpl & 0x3) | ((isGDT ? 0 : 1) << 2) | ((index & 0x1FFF) << 3);
    return selector;
}

void InitNullSegmentDescriptor( struct Segment_Descriptor* desc );

void InitCodeSegmentDescriptor(
    struct Segment_Descriptor* desc,
    unsigned long baseAddr,
    unsigned long numPages,
    int privilegeLevel
);
void InitDataSegmentDescriptor(
    struct SegmentDescriptor* desc,
    unsigned long baseAddr,
    unsigned long numPages,
    int privilegeLevel
);
void Init_SSDescriptor( struct Segment_Descriptor* desc, struct TSS* theTSS );
void InitLDTDescriptor(
    struct SegmentDescriptor* desc,
    struct SegmentDescriptor theLDT[],
    int numEntries
);

#endif // SEGMENT_H
