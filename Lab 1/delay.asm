;------------------------------------------------------
; Alarm System Simulation Assembler Program
; File: delay.asm
; Description: The Delay Module
; Author: Gilbert Arbez
; Date: Fall 2010
;------------------------------------------------------

; Some definitions
	SWITCH code_section

;------------------------------------------------------
; Subroutine setDelay
; Parameters: cnt - accumulator D
; Returns: nothing
; Global Variables: delayCount
; Description: Initializes the delayCount variable.
;------------------------------------------------------
setDelay: 
   std delayCount       ; store the value in accumulator D into delayCount
   rts                  ; return from subroutine

;------------------------------------------------------
; Subroutine: polldelay
; Parameters: none
; Returns: TRUE when delay counter reaches 0 - in accumulator A
; Local Variables
;   retval - acc A, cntr - X register
; Global Variables:
;   delayCount
; Description: The subroutine delays for 1 ms, decrements delayCount.
;              If delayCount is zero, return TRUE; FALSE otherwise.
;   Core Clock is set to 24 MHz, so 1 cycle is 41 2/3 ns
;   NOP takes up 1 cycle, thus 41 2/3 ns
;   Need 24 cycles to create a 1 microsecond delay
;   8 cycles create a 333 1/3 nano delay
;	DEX - 1 cycle
;	BNE - 3 cycles - when branch is taken
;	Need 4 NOP
;   Run Loop 3000 times to create a 1 ms delay   
;------------------------------------------------------
; Stack Usage:
	OFFSET 0  ; to setup offset into stack
PDLY_VARSIZE:
PDLY_PR_Y   DS.W 1 ;preserve Y register
PDLY_PR_X   DS.W 1 ;preserve X register
PDLY_PR_B   DS.B 1 ;preserve B register
PDLY_RA     DS.W 1 ;store return address

polldelay: 
   pshb       ; push register B onto the stack to save its value
   pshx       ; push register X onto the stack to save its value
   pshy      ; push register Y onto the stack to save its value

   ldy #delayCount ; load the address of delayCount into Y register

whileStart:
   nop         ; no operation, 1 cycle
   nop         ; no operation, 1 cycle
   nop         ; no operation, 1 cycle
   nop        ; no operation, 1 cycle
   dey          ; decrement Y register (delayCount)
   bne whileStart  ; if delayCount is not zero, continue looping

   ldx delayCount  ; load the value of delayCount into X register
   dex             ; decrement X register
   beq complete   ; if delayCount reaches zero, jump to 'complete'
   lda #FALSE     ; load FALSE into accumulator A
   bra return      ; branch to 'return' to exit subroutine

complete:
   lda #TRUE       ; load TRUE into accumulator A if delay is complete

return:
   stx delayCount  ; store the updated value of X back into delayCount

   ; restore registers and stack
   puly          ; restore Y register from the stack
   pulx            ; restore X register from the stack
   pulb            ;restore B register from the stack
   rts             ;return from subroutine

;------------------------------------------------------
; Global variables
;------------------------------------------------------
   switch globalVar
delayCount ds.w 1   ; 2-byte delay counter to store delay count value
