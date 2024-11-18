;----------------------------------------------------------------------
; File: Keypad.asm
; Author:

; Description:
;  This contains the code for reading the
;  16-key keypad attached to Port A
;  See the schematic of the connection in the
;  design document.
;
;  The following subroutines are provided by the module:
;
; char pollReadKey(): Polls the keypad for a key press
;                    Checks the keypad for 2 ms for a keypress, and
;                    returns NOKEY if no keypress is detected, otherwise
;                    returns the ASCII code for the key, e.g., 0-9, *, #, A-D
; void initkey(): Initializes Port A for the keypad
;
; char readKey(): Reads the key on the keypad
;                The value returned corresponds to the ASCII code for the key, 
;                e.g., 0-9, *, #, A-D
;----------------------------------------------------------------------

; Include header files
include "sections.inc"
include "reg9s12.inc"  ; Defines EQU's for Peripheral Ports

**************EQUATES**********

;-----Conversion table
NUMKEYS  EQU  16  ; Total number of keys on the keypad
BADCODE  EQU  $FF  ; Value returned if translation is unsuccessful
NOKEY    EQU  $00  ; Value returned when no key is pressed during polling
POLLCOUNT EQU 1    ; Number of loops to create 1 ms poll time

COLS   EQU PORTA
ROWS   EQU PORTA
LED    EQU PORTB
ROW0   EQU %00010000
ROW1   EQU %00100000
ROW2   EQU %01000000
ROW3   EQU %10000000
COLM   EQU %00001111
ROWM   EQU %11110000

PDELAY  RMB 1

KCODE0  FCB $31,$32,$33,$41    ; "123A"
KCODE1  FCB $34,$35,$36,$42    ; "456B"
KCODE2  FCB $37,$38,$39,$43    ; "789C"
KCODE3  FCB $2A,$30,$23,$44    ; "*0#D"
SWITCH  globalConst  ; Constant data

SWITCH  code_section  ; Place in the code section

;-----------------------------------------------------------	
; Subroutine: initKeyPad
; Description: 
;  Initializes PORT A for keypad interaction
;-----------------------------------------------------------	
initKeyPad:        
   CLR   DDRA               ; Set all pins on Port A to input
   LDAA  #$FF              ; Set all bits to high
   LDAA  #$F0              ; Set high nibble to output, low nibble to input
   STAA  DDRA              ; Apply the settings to Port A
   LDAA  #$0F              ; Set PORT P for column scanning
   STAA  DDRP

   rts                      ; Return from subroutine

;-----------------------------------------------------------	
; Subroutine: ch <- pollReadKey
; Parameters: none
; Local variable:
; Returns:
;       ch: NOKEY if no key pressed,
;       otherwise, ASCII code of the key in accumulator B
; Description:
;  Loops for 2 ms to check if a key is pressed. If a key is detected
;  (and debounce is successful), the ASCII code of the key is returned.
;-----------------------------------------------------------

pollReadKey:
   LDAA  #ROWM        ; Set all rows high
   STAA  ROWS         ; Activate rows
   LDAA  COLS         ; Read Port A to check the columns
   ANDA  #COLM        ; Mask out the row bits
   CMPA  #$00         ; Check if any column is active
   LBNE  pollReadKey  ; If no key is pressed, continue polling

debounce:
   LDD   #1           ; Set delay counter
   JSR   delayms      ; Call 15ms delay subroutine
   LDAA  COLS         ; Read Port A again
   ANDA  #COLM        ; Mask out the row bits
   CMPA  #$00         ; Check if any key is pressed
   BNE   OVER         ; If a key is detected, go to OVER
   BRA   debounce     ; Otherwise, keep checking for key press

OVER:
   JSR   delayms      ; Apply debounce delay
   LDAA  COLS         ; Read Port A
   ANDA  #COLM        ; Mask out the row bits
   CMPA  #$00         ; Check for a key press after debounce
   BNE   OVER1        ; If key is pressed, proceed to process key
   LBRA  debounce     ; If no key press, loop back to debounce

OVER1:
   LDAA  #ROW0        ; Set the first row high
   STAA  ROWS         ; Activate the row
   MOVB  #$08, PDELAY ; Set delay for stability
   LBRA readKey       ; Call readKey to determine which key was pressed
   rts                 ; Return from subroutine

;-----------------------------------------------------------	
; Subroutine: ch <- readKey
; Arguments: none
; Local variable: 
;	ch - ASCII code of the key pressed (in accumulator B)
; Description:
;  Main subroutine that reads the key from the keypad by checking each row.
;  If a key is pressed, the corresponding ASCII value is returned.
;-----------------------------------------------------------	

readKey:
   DEC   PDELAY       ; Decrement delay counter
   LBNE  readKey      ; Keep decrementing until count is zero
   LDAA  COLS         ; Read Port A
   ANDA  #COLM        ; Mask out the row bits
   CMPA  #$00         ; Check if any column is active
   BNE   R0           ; If column is active, key is in row 0
   LDAA  #ROW1        ; If no key in row 0, set row 1 high
   STAA  ROWS         ; Activate row 1
   MOVB  #$08, PDELAY ; Set short delay for stabilization
P2:                   ; Wait for stability
   DEC   PDELAY       ; Decrement delay counter
   BNE   P2           ; Repeat until delay count is zero
   LDAA  COLS         ; Read Port A
   ANDA  #COLM        ; Mask out the row bits
   CMPA  #$00         ; Check for key press
   BNE   R1           ; If key pressed, it's in row 1

   LDAA  #ROW2        ; If no key in row 1, set row 2 high
   STAA  ROWS         ; Activate row 2
   MOVB  #$08, PDELAY ; Set short delay
P3:                   ; Wait for stability
   DEC   PDELAY       ; Decrement delay
   BNE   P3           ; Repeat until delay count is zero
   LDAA  COLS         ; Read Port A
   ANDA  #COLM        ; Mask out row bits
   CMPA  #$00         ; Check for key press
   BNE   R2           ; If key pressed, it's in row 2

   LDAA  #ROW3        ; If no key in row 2, set row 3 high
   STAA  ROWS         ; Activate row 3
   MOVB  #$08, PDELAY ; Set short delay
P4:                   ; Wait for stability
   DEC   PDELAY       ; Decrement delay counter
   BNE   P4           ; Repeat until delay count is zero
   LDAA  COLS         ; Read Port A
   ANDA  #COLM        ; Mask out row bits
   CMPA  #$00         ; Check for key press
   BNE   R3           ; If key pressed, it's in row 3
   LBRA   debounce    ; No key found, restart debounce

R0:                   ; Key is in row 0
   LDX   #KCODE0      ; Load pointer to row 0 ASCII codes
   BRA   FIND         ; Go to find column

R1:                   ; Key is in row 1
   LDX   #KCODE1      ; Load pointer to row 1 ASCII codes
   BRA   FIND         ; Go to find column

R2:                   ; Key is in row 2
   LDX   #KCODE2      ; Load pointer to row 2 ASCII codes
   BRA   FIND         ; Go to find column

R3:                   ; Key is in row 3
   LDX   #KCODE3      ; Load pointer to row 3 ASCII codes
   BRA   FIND         ; Go to find column

FIND:                 ; Search for the key's column
   ANDA  #COLM        ; Mask out row bits
   COMA               ; Invert column
SHIFT:
   LSRA               ; Logical shift right
   BCC   MATCH        ; If carry clear, column is found
   INX                ; If carry set, increment pointer to next column
   BRA   SHIFT        ; Shift right until column is found

MATCH:
   LDAB  0,X          ; Load the corresponding ASCII code from row array
   rts                 ; Return from subroutine
