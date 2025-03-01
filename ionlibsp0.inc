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

iCheckInts0:
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
iVersion:
	ld	a,ION_COMPATIBILITY
	ld	hl,iVersion_MAJOR*256+iVersion_MINOR
	ld	de,LIBRARY_COMPATIBILITY*256+NUMBER_OF_LIBRARIES
	ret

;-----> Generate a iRandom number
; input b=upper bound
; ouput a=answer 0<=a<b
; all registers are preserved except: af and bc
iRandom:
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
iRandomLoop:
	add	hl,de
	djnz	iRandomLoop
	ld	a,h
	pop	de
	pop	hl
	ret

;-----> Draw a sprite
; b=size of sprite
; l=yc
; a=xc
; ix holds pointer
iPutSprite:
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
iPutSpriteLoop1:
sl1:	ld	d,(ix)
	ld	e,$00
	ld	a,c
	or	a
	jr	z,iPutSpriteSkip1
iPutSpriteLoop2:
	srl	d
	rr	e
	dec	a
	jr	nz,iPutSpriteLoop2
iPutSpriteSkip1:
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
	djnz	iPutSpriteLoop1
	ret

;-----> Draw a picture
;Input:	ix->sprite
;	a=x
;	l=y
;	b=height	(in pixels)
;	c=width		(in bytes, e.g. 2 would be 16)
;Output: nothing
; All registers are destroyed except bc', de', hl'
iLargeSprite:
	ld d,a
	call iCheckInts0
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
iLargeSpriteLoop1:
			push	hl
iLargeSpriteLoop2:
				ld	d,(ix)
				ld	e,$00
				ld	a,c
				or	a
				jr	z,iLargeSpriteSkip1
iLargeSpriteLoop3:
				srl	d
				rr	e
				dec	a
				jr	nz,iLargeSpriteLoop3
iLargeSpriteSkip1:
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
				jr	nz,iLargeSpriteLoop2
				pop	hl
			pop	af
		push	af
			ex	af,af'
			ld	de,$0C
			add	hl,de
			djnz	iLargeSpriteLoop1
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
iGetPixel:
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
iGetPixelLoop:
	rrca
	djnz	iGetPixelLoop
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

iFastCopy:
	call iCheckInts0
	push af
		di
		ld c,$10
iFastCopyNspire:
setrowNspire:
		call lcd_busy_quick
		ld a,$80
		out ($10),a
		ld hl,gbuf
		ld de,12
		ld a,$20
colNspire:
		call lcd_busy_quick
		out ($10),a
		push af
			ld b,64
rowNspire:
			call lcd_busy_quick
			ld a,(hl)
			out ($11),a
			add hl,de
			djnz rowNspire
		pop af
		dec h
		dec h
		dec h
		inc hl
		inc a
		cp $2c
		jr nz,colNspire
iFastCopyAllFinish:
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

;-----> iDetect a file
; input:
;	hl=place to start looking
;	ix->first line of data (0 terminated)
; output:
;	de=place stopped + 1
;	hl->program data (after the string)
;	z=0 if found, z=1 if not found.
;	All registers destroyed
iDetect: ld	de,(ptemp)
	bcall(_cphlde)
	ld	a,(hl)
	jr	nz,iDetectContinue
	inc	a
	ret
iDetectContinue:
	push	hl
	and	$01
	jr	nz,iDetectSkip

	dec	hl
	dec	hl
	dec	hl	; hl->lsb ptr
	ld	e,(hl)
	dec	hl
	ld	d,(hl)
	dec	hl	; hl->page
	ld	a,(hl)
	or	a
	push	af
	ld	h,d
	ld	l,e	; hl & de->program
	jr	z,iDetectNoMove
	push	hl
	bcall(_memfree)
	ld	bc,64
	sbc	hl,bc
	pop	hl
	jr	c,iDetectNotEnough
	ld	de,($9820)
	push	ix
	push	hl
	push	de
	bcall(_flashToRam)
	pop	hl
	push	hl
	pop	ix
	ld	a,10
	add	a,(ix+9)
	ld	e,a
	ld	d,0	; de=flash offset
	add	hl,de
	ex	(sp),hl
	add	hl,de
	pop	de
	ex	de,hl	; hl-temp, de-perm
	pop	ix
iDetectNoMove:
	inc	de
	inc	de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	inc	hl	; hl->data in ram
	push	bc
	push	ix
	pop	bc
iDetectCheck:
	ld	a,(bc)
	or	a
	jr	z,iDetectFound
	cp	(hl)
	inc	bc
	inc	de
	inc	hl
	jr	z,iDetectCheck
iDetectBad:
	pop	bc
iDetectNotEnough:
	pop	af
iDetectSkip:
	pop	hl
	ld	bc,-6
	add	hl,bc
	ld	b,(hl)
	dec	hl
iDetectNameLoop2:
	dec	hl
	djnz	iDetectNameLoop2
	jr	iDetect
iDetectFound:
	pop	hl
	; hl=size, de->data
	pop	af	; a=page, f=(or a)
	jr	z,iDetectInRam
	push	de	; data
	push	af
	push	hl
	bcall(_enoughRam)
	pop	bc
	jr	c,iDetectBad
	pop	af
	pop	hl
	ld	de,($9820)	; tempMem
	push	de
	bcall(_flashToRam)
	pop	de
iDetectInRam:	; de->data in RAM
	pop	hl	; hl->vat location
	ld	bc,-6
	add	hl,bc
	ld	b,(hl)
	inc	b
iDetectNameLoop1:
	dec	hl
	djnz	iDetectNameLoop1
	ex	de,hl
	xor	a
	ret

;-----> iDecompress data
; input:
;	hl->compressed data
;	de->place to load data
;	b=length of compressed data
;	c=compression factor (1, 3, or 15)
; output:
;	level is iDecompressed
;	hl->next byte of compressed data
iDecompress:
	call iCheckInts0
	push af
		di
iDecompressLoop:
		push	bc
		ld	a,(hl)
		ex	af,af'
		ld	a,c
		ld	b,8
		cp	1
		jr	z,dcmp1
		ld	b,4
		cp	3
		jr	z,dcmp1
		ld	b,2
dcmp1:	push	bc
		ld	a,c
		ld	b,1
		cp	1
		jr	z,dcmp2
		inc	b
		cp	3
		jr	z,dcmp2
		ld	b,4
dcmp2:	ex	af,af'
dcmp3:	rlca
		djnz	dcmp3
		ld	b,a
		ex	af,af'
		ld	a,b
		and	c
		ld	(de),a
		inc	de
		pop	bc
		djnz	dcmp1
		inc	hl
		pop	bc
		djnz	iDecompressLoop
		pop af
	ret po
;#ifdef enablecn2eis
	ei
;#endif
	ret

; End of Ion Library Routines by Joe Wingbermuehle
; Ion Extended Routines by Joe Wingbermuehle and Kerm Martian

iPutSpriteMask:
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
iPutSpriteLoop1Mask:
	ld	d,(ix)
	ld	e,$FF
	ld	a,c
	or	a
	jr	z,iPutSpriteSkip1Mask
iPutSpriteLoop2Mask:
	srl	d
	rr	e
	set 7,d
	dec	a
	jr	nz,iPutSpriteLoop2Mask
iPutSpriteSkip1Mask:
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
iPutSpriteLoop1AND:
	ld	d,(ix)
	ld	e,$00
	ld	a,c
	or	a
	jr	z,iPutSpriteSkip1AND
iPutSpriteLoop2AND:
	srl	d
	rr	e
	dec	a
	jr	nz,iPutSpriteLoop2AND
iPutSpriteSkip1AND:
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
	djnz	iPutSpriteLoop1Mask
	ret
	
; End of Ion Library Routines by Kerm Martian