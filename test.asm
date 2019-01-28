	processor	6502
	org	$0820

init:
	lda #$00
	sta $2a 			;clear song number
	tax
	tay

start:
	lda $2a 			;load song number
	jsr $dead 			;jump to init routine
	sei
	lda #$7f
	sta $dc0d
	sta $dd0d
	lda #$01
	sta $d01a
	lda #$1b
	ldx #$08
	ldy #$14
	sta $d011
	stx $d016
	sty $d018
	lda #<irq
	ldx #>irq
	ldy #$7e
	sta $0314
	stx $0315
	sty $d012
	lda $dc0d
	lda $dd0d
	asl $d019
	cli

loop1:
	lda #$27 			;value for 'n' key
	cmp $cb 			;was 'n' key pressed?
	bne nopress 		;no
	lda $02 			;yes! get max song number
	cmp $2a
	beq reset_songno		;set song number to 0 if max song number reached
	inc $2a

loop2:
	lda #$40 			;value for "no key pressed"
	cmp $cb 			;was the key released?
	bne loop2 			;no
	jmp start 			;yes!

nopress:
	jmp loop1

irq:
	jsr $ffff 			;jump to sid play routine
	asl $d019
	jmp $ea31

reset_songno:
	lda #$00
	sta $2a
	jmp loop2

