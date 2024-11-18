;------------------------------------------------------
; Alarm System Assembler Program
; File: armed.asm
; Description: The Armed Module
; Author: Gilbert Arbez
; Date: Fall 2010
;------------------------------------------------------

        SWITCH code_section
ARMDELAY  equ 3000; 15000
BEEPDELAY equ 1000

;-------------------------------------------
; Subroutine: enableAlarm
; Parameters: none
; Returns: nothing
; Variables
;       input, codeValid, delayFlag - on stack
; Description:
;     Gets user to enter a valid alarm code to arm
;     the alarm system.  Delays 15 seconds (to allow user
;     to leave; the alarm can be disarmed during this period),
;     monitors for trigger events (a, b, c, d, e).  When 'a'
;     is detected, 15 seconds are allowed to enter an alarm code to
;     disarm the system; otherwise the alarm is triggered. For other
;     trigger events, the alarm is triggered instantly.  The alarm
;     system can be turned off after being triggered with an alarm
;     code.
;------------------------------------------------------
; Stack Usage:
OFFSET 0  ; Set up stack offset
EAL_INPUT      DS.B 1  ; Input from user
EAL_CODEVALID  DS.B 1 ; Indicates if the entered code is valid
EAL_DELAYFLAG  DS.B 1 ; Flag to indicate if delay is complete
EAL_VARSIZE:
EAL_PR_D	      DS.W 1 ; Preserve D register
EAL_RA         DS.W 1 ; Preserve return address

enableAlarm:  pshd
   leas  -EAL_VARSIZE,SP        ; Reserve space for input, code validity, and delay flag
   ;// Prompt user for a valid alarm code to arm the system
   ldd #CODEMSG                 ; Show message to prompt for code entry
   jsr [printf, PCr]
   movb #FALSE,EAL_CODEVALID,SP ; Set code validity to false
eal_while1:
   tst EAL_CODEVALID,SP         ; Keep checking until a valid code is entered
   bne eal_endwhile1            ; If code is valid, exit loop
   jsr readKey                  ; Get input from user
   stab EAL_INPUT,SP
   tba
   jsr checkCode                ; Check if the entered code is valid
   staa EAL_CODEVALID,SP
   bra eal_while1               ; Continue checking for valid code
eal_endwhile1:                   ; End of loop when valid code is entered

   ldd #ARMING                   ; Show message: "System is arming"
   jsr [printf,PCr]
   ; // Delay for 1500 ms, i.e., 15 seconds
   ldd #ARMDELAY                 ; Set delay duration to 15 seconds
   jsr setDelay
   movb #FALSE,EAL_CODEVALID,SP ; Reset code validity
   movb #FALSE,EAL_DELAYFLAG,SP ; Reset delay flag
eal_while2:                     ; Wait until the delay is over
   tst EAL_DELAYFLAG,SP         ; Check if delay is finished
   bne eal_endwhile2            ; If delay is complete, exit loop
   jsr polldelay                ; Check if delay is still active
   staa EAL_DELAYFLAG,SP
   jsr pollreadKey              ; Check for input to disarm
   stab EAL_INPUT,SP
   tba
eal_if1:
   cmpa #'#'                    ; Check if the input is a number or the '#' symbol
   beq eal_blk1
   jsr isdigit
   tsta
   bne eal_blk1                 ; If input is not a number, ignore
   bra eal_endif1   
eal_blk1:                       ; If input is valid, check the code
   ldaa EAL_INPUT,SP            
   jsr checkCode                ; Validate entered code
   staa EAL_CODEVALID,SP
eal_if2:
   tsta                         ; Check if code is valid
   beq eal_endif2
   movb #TRUE,EAL_DELAYFLAG,SP  ; Set delay flag to true to break loop
eal_endif2:
eal_endif1:                     ; End of if condition
   bra eal_while2                ; Continue monitoring for input
eal_endwhile2:                  ; End of delay check loop

   ;// Monitor for trigger events and code input to disable alarm
   ;// Code is valid if entered during delay
eal_if3:			; If code is not valid
   tst EAL_CODEVALID,SP
   bne eal_endif3
   ldd #ARMED                   ; Show message: "System is armed"
   jsr [printf, PCr]
eal_endif3:
eal_while3:                      ; Keep checking until a valid code is entered
   tst EAL_CODEVALID,SP         ; Check if code is valid
   bne eal_endwhile3            ; Exit loop if code is valid
   jsr pollReadKey              ; Get input from user
   stab EAL_INPUT,SP
   tba
eal_elseif:
   cmpa #'#'                    ; Check if input is a number or the '#' symbol
   beq eal_blk2
   jsr isdigit
   tsta
   bne eal_blk2                 ; Ignore if input is not valid
   bra eal_elseifA  
eal_blk2:      
   ldaa EAL_INPUT,SP             ; Check the input code
   bsr checkCode                ; Validate the code
   staa EAL_CODEVALID,SP
   bra eal_endelseif    
eal_elseifA:
   jsr getSwStatus               ; Check if front door is opened
   anda #%00000001
   beq eal_elseifB               ; If not opened, check other conditions
   ldd #DISARMING                ; Show message: "System is disarming"
   jsr [printf, PCr]
   ldd #ARMDELAY                 ; Set delay for disarming
   jsr setDelay
eal_while4:                      ; Wait for valid code to disarm
   ldaa EAL_CODEVALID,SP         ; Check if valid code entered
   bne eal_endwhile4 
eal_if4:
   jsr pollDelay                 ; If delay occurred, check if alarm should trigger
   tsta  
   beq eal_else4                 ; If no alarm, continue checking
   jsr triggerAlarm              ; Trigger alarm if condition met
   movb #TRUE,EAL_CODEVALID,SP   ; Mark code as valid
   bra eal_endif4                ; Exit loop
eal_else4:                        ; Otherwise, check for input again
   jsr pollReadKey               ; Get input for disarming
   stab EAL_INPUT,SP
eal_if5:
   tba                           ; Check if input is a number or '#' symbol
   cmpa #'#' 
   beq eal_blk3
   jsr isdigit
   tsta
   bne eal_blk3
   bra eal_endif5   
eal_blk3:  
   ldaa EAL_INPUT,SP
   bsr checkCode                ; Validate entered code
   staa EAL_CODEVALID,SP 
eal_endif5:
eal_endif4:                     ; End of condition for disarming
   bra eal_while4                ; Continue checking
eal_endwhile4:                  ; End of disarming loop
eal_endelseifA                  ; End of door status check
   bra eal_endelseif;      
eal_elseifB:
   jsr getSwStatus              ; Check if other doors/windows are opened
   anda #%11111110              
   beq eal_endelseifB           ; If none opened, skip alarm
   jsr triggerAlarm             ; Trigger alarm if door/window opened
   movb #TRUE,EAL_CODEVALID,SP  ; Mark code as valid
eal_endelseifB:                 ; End of door/window check
   ;// Ignore any other input
eal_endelseif:
    bra eal_while3              ; Continue checking for valid code
eal_endwhile3:                   ; End of monitoring for valid code
    leas EAL_VARSIZE,SP          ; Clean up the stack
    puld
    rts                          ; Return from function



;------------------------------------------------------
; Subroutine: checkCode
; Parameters: input - accumulator A
; Returns: TRUE when a valid alarm code is detected, FALSE otherwise - stored in 
;          accumulator A
; Local Variables: retval - on stack
; Global Variables:
;       mult - initilased to 1000 in inithw (Alarm System Module)
;       alarmCode - initialised to 0 in inithw (Alarm System Module)
; Descriptions: Creates alarm code using digits entered until
;               4 digits are seen. After 4th digit, see if
;               alarm code is valid using isCodeValid().
;------------------------------------------------------
; Stack usage
OFFSET 0
CKC_INPUT  DS.B 1 ; input parameter
CKC_RETVAL DS.B 1 ; variable to store the return value (retval)
CKC_VARSIZE
CKC_PR_B	  DS.B 1 ; preserve register B
CKC_PR_X   DS.W 1 ; preserve register X
CKC_PR_Y   DS.W 1 ; preserve register Y
CKC_RA	  DS.W 1 ; store return address

checkCode: pshy
   pshx
   pshb
   leas -CKC_VARSIZE,SP       
                              ; declare a static variable 'mult' with initial value 1000; multiplier for the digit
                              ; declare 'alarmCode' variable with initial value 0; holds the alarm code value
   movb #FALSE,CKC_RETVAL,SP  ; set retval to FALSE
   staa CKC_INPUT,SP          ; store the input value
   
   jsr isdigit                ; check if the input is a digit
   tsta
   beq ckc_else               ; if it's not a digit, jump to the else block
   ldaa CKC_INPUT,SP          ; if it's a digit, update the alarm code 
   suba #ASCII_CONV_NUM       ; convert the input to an integer by subtracting ASCII offset
   tab                        
   clra
   ldy mult
   emul                       ; multiply the result of the conversion by the multiplier (mult) and store in D register
   addd alarmCode
   std alarmCode
   ldd mult                   ; load the multiplier
   ldx #10
   idiv                       ; divide the multiplier by 10
   stx mult                   ; store the new multiplier value
   ldd mult                   ; check if the multiplier has reached 0
   bne ckc_endif1             ; if the multiplier is not 0, skip the next step
   ldd alarmcode              ; check if the alarm code is valid
   bsr isCodeValid            ; call isCodeValid to validate the alarm code
   staa CKC_RETVAL,SP         ; store the result of the validation (TRUE or FALSE) in retval
   ldd #0                     ; reset alarm code to 0
   std alarmCode 
   ldd #1000                  ; reset the multiplier to 1000
   std mult     
ckc_endif1:                   ; end of multiplier check
   bra ckc_endif              ; jump to the end

ckc_else:                     ; if input is not a digit
   ldd #0                     ; reset alarm code to 0
   std alarmCode 
   ldd #1000                  ; reset multiplier to 1000
   std mult                   ; reset multiplier to 1000

ckc_endif:                     ; end of check

   ldaa CKC_RETVAL,SP         ; return retval
   ; Restore registers and stack
   leas CKC_VARSIZE,SP
   pulb
   pulx
   puly
   rts
   


;------------------------------------------------------
; Subroutine: isCodeValid
; Parameters: alarmCode stored in register D
; Local Variables
;   ptr - pointer to array - in register X
;   cnt, retval - on the stack.
; Returns: TRUE/FALSE - Returned in accumulator A 
; Description: Checks to see if alarm code is in the
;              alarmCodes array.
;------------------------------------------------------

; Stack usage
OFFSET 0
CDV_ALARMCODE   DS.W 1 ; variable to store the alarm code
CDV_CNT    DS.B 1 ; counter for the number of alarm codes to check
CDV_RETVAL DS.B 1 ; variable to store the return value (retval)
CDV_VARSIZE:
CDV_PR_X   DS.W 1 ; preserve register X
CDV_RA     DS.W 1 ; store return address

isCodeValid: pshx
   leas -CDV_VARSIZE,SP
   std CDV_ALARMCODE,SP
   ; declare a pointer to alarmCodes (pointer 'ptr' to the array of alarm codes)
   movb #NUMCODES,CDV_CNT,SP ; set the counter to 5 (the number of alarm codes)
   movb #FALSE,CDV_RETVAL,SP ; set retval to FALSE initially
   ldx #alarmCodes           ; set pointer 'ptr' to the address of alarmCodes
cdv_while                    ; start the loop
   ldd 2,X+                  ; load the current alarm code from the array (pointed by 'ptr')
   cpd CDV_ALARMCODE,SP      ; compare the current alarm code with the input alarmCode
   bne cdv_endif             ; if they are not equal, skip to 'endif'
   movb #TRUE,CDV_RETVAL,SP  ; if they are equal, set retval to TRUE
   bra cdv_endwhile          ; exit the loop
cdv_endif:                   ; end of comparison check
   dec CDV_CNT,SP            ; decrease the counter by 1
   bne cdv_while             ; if counter is not zero, continue the loop
cdv_endwhile:
   ldaa CDV_RETVAL,SP         ; return the value of retval
   ; restore registers and stack
   leas CDV_VARSIZE,SP 
   pulx
   rts


;----------------------------
; Subroutine: triggerAlarm
; Parameters: none
; Returns: nothing
; Local Variables: retval, dobel, input - on stack
; Description: Repeatedly sends a bel character to the
;              terminal until a valid alarm code is entered.
;----------------------------
; Stack usage
	OFFSET 0
TRA_DONE   DS.W 1 ; done
TRA_DONEINPUT  DS.B 1 ; doneInput
TRA_INPUT  DS.B 1 ; input
TRA_VARSIZE:
TRA_PR_D   DS.W 1 ; preserve D register
TRA_RA     DS.W 1 ; return address

triggerAlarm: pshd
   leas -TRA_VARSIZE,SP
   movb #FALSE,TRA_DONE,SP   ; byte done = FALSE;
                             ; byte dobel;
                             ; byte input;
tra_while:                   ; while(!done)
   tst TRA_DONE,SP         ; {
   bne tra_endwhile
   ldb #BEL                  ;    putchar(BEL);
   clra
   jsr [putchar, PCr]
   ldd #BEEPDELAY            ;    setDelay(BEEPDELAY);   // 1 sec between beeps
   jsr setDelay
   movb #FALSE,TRA_DONEINPUT,SP ; doneInput = FALSE;
tra_while2:                  ;    while(!doneInput) 
   tst TRA_DONEINPUT,SP      ;    {
   bne tra_endwhile2
   jsr polldelay             ;       doneInput = delay();
   staa TRA_DONEINPUT,SP
   jsr pollReadKey           ;       input = pollReadKey();  // check if code entered to disarm
   stab TRA_INPUT,SP
   tba
   cmpa #'#'                 ;       if(isdigit(input) || input == '#') 
   beq tra_blk1
   jsr isdigit
   tsta
   bne tra_blk1
   bra tra_endif   
tra_blk1:
   ldaa TRA_INPUT,SP
   jsr checkCode             ;                done = checkCode(input);
   staa TRA_DONE,SP 
tra_endif: 
   bra tra_while2
tra_endwhile2                ;    }
   bra tra_while
tra_endwhile                 ; }
   leas TRA_VARSIZE,SP
   puld
   rts

;-------------------------------------------------------------------------
; Data Structures
;-------------------------------------------------------------------------
	SWITCH globalConst
; Message strings
CODEMSG dc.b  "Code",NL,CR,$00
ARMING  dc.b  "*** Arming ***",NL,CR,$00
ARMED	dc.b  "*** Armed ***",NL,CR,$00
DISARMING dc.b  "-- Disarming --",NL,CR,$00

   switch globalVar
mult dc.w 1000
alarmCode dc.w 0