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
   STD delayCount         ; Store D register value in delayCount
   RTS                    ; Return from subroutine


;------------------------------------------------------
; Subroutine: polldelay
; Parameters: none
; Returns: TRUE when delay counter reaches 0 - in accumulator A
; Global Variables: delayCount
; Description: Delays for 1 ms, decrements delayCount.
;              If delayCount is zero, return TRUE; FALSE otherwise.
;------------------------------------------------------
polldelay: 
   PSHB                   ; Preserve B register
   PSHX                   ; Preserve X register
   PSHY                   ; Preserve Y register

   LDY delayCount         ; Load delayCount into Y register

   ; Poll loop
whileStart:
   NOP                    ; Small delay
   NOP
   NOP
   NOP
   DEY                    ; Decrement Y (delay count)
   BNE whileStart         ; If Y not zero, repeat

   LDX delayCount         ; Load delayCount into X
   DEX                    ; Decrement X
   BEQ complete           ; If X reaches zero, delay is complete
   LDAA #FALSE            ; Return FALSE (0)
   BRA return

complete:
   LDAA #TRUE             ; Return TRUE (1) if delay is complete

return:
   STX delayCount         ; Store updated delayCount

   PULY                   ; Restore Y register
   PULX                   ; Restore X register
   PULB                   ; Restore B register
   RTS                    ; Return from subroutine


;------------------------------------------------------
; Subroutine: delayms
; Parameters: num (D register) - number of milliseconds
; Returns: nothing
; Global Variables: delayCount
; Description: Creates a delay of "num" milliseconds
;              by calling REALDELAY for each ms.
;------------------------------------------------------
delayms: 
   PSHB                   ; Push register B onto the stack
   PSHX                   ; Push register X onto the stack
   PSHY                   ; Push register Y onto the stack
   PSHD                   ; Push register D onto the stack

   STD delayCount         ; Store the value of num (passed in D) into delayCount
   
   ; Begin loop to create "num" milliseconds of delay
delayms_loop:
   JSR REALDELAY          ; Call REALDELAY subroutine to create 1 ms delay
   LDD delayCount         ; Load the current value of delayCount
   SUBD #1                ; Decrement the delay counter (D = D - 1)
   STD delayCount         ; Store the updated delayCount back into memory
   BNE delayms_loop       ; If delayCount is not zero, repeat loop

   ; Restore the registers
   PULD                   ; Pull register D from the stack
   PULY                   ; Pull register Y from the stack
   PULX                   ; Pull register X from the stack
   PULB                   ; Pull register B from the stack
   RTS                    ; Return from subroutine


;------------------------------------------------------
; Subroutine: REALDELAY
; Description: Creates a delay of 1 ms by using nested loops.
; Global Variables: DR15mH, DR15mL
;------------------------------------------------------
REALDELAY:                
   LDAA  #D15mH           ; Load first layer delay count (high byte)
   STAA  DR15mH           ; Store in DR15mH

D2:
   LDAA  #D15mL           ; Load second layer delay count (low byte)
   STAA  DR15mL           ; Store in DR15mL

D1:
   DEC   DR15mL           ; Decrement second layer delay count
   NOP                    ; Short delay
   NOP
   BNE   D1               ; If DR15mL is not zero, repeat second layer

   DEC   DR15mH           ; Decrement first layer delay count
   BNE   D2               ; If DR15mH is not zero, repeat first layer
   RTS                    ; Return when both layers reach zero


;------------------------------------------------------
; Global variables
;------------------------------------------------------
	SWITCH globalVar

delayCount  DS.W 1        ; 2-byte delay counter

; Constants for REALDELAY (for 1 ms delay)
D15mH  EQU 17             ; First layer count
D15mL  EQU 250            ; Second layer count

DR15mH  RMB 1             ; Register for first layer
DR15mL  RMB 1             ; Register for second layer