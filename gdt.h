#ifndef GDTH
#define GDTH
struct SegmentDescriptor;
void InitGDT( void );
struct SegmentDescriptor* AllocateSegmentDescriptor( void );
void FreeSegmentDescriptor( struct SegmentDescriptor* desc );
int GetDescriptorIndex( struct SegmentDescriptor* desc );
#endif // GDTH
