#ifndef __COMMON_H
#define __COMMON_H

    /* exact-width signed integer types */
typedef   signed char int8_t;
typedef   signed int int16_t;
typedef   signed long int32_t;
//typedef   signed        int64_t;

    /* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned      int uint16_t;
typedef unsigned           long uint32_t;
//typedef unsigned        uint64_t;

/* These types must be 16-bit, 32-bit or larger integer */
typedef int16_t 	INT;
typedef uint16_t	UINT;

/* These types must be 8-bit integer */
// typedef int8_t		CHAR;
typedef int8_t	        UCHAR;
typedef uint8_t	        BYTE;
typedef uint8_t	        byte;


/* These types must be 16-bit integer */
typedef int16_t		SHORT;
typedef uint16_t	USHORT;
typedef uint16_t	WORD;
typedef uint16_t	WCHAR;

/* These types must be 32-bit integer */
typedef int32_t		LONG;
typedef uint32_t	ULONG;
typedef uint32_t	DWORD;



#ifndef BOOL
typedef unsigned int    BOOL;
#endif

#ifndef UINT8
typedef unsigned char	UINT8;
#endif

#ifndef SINT8
typedef signed   char   SINT8;
#endif


#ifndef SINT16
typedef signed   short  SINT16;
#endif

#ifndef SINT16
typedef unsigned   short  UINT16;
#endif

#ifndef UINT32
typedef unsigned long   UINT32;
#endif

#ifndef SINT32
typedef signed   long   SINT32;
#endif

#ifndef SIZE_T
typedef unsigned int    SIZE_T;
#endif

#ifndef BIT_T
typedef unsigned int    BIT_T;
#endif



UINT16 SWAP_INT(UINT16 data);



#endif

