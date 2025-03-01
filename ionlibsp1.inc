;-----------------------------------------------------------
;	Filename:		iolibsp.inc
;	Long name:  	Ion Extended Libs for 83+/84+/SE
;	Author:			Kerm Martian aka Christopher Mitchell
;	Last Update:	September 14, 2021
;
;Please consult license.txt for the fair use agreement
;implicitly binding by you continuing to read past this
;point.  Close and delete this file if you do not agree
;to the terms of the agreement.
;-----------------------------------------------------------

; LCD changes made by CVSoft, 14 September 2021. 
; Removed the Nspire check and always run Nspire FastCopy

; The Built-in Ion Library Routines by Joe Wingbermuehle

#define TI83P_USE_FLASH_iDetect

iCheckInts1:
;Push a zero byte to the stack and pop it
	xor a
	push af
		pop af
	ld a,i
	ret pe
	;See if an interrupt triggered. If so, the byte on the stack will not be 0 anymore
	dec sp
	dec sp
	pop af
	add a,a
	ret z
	xor a
	ret

;-----> Return Ion iVersion information
; hl=Ion iVersion,a=Ion compatibility number
; d=routine compatability number
; e=number of routines
imVersion:
	ld	a,ION_COMPATIBILITY
	ld	hl,iVersion_MAJOR*256+iVersion_MINOR
	ld	de,LIBRARY_COMPATIBILITY*256+NUMBER_OF_LIBRARIES
	ret

;-----> Generate a iRandom number
; input b=upper bound
; ouput a=answer 0<=a<b
; all registers are preserved except: af and bc
imRandom:
	push	hl
	push	de
	ld	hl,(randData)
	ld	a,r
	ld	d,a
	ld	e,(hl)
	add	hl,de
	add	a,l
	xor	h
	ld	(randData),hl
	sbc	hl,hl
	ld	e,a
	ld	d,h
imRandomLoop:
	add	hl,de
	djnz	imRandomLoop
	ld	a,h
	pop	de
	pop	hl
	ret

;-----> Draw a sprite
; b=size of sprite
; l=yc
; a=xc
; ix holds pointer
imPutSprite:
	ld	e,l
	ld	h,$00
	ld	d,h
	add	hl,de
	add	hl,de
	add	hl,hl
	add	hl,hl
	ld	e,a
	and	$07
	ld	c,a
	srl	e
	srl	e
	srl	e
	add	hl,de
	ld	de,gbuf
	add	hl,de
imPutSpriteLoop1:
sml1:	ld	d,(ix)
	ld	e,$00
	ld	a,c
	or	a
	jr	z,imPutSpriteSkip1
imPutSpriteLoop2:
	srl	d
	rr	e
	dec	a
	jr	nz,imPutSpriteLoop2
imPutSpriteSkip1:
	ld	a,(hl)
	xor	d
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	xor	e
	ld	(hl),a
	ld	de,$0B
	add	hl,de
	inc	ix
	djnz	imPutSpriteLoop1
	ret

;-----> Draw a picture
;Input:	ix->sprite
;	a=x
;	l=y
;	b=height	(in pixels)
;	c=width		(in bytes, e.g. 2 would be 16)
;Output: nothing
; All registers are destroyed except bc', de', hl'
imLargeSprite:
	ld d,a
	call iCheckInts1
	push af
		ld a,d
		di
		ex	af,af'
		ld	a,c
		push	af
		ex	af,af'
		ld	e,l
		ld	h,$00
		ld	d,h
		add	hl,de
		add	hl,de
		add	hl,hl
		add	hl,hl
		ld	e,a
		and	$07
		ld	c,a
		srl	e
		srl	e
		srl	e
		add	hl,de
		ld	de,gbuf
		add	hl,de
imLargeSpriteLoop1:
		push	hl
imLargeSpriteLoop2:
		ld	d,(ix)
		ld	e,$00
		ld	a,c
		or	a
		jr	z,imLargeSpriteSkip1
imLargeSpriteLoop3:
		srl	d
		rr	e
		dec	a
		jr	nz,imLargeSpriteLoop3
imLargeSpriteSkip1:
		ld	a,(hl)
		xor	d
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		xor	e
		ld	(hl),a
		inc	ix
		ex	af,af'
		dec	a
		push	af
		ex	af,af'
		pop	af
		jr	nz,imLargeSpriteLoop2
		pop	hl
		pop	af
		push	af
		ex	af,af'
		ld	de,$0C
		add	hl,de
		djnz	imLargeSpriteLoop1
		pop	af
		pop af
	ret po
;#ifdef enablecn2eis
	ei
;#endif
	ret

;-----> Get pixel information
; input:	e=y coordinate
;		a=x coordinate
; output:	a holds data for pixel (e.g. %00100000)
;		hl->byte where pixel is on the gbuf
imGetPixel:
	ld	d,$00
	ld	h,d
	ld	l,e
	add	hl,de
	add	hl,de
	add	hl,hl
	add	hl,hl
	ld	de,gbuf
	add	hl,de
	ld	b,$00
	ld	c,a
	and	%00000111
	srl	c
	srl	c
	srl	c
	add	hl,bc
	ld	b,a
	inc	b
	ld	a,%00000001
imGetPixelLoop:
	rrca
	djnz	imGetPixelLoop
	ret

;-----> Copy the gbuf to the screen, guaranteed 
;Courtesy of Jim E.
;Input: nothing
;Output:graph buffer is copied to the screen, no matter the speed settings
;
;in f,(c) is an unofficial instruction.
;It must be noted that you cannot specify any other register. Only f works.
;You may have to add it in order for the routine to work.

 .addinstr IN	F,(C)	70ED	2	NOP	1

imFastCopy:
	call iCheckInts1
	push af
		di
		ld c,$10
imFastCopyNspire:
msetrowNspire:
		call lcd_busy_quick
		ld a,$80
		out ($10),a
		ld hl,gbuf
		ld de,12
		ld a,$20
mcolNspire:
		call lcd_busy_quick
		out ($10),a
		push af
			ld b,64
mrowNspire:
			call lcd_busy_quick
			ld a,(hl)
			out ($11),a
			add hl,de
			djnz mrowNspire
		pop af
		dec h
		dec h
		dec h
		inc hl
		inc a
		cp $2c
		jr nz,mcolNspire
imFastCopyAllFinish:
		pop af
	ret po
;#ifdef enablecn2eis
	ei
;#endif
	ret

; Critical timings:
;	command->command: 65
;	command->value  : 68
;	value  ->value  : 66
;	value  ->command: 67


;-----> iDecompress data
; input:
;	hl->compressed data
;	de->place to load data
;	b=length of compressed data
;	c=compression factor (1, 3, or 15)
; output:
;	level is iDecompressed
;	hl->next byte of compressed data
imDecompress:
	call iCheckInts1
	push af
		di
imDecompressLoop:
		push	bc
		ld	a,(hl)
		ex	af,af'
		ld	a,c
		ld	b,8
		cp	1
		jr	z,mdcmp1
		ld	b,4
		cp	3
		jr	z,mdcmp1
		ld	b,2
mdcmp1:	push	bc
		ld	a,c
		ld	b,1
		cp	1
		jr	z,mdcmp2
		inc	b
		cp	3
		jr	z,mdcmp2
		ld	b,4
mdcmp2:	ex	af,af'
mdcmp3:	rlca
		djnz	mdcmp3
		ld	b,a
		ex	af,af'
		ld	a,b
		and	c
		ld	(de),a
		inc	de
		pop	bc
		djnz	mdcmp1
		inc	hl
		pop	bc
		djnz	imDecompressLoop
		pop af
	ret po
;#ifdef enablecn2eis
	ei
;#endif
	ret
mputsApp:
	PUSH BC
	PUSH AF
	LD A,(winBtm)
	LD B,A ; B = bottom line of window
MPutS10:
	LD A,(HL) ; get a character of string name
	INC HL
	OR A ; end of string?
	SCF ; indicate entire string was
	; displayed
	JR Z, MPutS20 ; yes --->
	bcall(_PutC) ; display one character of string
	;
	LD A,(curRow) ; check cursor position
	CP B ; off end of window?
	JR C,MPutS10 ; no, display rest of string
MPutS20:
	POP BC ; restore A (but not F)
	LD A,B
	POP BC ; restore BC
	RET

imPutSpriteMask:
	ld	e,l
	ld	h,$00
	ld	d,h
	add	hl,de
	add	hl,de
	add	hl,hl
	add	hl,hl
	ld	e,a
	and	$07
	ld	c,a
	srl	e
	srl	e
	srl	e
	add	hl,de
	ld	de,gbuf
	add	hl,de
imPutSpriteLoop1Mask:
	ld	d,(ix)
	ld	e,$FF
	ld	a,c
	or	a
	jr	z,imPutSpriteSkip1Mask
imPutSpriteLoop2Mask:
	srl	d
	rr	e
	set 7,d
	dec	a
	jr	nz,imPutSpriteLoop2Mask
imPutSpriteSkip1Mask:
	ld	a,(hl)
	and	d
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	and	e
	ld	(hl),a
	push ix
	dec hl
	ld de,8
	add ix,de
imPutSpriteLoop1AND:
	ld	d,(ix)
	ld	e,$00
	ld	a,c
	or	a
	jr	z,imPutSpriteSkip1AND
imPutSpriteLoop2AND:
	srl	d
	rr	e
	dec	a
	jr	nz,imPutSpriteLoop2AND
imPutSpriteSkip1AND:
	ld	a,(hl)
	xor	d
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	xor	e
	ld	(hl),a

	pop ix
	ld	de,$0B
	add	hl,de
	inc	ix
	djnz	imPutSpriteLoop1Mask
	ret
	
; End of Ion Library Routines by Kerm Martian