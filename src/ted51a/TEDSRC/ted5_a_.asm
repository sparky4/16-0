.387
		PUBLIC	_CombineTiles
		PUBLIC	_DrawTile
		PUBLIC	_CopyCGA
		PUBLIC	_CopyEGA
		PUBLIC	_CopyVGA
		PUBLIC	_Overlay
		PUBLIC	_tdata
		EXTRN	_xmshandle:BYTE
		EXTRN	_videomode:BYTE
		EXTRN	_XMSlookup:BYTE
		EXTRN	_XMSdriver:BYTE
		EXTRN	_lasticon:BYTE
		EXTRN	_firsticon:BYTE
		EXTRN	_CGAlookup:BYTE
		EXTRN	_EGA1lookup:BYTE
		EXTRN	_EGA2lookup:BYTE
		EXTRN	_VGAlookup:BYTE
		EXTRN	_tilenum:BYTE
DGROUP		GROUP	_DATA
TED5_A_TEXT		SEGMENT	WORD PUBLIC USE16 'CODE'
		ASSUME CS:TED5_A_TEXT, DS:DGROUP, SS:DGROUP
_CombineTiles:
    enter       0,0 
    push        si 
    push        di 
    cld         
    mov         bx,word ptr 0cH[bp] 
    shl         bx,1 
    mov         dx,word ptr DGROUP:L$88[bx] 
    mov         word ptr DGROUP:L$90,dx 
    mov         cx,word ptr DGROUP:L$87[bx] 
    mov         word ptr DGROUP:L$89,cx 
    mov         ax,word ptr _xmshandle 
    mov         word ptr DGROUP:L$100,ax 
    mov         si,word ptr _videomode 
    shl         si,1 
    mov         ax,word ptr DGROUP:L$80[si] 
    jmp         ax 
COMBINECGA:
    shl         word ptr DGROUP:L$89,1 
    mov         cx,word ptr DGROUP:L$95[bx] 
    mov         word ptr DGROUP:L$91,cx 
    mov         ax,word ptr DGROUP:L$92[bx] 
    mov         word ptr DGROUP:L$99,ax 
    sub         cx,ax 
    mov         word ptr DGROUP:L$92,cx 
    mov         ax,word ptr 6[bp] 
    cmp         ax,8000H 
    jb          L$1 
    nop         
    nop         
    neg         ax 
    mov         di,ds 
    mov         es,di 
    mov         di,offset DGROUP:_tdata 
    mov         bx,ax 
    and         bx,1 
    mov         al,byte ptr DGROUP:L$83[bx] 
    mov         cx,word ptr DGROUP:L$99 
    rep stosb   
    jmp         L$3 
L$1:
    shl         ax,2 
    mov         bx,ax 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$2 
    nop         
    nop         
    mov         ax,offset DGROUP:_tdata 
    mov         word ptr DGROUP:L$103,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         ax,offset DGROUP:L$98 
    mov         word ptr DGROUP:L$103,ax 
    jmp         L$3 
    nop         
L$2:
    mov         bx,word ptr 0cH[bp] 
    shl         bx,1 
    mov         si,word ptr DGROUP:L$104[bx] 
    mov         di,offset DGROUP:_tdata 
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,1 
    mov         ax,ds 
    mov         es,ax 
    rep movsw   
L$3:
    mov         bx,word ptr 8[bp] 
    cmp         bx,0 
    je          L$6 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$6 
    nop         
    nop         
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$4:
    mov         cx,word ptr DGROUP:L$89 
L$5:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$5 
    dec         dx 
    jne         L$4 
L$6:
    mov         bx,word ptr 0aH[bp] 
    cmp         bx,word ptr _lasticon 
    ja          L$9 
    nop         
    nop         
    cmp         bx,word ptr _firsticon 
    jbe         L$9 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$7:
    mov         cx,word ptr DGROUP:L$89 
L$8:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$8 
    dec         dx 
    jne         L$7 
L$9:
    pop         di 
    pop         si 
    leave       
    retf        
COMBINEEGA:
    mov         cx,word ptr DGROUP:L$96[bx] 
    mov         word ptr DGROUP:L$91,cx 
    mov         ax,word ptr DGROUP:L$93[bx] 
    mov         word ptr DGROUP:L$99,ax 
    sub         cx,ax 
    mov         word ptr DGROUP:L$92,cx 
    mov         ax,word ptr 6[bp] 
    cmp         ax,8000H 
    jb          L$10 
    nop         
    nop         
    neg         ax 
    mov         di,ds 
    mov         es,di 
    mov         di,offset DGROUP:_tdata 
    mov         dx,ax 
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,3 
    mov         bx,dx 
    shr         dx,1 
    and         bx,1 
    shl         bx,1 
    mov         ax,word ptr DGROUP:L$84[bx] 
    rep stosw   
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,3 
    mov         bx,dx 
    shr         dx,1 
    and         bx,1 
    shl         bx,1 
    mov         ax,word ptr DGROUP:L$84[bx] 
    rep stosw   
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,3 
    mov         bx,dx 
    shr         dx,1 
    and         bx,1 
    shl         bx,1 
    mov         ax,word ptr DGROUP:L$84[bx] 
    rep stosw   
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,3 
    mov         bx,dx 
    shr         dx,1 
    and         bx,1 
    shl         bx,1 
    mov         ax,word ptr DGROUP:L$84[bx] 
    rep stosw   
    jmp         L$12 
L$10:
    shl         ax,2 
    mov         bx,ax 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$11 
    nop         
    nop         
    mov         ax,offset DGROUP:_tdata 
    mov         word ptr DGROUP:L$103,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         ax,offset DGROUP:L$98 
    mov         word ptr DGROUP:L$103,ax 
    jmp         L$12 
    nop         
L$11:
    mov         bx,word ptr 0cH[bp] 
    shl         bx,1 
    mov         si,word ptr DGROUP:L$105[bx] 
    mov         di,offset DGROUP:_tdata 
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,1 
    mov         ax,ds 
    mov         es,ax 
    rep movsw   
L$12:
    mov         bx,word ptr 8[bp] 
    cmp         bx,0 
    je          L$16 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$16 
    nop         
    nop         
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         ah,4 
L$13:
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$14:
    mov         cx,word ptr DGROUP:L$89 
L$15:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$15 
    dec         dx 
    jne         L$14 
    dec         ah 
    jne         L$13 
L$16:
    mov         bx,word ptr 0aH[bp] 
    cmp         bx,word ptr _lasticon 
    ja          L$20 
    nop         
    nop         
    cmp         bx,word ptr _firsticon 
    jbe         L$20 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         ah,4 
L$17:
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$18:
    mov         cx,word ptr DGROUP:L$89 
L$19:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$19 
    dec         dx 
    jne         L$18 
    dec         ah 
    jne         L$17 
L$20:
    pop         di 
    pop         si 
    leave       
    retf        
COMBINEVGA:
    shl         word ptr DGROUP:L$89,3 
    mov         cx,word ptr DGROUP:L$97[bx] 
    mov         word ptr DGROUP:L$91,cx 
    mov         ax,word ptr DGROUP:L$94[bx] 
    mov         word ptr DGROUP:L$99,ax 
    sub         cx,ax 
    mov         word ptr DGROUP:L$92,cx 
    mov         ax,word ptr 6[bp] 
    cmp         ax,8000H 
    jb          L$21 
    nop         
    nop         
    neg         ax 
    mov         di,ds 
    mov         es,di 
    mov         di,offset DGROUP:_tdata 
    shl         ax,4 
    mov         bx,word ptr DGROUP:L$99 
    shr         bx,3 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    inc         al 
    mov         cx,bx 
    rep stosb   
    jmp         L$23 
L$21:
    shl         ax,2 
    mov         bx,ax 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$22 
    nop         
    nop         
    mov         ax,offset DGROUP:_tdata 
    mov         word ptr DGROUP:L$103,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         ax,offset DGROUP:L$98 
    mov         word ptr DGROUP:L$103,ax 
    jmp         L$23 
    nop         
L$22:
    mov         bx,word ptr 0cH[bp] 
    shl         bx,1 
    mov         si,word ptr DGROUP:L$106[bx] 
    mov         di,offset DGROUP:_tdata 
    mov         cx,word ptr DGROUP:L$99 
    shr         cx,1 
    mov         ax,ds 
    mov         es,ax 
    rep movsw   
L$23:
    mov         bx,word ptr 8[bp] 
    cmp         bx,0 
    je          L$26 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    cmp         ax,0ffffH 
    je          L$26 
    nop         
    nop         
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$24:
    mov         cx,word ptr DGROUP:L$89 
L$25:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$25 
    dec         dx 
    jne         L$24 
L$26:
    mov         bx,word ptr 0aH[bp] 
    cmp         bx,word ptr _lasticon 
    ja          L$29 
    nop         
    nop         
    cmp         bx,word ptr _firsticon 
    jbe         L$29 
    nop         
    nop         
    shl         bx,2 
    mov         es,word ptr _XMSlookup 
    mov         ax,word ptr es:[bx] 
    mov         word ptr DGROUP:L$101,ax 
    mov         ax,word ptr es:2[bx] 
    mov         word ptr DGROUP:L$102,ax 
    mov         ax,word ptr DGROUP:L$91 
    mov         word ptr DGROUP:L$99,ax 
    mov         si,offset DGROUP:L$99 
    mov         ah,0bH 
    call        dword ptr _XMSdriver 
    mov         si,offset DGROUP:L$98 
    add         si,word ptr DGROUP:L$92 
    mov         di,offset DGROUP:_tdata 
    mov         dx,word ptr DGROUP:L$90 
    mov         bx,offset DGROUP:L$98 
L$27:
    mov         cx,word ptr DGROUP:L$89 
L$28:
    mov         al,byte ptr [bx] 
    and         byte ptr [di],al 
    lodsb       
    or          byte ptr [di],al 
    inc         di 
    inc         bx 
    loop        L$28 
    dec         dx 
    jne         L$27 
L$29:
    pop         di 
    pop         si 
    leave       
    retf        
_DrawTile:
    enter       0,0 
    push        si 
    push        di 
    mov         bx,word ptr 0aH[bp] 
    shl         bx,1 
    mov         ah,byte ptr DGROUP:L$88[bx] 
    mov         byte ptr DGROUP:L$85,ah 
    mov         cx,word ptr DGROUP:L$87[bx] 
    mov         bx,word ptr _videomode 
    shl         bx,1 
    mov         dx,word ptr DGROUP:L$81[bx] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    jmp         dx 
DRAWTCGA:
    mov         ax,0b800H 
    mov         es,ax 
    mov         si,offset DGROUP:_tdata 
    mov         di,word ptr 6[bp] 
    shl         di,1 
    add         di,word ptr _CGAlookup[bx] 
    mov         bx,cx 
    shl         bx,1 
    mov         ah,byte ptr DGROUP:L$85 
L$30:
    mov         cx,bx 
    rep movsb   
    sub         di,bx 
    xor         di,2000H 
    mov         cx,bx 
    rep movsb   
    sub         di,bx 
    xor         di,2000H 
    add         di,50H 
    sub         ah,2 
    jne         L$30 
    pop         di 
    pop         si 
    leave       
    retf        
DRAWTEGA1:
    mov         word ptr DGROUP:L$87,26H 
    mov         di,word ptr _EGA1lookup[bx] 
    jmp         L$31 
DRAWTEGA2:
    mov         word ptr DGROUP:L$87,4eH 
    mov         di,word ptr _EGA2lookup[bx] 
    jmp         L$31 
L$31:
    add         di,word ptr 6[bp] 
    mov         word ptr DGROUP:L$86,di 
    mov         si,offset DGROUP:_tdata 
    mov         bx,0a000H 
    mov         es,bx 
    mov         bx,cx 
    cld         
    mov         dx,3ceH 
    mov         al,5 
    out         dx,al 
    inc         dx 
    mov         al,0 
    out         dx,al 
    mov         dx,3c4H 
    mov         al,2 
    out         dx,al 
    inc         dx 
    mov         al,1 
    out         dx,al 
    mov         dx,word ptr DGROUP:L$87 
L$32:
    mov         cx,bx 
    rep movsb   
    add         di,dx 
    dec         ah 
    jne         L$32 
    mov         dx,3c4H 
    mov         al,2 
    out         dx,al 
    inc         dx 
    mov         al,2 
    out         dx,al 
    mov         dx,word ptr DGROUP:L$87 
    mov         di,word ptr DGROUP:L$86 
    mov         ah,byte ptr DGROUP:L$85 
L$33:
    mov         cx,bx 
    rep movsb   
    add         di,dx 
    dec         ah 
    jne         L$33 
    mov         dx,3c4H 
    mov         al,2 
    out         dx,al 
    inc         dx 
    mov         al,4 
    out         dx,al 
    mov         dx,word ptr DGROUP:L$87 
    mov         di,word ptr DGROUP:L$86 
    mov         ah,byte ptr DGROUP:L$85 
L$34:
    mov         cx,bx 
    rep movsb   
    add         di,dx 
    dec         ah 
    jne         L$34 
    mov         dx,3c4H 
    mov         al,2 
    out         dx,al 
    inc         dx 
    mov         al,8 
    out         dx,al 
    mov         dx,word ptr DGROUP:L$87 
    mov         di,word ptr DGROUP:L$86 
    mov         ah,byte ptr DGROUP:L$85 
L$35:
    mov         cx,bx 
    rep movsb   
    add         di,dx 
    dec         ah 
    jne         L$35 
    pop         di 
    pop         si 
    leave       
    retf        
DRAWTVGA:
    mov         ax,0a000H 
    mov         es,ax 
    mov         si,offset DGROUP:_tdata 
    mov         di,word ptr 6[bp] 
    shl         di,3 
    add         di,word ptr _VGAlookup[bx] 
    mov         bx,cx 
    shl         bx,2 
    mov         ah,byte ptr DGROUP:L$85 
L$36:
    mov         cx,bx 
    rep movsw   
    add         di,130H 
    dec         ah 
    jne         L$36 
    pop         di 
    pop         si 
    leave       
    retf        
_CopyCGA:
    enter       0,0 
    push        si 
    push        di 
    push        ds 
    mov         ax,0b800H 
    mov         es,ax 
    mov         ds,ax 
    shl         word ptr 0aH[bp],1 
    shl         word ptr 0eH[bp],1 
    shl         word ptr 6[bp],1 
    mov         ax,word ptr 8[bp] 
    cmp         ax,word ptr 10H[bp] 
    jb          L$40 
    nop         
    nop         
    ja          L$42 
    mov         ax,word ptr 6[bp] 
    cmp         ax,word ptr 0eH[bp] 
    jb          L$38 
    nop         
    nop         
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    cld         
L$37:
    mov         si,word ptr ss:_CGAlookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    add         bx,2 
    dec         dx 
    jne         L$37 
    je          L$44 
L$38:
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
L$39:
    mov         si,word ptr ss:_CGAlookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         si,word ptr 0aH[bp] 
    dec         si 
    add         di,word ptr 0eH[bp] 
    add         di,word ptr 0aH[bp] 
    dec         di 
    mov         cx,word ptr 0aH[bp] 
    std         
    rep movsb   
    cld         
    add         bx,2 
    dec         dx 
    jne         L$39 
    je          L$44 
    nop         
    nop         
L$40:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    add         dl,ah 
    dec         dl 
    mov         dh,byte ptr 10H[bp] 
    add         dh,ah 
    dec         dh 
    cld         
L$41:
    mov         bx,word ptr 8[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         si,word ptr ss:_CGAlookup[bx] 
    mov         bx,word ptr 10H[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         di,word ptr ss:_CGAlookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    dec         word ptr 0cH[bp] 
    jne         L$41 
    je          L$44 
    nop         
    nop         
L$42:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    mov         dh,byte ptr 10H[bp] 
    cld         
L$43:
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    mov         si,word ptr ss:_CGAlookup[bx] 
    mov         bx,word ptr 10H[bp] 
    shl         bx,1 
    mov         di,word ptr ss:_CGAlookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    inc         word ptr 8[bp] 
    inc         word ptr 10H[bp] 
    dec         word ptr 0cH[bp] 
    jne         L$43 
    je          L$44 
    nop         
    nop         
L$44:
    pop         ds 
    pop         di 
    pop         si 
    leave       
    retf        
_CopyEGA:
    enter       0,0 
    push        si 
    push        di 
    push        ds 
    mov         dx,3ceH 
    mov         ax,105H 
    out         dx,ax 
    mov         ax,0a000H 
    mov         es,ax 
    mov         ds,ax 
    mov         ax,word ptr 8[bp] 
    cmp         ax,word ptr 10H[bp] 
    jb          L$50 
    ja          L$53 
    mov         ax,word ptr 6[bp] 
    cmp         ax,word ptr 0eH[bp] 
    jb          L$47 
    nop         
    nop         
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    cld         
    cmp         word ptr ss:_videomode,2 
    jb          L$46 
    nop         
    nop         
L$45:
    mov         si,word ptr ss:_EGA2lookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    add         bx,2 
    dec         dx 
    jne         L$45 
    je          L$56 
L$46:
    mov         si,word ptr ss:_EGA1lookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    add         bx,2 
    dec         dx 
    jne         L$46 
    je          L$56 
L$47:
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    cmp         word ptr ss:_videomode,2 
    jb          L$49 
    nop         
    nop         
L$48:
    mov         si,word ptr ss:_EGA2lookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         si,word ptr 0aH[bp] 
    dec         si 
    add         di,word ptr 0eH[bp] 
    add         di,word ptr 0aH[bp] 
    dec         di 
    mov         cx,word ptr 0aH[bp] 
    std         
    rep movsb   
    cld         
    add         bx,2 
    dec         dx 
    jne         L$48 
    je          L$56 
L$49:
    mov         si,word ptr ss:_EGA1lookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         si,word ptr 0aH[bp] 
    dec         si 
    add         di,word ptr 0eH[bp] 
    add         di,word ptr 0aH[bp] 
    dec         di 
    mov         cx,word ptr 0aH[bp] 
    std         
    rep movsb   
    cld         
    add         bx,2 
    dec         dx 
    jne         L$49 
    je          L$56 
L$50:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    add         dl,ah 
    dec         dl 
    mov         dh,byte ptr 10H[bp] 
    add         dh,ah 
    dec         dh 
    cld         
    cmp         word ptr ss:_videomode,2 
    jb          L$52 
    nop         
    nop         
L$51:
    mov         bx,word ptr 8[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         si,word ptr ss:_EGA2lookup[bx] 
    mov         bx,word ptr 10H[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         di,word ptr ss:_EGA2lookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    dec         word ptr 0cH[bp] 
    jne         L$51 
    je          L$56 
L$52:
    mov         bl,dl 
    xor         bh,bh 
    shl         bx,1 
    mov         si,word ptr ss:_EGA1lookup[bx] 
    mov         bl,dh 
    xor         bh,bh 
    shl         bx,1 
    mov         di,word ptr ss:_EGA1lookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    dec         dl 
    dec         dh 
    dec         ah 
    jne         L$52 
    je          L$56 
    nop         
    nop         
L$53:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    mov         dh,byte ptr 10H[bp] 
    cld         
    cmp         word ptr ss:_videomode,2 
    jb          L$55 
    nop         
    nop         
L$54:
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    mov         si,word ptr ss:_EGA2lookup[bx] 
    mov         bx,word ptr 10H[bp] 
    shl         bx,1 
    mov         di,word ptr ss:_EGA2lookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    inc         word ptr 8[bp] 
    inc         word ptr 10H[bp] 
    dec         word ptr 0cH[bp] 
    jne         L$54 
    je          L$56 
    nop         
    nop         
L$55:
    mov         bl,dl 
    xor         bh,bh 
    shl         bx,1 
    mov         si,word ptr ss:_EGA1lookup[bx] 
    mov         bl,dh 
    xor         bh,bh 
    shl         bx,1 
    mov         di,word ptr ss:_EGA1lookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    inc         dl 
    inc         dh 
    dec         ah 
    jne         L$55 
L$56:
    pop         ds 
    pop         di 
    pop         si 
    leave       
    retf        
_CopyVGA:
    enter       0,0 
    push        si 
    push        di 
    push        ds 
    mov         ax,0a000H 
    mov         es,ax 
    mov         ds,ax 
    shl         word ptr 0aH[bp],3 
    shl         word ptr 0eH[bp],3 
    shl         word ptr 6[bp],3 
    mov         ax,word ptr 8[bp] 
    cmp         ax,word ptr 10H[bp] 
    jb          L$60 
    nop         
    nop         
    ja          L$62 
    mov         ax,word ptr 6[bp] 
    cmp         ax,word ptr 0eH[bp] 
    jb          L$58 
    nop         
    nop         
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    cld         
L$57:
    mov         si,word ptr ss:_VGAlookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    add         bx,2 
    dec         dx 
    jne         L$57 
    je          L$64 
L$58:
    mov         dx,word ptr 0cH[bp] 
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
L$59:
    mov         si,word ptr ss:_VGAlookup[bx] 
    mov         di,si 
    add         si,word ptr 6[bp] 
    add         si,word ptr 0aH[bp] 
    dec         si 
    add         di,word ptr 0eH[bp] 
    add         di,word ptr 0aH[bp] 
    dec         di 
    mov         cx,word ptr 0aH[bp] 
    std         
    rep movsb   
    cld         
    add         bx,2 
    dec         dx 
    jne         L$59 
    je          L$64 
    nop         
    nop         
L$60:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    add         dl,ah 
    dec         dl 
    mov         dh,byte ptr 10H[bp] 
    add         dh,ah 
    dec         dh 
    cld         
L$61:
    mov         bx,word ptr 8[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         si,word ptr ss:_VGAlookup[bx] 
    mov         bx,word ptr 10H[bp] 
    add         bx,word ptr 0cH[bp] 
    dec         bx 
    shl         bx,1 
    mov         di,word ptr ss:_VGAlookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    dec         word ptr 0cH[bp] 
    jne         L$61 
    je          L$64 
    nop         
    nop         
L$62:
    mov         ah,byte ptr 0cH[bp] 
    mov         dl,byte ptr 8[bp] 
    mov         dh,byte ptr 10H[bp] 
    cld         
L$63:
    mov         bx,word ptr 8[bp] 
    shl         bx,1 
    mov         si,word ptr ss:_VGAlookup[bx] 
    mov         bx,word ptr 10H[bp] 
    shl         bx,1 
    mov         di,word ptr ss:_VGAlookup[bx] 
    add         si,word ptr 6[bp] 
    add         di,word ptr 0eH[bp] 
    mov         cx,word ptr 0aH[bp] 
    rep movsb   
    inc         word ptr 8[bp] 
    inc         word ptr 10H[bp] 
    dec         word ptr 0cH[bp] 
    jne         L$63 
    je          L$64 
    nop         
    nop         
L$64:
    pop         ds 
    pop         di 
    pop         si 
    leave       
    retf        
_Overlay:
    enter       0,0 
    push        di 
    push        si 
    mov         di,offset DGROUP:_tdata 
    mov         bx,word ptr 6[bp] 
    shl         bx,1 
    mov         si,word ptr _videomode 
    shl         si,1 
    mov         ax,word ptr DGROUP:L$82[si] 
    jmp         ax 
OVERLAYCGA:
    mov         cx,word ptr DGROUP:L$92[bx] 
    shr         cx,1 
    mov         si,word ptr DGROUP:L$104[bx] 
L$65:
    lodsw       
    or          word ptr [di],ax 
    add         di,2 
    loop        L$65 
    pop         si 
    pop         di 
    leave       
    retf        
OVERLAYEGA:
    mov         cx,word ptr DGROUP:L$93[bx] 
    shr         cx,1 
    mov         si,word ptr DGROUP:L$105[bx] 
L$66:
    lodsw       
    or          word ptr [di],ax 
    add         di,2 
    loop        L$66 
    pop         si 
    pop         di 
    leave       
    retf        
OVERLAYVGA:
    mov         cx,word ptr DGROUP:L$94[bx] 
    shr         cx,1 
    mov         si,word ptr DGROUP:L$106[bx] 
L$67:
    lodsw       
    or          word ptr [di],ax 
    add         di,2 
    loop        L$67 
    pop         si 
    pop         di 
    leave       
    retf        
TED5_A_TEXT		ENDS
_DATA		SEGMENT	WORD PUBLIC USE16 'DATA'
L$80:
    DD	offset L$68
    DD	offset L$70
L$81:
    DD	offset L$72
    DD	offset L$74
L$82:
    DD	offset L$76
    DD	offset L$78
L$83:
    DB	55H, 0aaH
L$84:
    DB	0, 0, 0ffH, 0ffH
L$85:
    DB	0
L$86:
    DB	0, 0
L$87:
    DB	0, 0, 1, 0, 2, 0
L$88:
    DB	4, 0, 8, 0, 10H, 0, 20H, 0
L$89:
    DB	0, 0
L$90:
    DB	0, 0
L$91:
    DB	0, 0
L$92:
    DB	0, 0, 20H, 0, 40H, 0
L$93:
    DB	0, 1, 20H, 0, 80H, 0
L$94:
    DB	0, 2, 40H, 0, 0, 1
L$95:
    DB	0, 4, 40H, 0, 80H, 0
L$96:
    DB	0, 2, 28H, 0, 0a0H, 0
L$97:
    DB	80H, 2, 80H, 0, 0, 2, 0, 8
_tdata:
    DB	0FFH DUP(0,0,0,0,0,0,0,0)
    DB	0, 0, 0, 0, 0, 0, 0, 0
L$98:
    DB	0FFH DUP(0,0,0,0,0,0,0,0)
    DB	0, 0, 0, 0, 0, 0, 0, 0
L$99:
    DB	0, 0, 0, 0
L$100:
    DB	0, 0
L$101:
    DB	0, 0
L$102:
    DB	0, 0, 0, 0
L$103:
    DD	DGROUP:L$98
L$104 equ $-2
    DW	offset DGROUP:L$107
    DW	offset DGROUP:L$108
L$105:
    DW	offset DGROUP:L$109
    DW	offset DGROUP:L$110
    DW	offset DGROUP:L$111
L$106:
    DW	offset DGROUP:L$112
    DW	offset DGROUP:L$113
    DW	offset DGROUP:L$114
    DW	offset DGROUP:L$115
L$107:
    DB	0ffH, 0ffH, 0c0H, 0, 0c0H, 0, 0c0H, 0
    DB	0c0H, 0, 0c0H, 0, 0c0H, 0, 0c0H, 0
L$108:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0c0H, 0, 0, 0
    DB	06H DUP(0c0H,0,0,0,0c0H,0,0,0)
    DB	0c0H, 0, 0, 0, 0c0H, 0, 0, 0
L$109:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	01EH DUP(0c0H,0,0,0,0,0,0,0)
    DB	0c0H, 0, 0, 0, 0, 0, 0, 0
L$110:
    DB	0ffH, 80H, 80H, 80H, 80H, 80H, 80H, 80H
    DB	0ffH, 80H, 80H, 80H, 80H, 80H, 80H, 80H
    DB	0ffH, 80H, 80H, 80H, 80H, 80H, 80H, 80H
    DB	0ffH, 80H, 80H, 80H, 80H, 80H, 80H, 80H
L$111:
    DB	0ffH, 0ffH, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	0ffH, 0ffH, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	0ffH, 0ffH, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	0ffH, 0ffH, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
    DB	80H, 0, 80H, 0, 80H, 0, 80H, 0
L$112:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	0ffH, 0ffH, 0ffH, 0ffH, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	0ffH, 0ffH, 0ffH, 0ffH, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
    DB	0ffH, 0ffH, 0ffH, 0ffH, 80H, 0, 0, 0
    DB	0EH DUP(80H,0,0,0,80H,0,0,0)
    DB	80H, 0, 0, 0, 80H, 0, 0, 0
L$113:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	06H DUP(0ffH,0,0,0,0,0,0,0)
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
L$114:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
L$115:
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH, 0ffH
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0ffH, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0
    DB	0, 0, 0, 0, 0, 0, 0, 0

_DATA		ENDS
		END
