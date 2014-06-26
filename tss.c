#include "kassert.h"
#include "defs.h"
#include "gdt.h"
#include "segment.h"
#include "string.h"
#include "tss.h"
struct TSS gtheTSS;
void InitTSS( void )
{
    struct SegmentDescriptor* desc;
    unsigned short selector;
    desc = AllocateSegmentDescriptor();
    KASSERT( desc != 0 );
    memset( &gtheTSS, '\0', sizeof( struct TSS ) );
    InitTSSDescriptor( desc, &gtheTSS );
    selector = Selector( 0, TRUE, GetDescriptorIndex( desc ) );
    asm volatile (
	"ltr %0"
	:
	: "a" (selector)
    );
}
