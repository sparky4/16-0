/* This file implements rudimentary XMS memory handling.
 * Documentation on the XMS API was found on http://www.qzx.com/pc-gpe/xms30.txt
 */

#include <stddef.h> /* Definition of NULL */
#include <limits.h> /* Definition of UINT_MAX */
#include <stdio.h>  /* fprintf and (FILE *) */

/* Allow external configuration of maximum concurrent XMS allocations */
#ifndef MAX_XMS_ALLOCATIONS
#define MAX_XMS_ALLOCATIONS 4
#endif

/* Address of the XMS driver */
static long XMSControl;

/* Mapping of XMS handle <-> normal pointer */
typedef struct {
    unsigned int XMSHandle;
    void huge * XMSPointer;
} XMSHandleMap;

static XMSHandleMap allocMap[MAX_XMS_ALLOCATIONS];

/* Set up the XMS driver, returns 0 on success and non-zero on failure */
static int initxms(void)
{
    char XMSStatus = 0;

    if ( XMSControl == 0 )
    {
        __asm {
        // Is an XMS driver installed?
            mov ax,4300h
            int 2Fh
            mov [XMSStatus], al
        }

        if ( XMSStatus == 0x80 )
        {
            __asm {
            // Get the address of the driver control function
                mov ax,4310h
                int 2Fh
                mov word ptr [XMSControl]  ,bx
                mov word ptr [XMSControl+2],es
            }
        }
    }

    return ( XMSControl == 0 );
}

/* Allocate a slab of memory from XMS */
void huge * xmsmalloc(long unsigned int size)
{
    unsigned int kB;
    unsigned int XMSStatus = 0;
    unsigned int XMSHandle = 0;
    void huge * XMSPointer = NULL;
    int n;

    /* If we can not initialize XMS, the allocation fails */
    if ( initxms() )
        return NULL;

    /* It is not possible to allocate more kilobytes than a 16-bit register can hold :-) */
    if ( size / 1024 > UINT_MAX )
        return NULL;

    /* Get the next free entry in the handle <-> pointer mapping */
    for ( n = 0; n < MAX_XMS_ALLOCATIONS; n++ )
    {
        if ( allocMap[n].XMSPointer == NULL )
            break;
    }

    if ( n == MAX_XMS_ALLOCATIONS )
        return NULL;

    kB = size / 1024 + (size % 1024 > 0);

    __asm {
    // Allocate [kB] kilobytes of XMS memory
        mov ah, 09h
        mov dx, [kB]
        call [XMSControl]
        mov [XMSStatus], ax
        mov [XMSHandle], dx
    }

    /* Check if XMS allocation failed */
    if ( XMSStatus == 0)
        return NULL;

    __asm {
    // Convert XMS handle to normal pointer
        mov ah, 0Ch
        mov dx, [XMSHandle]
        call [XMSControl]
        mov [XMSStatus], ax

        mov word ptr [XMSPointer],  bx
        mov word ptr [XMSPointer+2],dx
    }

    if ( XMSStatus == 0 )
    {
        /* Lock failed, deallocate the handle */
        __asm {
        // Free XMS handle
            mov ah, 0Ah
            mov dx, [XMSHandle]
            call [XMSControl]

        // Return value is not really interesting
        //   mov [XMSStatus], ax
        }
        return NULL;
    }

    /* Create an entry in the handle <-> pointer mapping */
    allocMap[n].XMSHandle = XMSHandle;
    allocMap[n].XMSPointer = XMSPointer;

    return XMSPointer;
}

/* Free a pointer allocated with xmsalloc */
void xmsfree(void huge * XMSPointer)
{
    int n;

    if ( XMSPointer == NULL )
        return;

    if ( initxms() )
        return;

    for ( n = 0; n < MAX_XMS_ALLOCATIONS; n++ )
    {
        if ( allocMap[n].XMSPointer == XMSPointer )
        {
            int XMSHandle = allocMap[n].XMSHandle;

            __asm {
            // Unlock handle so we can free the memory block
                mov ah, 0Dh
                mov dx, [XMSHandle]
                call [XMSControl]

            // Free XMS memory
                mov ah, 0Ah
                mov dx, [XMSHandle]
                call [XMSControl]

            // Return value ignored
            }

            /* Clear handle <-> pointer map entry so it can be reused */
            allocMap[n].XMSHandle = 0;
            allocMap[n].XMSPointer = NULL;

            return;
        }
    }
}

/* Write a memory report for debugging purposes */
void xmsreport(FILE * stream)
{
    int XMSVersionNumber = 0;
    int XMSLargestBlock = 0;
    int XMSTotal = 0;

    if ( initxms() )
    {
        puts("Could not initialize XMS Driver!");
        return;
    }

    __asm {
    // Get the driver version number
        mov ah,00h
        call [XMSControl] ; Get XMS Version Number
        mov [XMSVersionNumber], ax

    // Get the amount of free XMS memory
        mov ah, 08h
        call [XMSControl]
        mov [XMSLargestBlock], ax
        mov [XMSTotal], dx
    }

    fprintf(stream, "XMS Version number: %d\n", XMSVersionNumber);
    fprintf(stream, "Largest available block: %d kB (%d kB total)\n", XMSLargestBlock, XMSTotal);
	printf("XMS Version number: %d\n", XMSVersionNumber);
    printf("Largest available block: %d kB (%d kB total)\n", XMSLargestBlock, XMSTotal);
}

void main ()
{
	XMSHandleMap a;
	if(!initxms())
	{
		a.XMSPointer = xmsmalloc(128);
		xmsreport(stdout);
		xmsfree(a.XMSPointer);
	}
	else printf("error\n");
}
