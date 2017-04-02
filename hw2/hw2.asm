;; Name: Stefan Cao
;; Assignment: Homework 2
;; Status: Works!

	ORG 0H
	SJMP MAIN 	;; jump to MAIN

;; defining strings
STRING1: 
	DB "your test string"
	DB 0 ;; Null termination
STRING1LEN: 
	DB 16 ;; String length follows the null

STRING2:
	DB 0 ;; Null termination
STRING2LEN: 
	DB 0 ;; String length follows the null
	
STRING3:
	DB "this is last string"
	DB 0 ;; Null termination
STRING3LEN: 
	DB 19 ;; String length follows the null


;; Function STRCPY
	;; source string pointer in DPTR
	;; destination string pointer in R0
	;; return value in A
STRCPY:

	MOV R7, #0	;; Using R7 as len variable
	
	;; copy R0 to R1 because don't want to modify R0
	MOV A, R0
	MOV R1, A
	
	check_condition:

		MOV A, R7	;; putting len into A
		
		MOVC A, @A+DPTR ;; getting content of s[len]
	
		CJNE A, #0, Copy_loop	;; if not equal to 0 (means not Null termination), 
								;; then copy over to destination in RAM	(R0)
								
		SJMP return_len	;; if Null termination, return the len and jump back to caller
	
	;; copying content from ROM (base address DPTR) to RAM (base address R0)
	Copy_loop:

		MOV @R1, A	;; sd[len]=s[len] (remember that R0=R1)
	
		INC R1		;; increment R1 (len)
		INC R7		;; increment len

		SJMP check_condition	;; jump back to check_condition

	;; returning len and go back to caller (TESTSTRING)
	return_len:
		MOV A, R7	;; moving len (in R7) to A
		RET

;; Function TESTSTRING
TESTSTRING:
	ACALL STRCPY	;; call STRCPY
	
	MOV R6, A	;; copy A to R6 -- (len) 
	MOV R5, A	;; copy A to R5 -- (len) this one is used to return later
	
	INC A		;; increment A to get len+1
	
	MOVC A, @A+DPTR	;; expectedLen

	;; compare with XRL
	XRL A, R6		;; if they are the same then should be 0
	CJNE A, #0, Error	;; check if len==expectedLen
	
	;; check if len is 0, if so return
	CJNE R6, #0, continue
	MOV A, R5	;; returning len
	RET

continue:	
	;; copy R0 to R5
	MOV A, R0
	MOV R1, A
	
	MOV A, #0	;; clear A
	
	MOV R7, #0	;; use R7 as counter
	
	;; compare the original data with the copied data
	AGAIN:
		MOVC A, @A+DPTR	;; get original
	
		MOV R4, A	;; save A
	
		MOV A, @R1		;; get copied

		;; compare the two contents with XRL
		XRL A, R4		;; if they are the same then should be 0
		CJNE A, #0, Error	;; if they are not equal go to error
	
		INC R7
		MOV A, R7	;; move R7 to A (increment A)
	
		INC R1
	
		;; go back and repeat until checked the entire string
		DJNZ R6, AGAIN
		
		MOV A, R5	;; returning len
	
		RET		;; returning back to caller (MAIN)

;; Function MAIN
MAIN:

	;; R2 is used as variable t
	MOV R2, #0	;; initialize R2 to 0
	
	MOV DPTR, #STRING1	;; load the address of STRING1 to DPTR
	MOV R0, #60H
	ACALL TESTSTRING	;; call TESTSTRING
	
	INC R2	;; increment t
	
	MOV DPTR, #STRING2	;; load the address of STRING1 to DPTR
	MOV R0, #60H
	ACALL TESTSTRING	;; call TESTSTRING
	
	INC R2	;; increment t
	
	MOV DPTR, #STRING3	;; load the address of STRING1 to DPTR
	MOV R0, #60H
	ACALL TESTSTRING	;; call TESTSTRING
	
	INC R2	;; increment t


Success:
	SJMP Success

Error:
	SJMP Error
	
END
