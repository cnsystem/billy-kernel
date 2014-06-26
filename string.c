#include "string.h"
void* memset( void* v, int i, size_t n )
{
    unsigned char* c = (unsigned char*) v;
    while ( n > 0 ) {
	*c++ = (unsigned char) i;
	--n;
    }
    return v;
}
void* memcpy( void *dst, const void* src, size_t n )
{
    unsigned char* d = (unsigned char*) dst;
    const unsigned char* s = (const unsigned char*) src;
    while ( n > 0 ) {
	*d++ = *s++;
	--n;
    }
    return dst;
}
size_t strlen( const char* s )
{
    size_t len = 0;
    while ( *s++ != '\0' )
	++len;
    return len;
}

int strcmp( const char* s1, const char* s2 )
{
    while ( 1 ) {
	int cmp = *s1 - *s2;
	if ( cmp != 0 || *s1 == '\0' || *s2 == '\0' )
	    return cmp;
	++s1;
	++s2;
    }
}
