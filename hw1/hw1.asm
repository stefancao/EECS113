;; Name: Stefan Cao
;; Assignment: Homework 1
;; Status: Works!

	ORG 0H
	SJMP main	 	;; jump to main

;; max function -- max(a,b) where a=R0, b=R1, return is put in register A
max:

	MOV	A, R0		;; move R0 to A
	
	XRL A, R1		;; XOR R0 and R1 (only care about MSB (bit 8))
	
	CLR C			;; clear C just in case
	RLC A			;; rotate left with carry to put MSB in carry bit
					;; to check if the MSB is 1 or 0 
					;;(if 1 means different signs, 0 means same sign)
	
	JC	different_sign		;; jump if C=1 (sign is different)
	
	;; else subtract to see which is larger and greater
	MOV A, R0		;; move R0 to A
	
	SUBB A, R1		;; R0-R1, if carry is 1 --> R0 < R1
	
	JNC return_a	;; if R0 is greater than there is no carry
	
return_b:
	MOV A, R1		;; else R1 is greater, put it in A and return
	RET
	
return_a: 
	MOV A, R0		;; R0 is greater, put it in A and return
	RET	
	
different_sign:
	
	;; check the one that has MSB = 0 , that is the positive number which is the greater number
	MOV A, R0
	
	CLR C		;; clear C just in case
	RLC A
	
	JC return_b		;; if C=1 then B is positive so return B
	SJMP return_a	;; else return A
	
	
;; main function
main:

	;; R2 as variable t
	MOV R2, #0	;; initialize to 0
	
	;; test case 1
	;; R0=1, R1=2
	MOV R0, #1
	MOV R1, #2
	ACALL max
	;; comparing
	CJNE A, #2, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 2
	;; R0=2, R1=1
	MOV R0, #2
	MOV R1, #1
	ACALL max
	;; comparing
	CJNE A, #2, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 3
	;; R0=-3, R1=4
	MOV R0, #-3
	MOV R1, #4
	ACALL max
	;; comparing
	CJNE A, #4, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 4
	;; R0=-3, R1=-2
	MOV R0, #-3
	MOV R1, #-2
	ACALL max
	;; comparing
	CJNE A, #-2, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 5
	;; R0=0, R1=-63
	MOV R0, #0
	MOV R1, #-63
	ACALL max
	;; comparing
	CJNE A, #0, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 6
	;; R0=12, R1=125
	MOV R0, #12
	MOV R1, #125
	ACALL max
	;; comparing
	CJNE A, #125, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 7
	;; R0=-48, R1=90
	MOV R0, #-48
	MOV R1, #90
	ACALL max
	;; comparing
	CJNE A, #90, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
	;; test case 8
	;; R0=-90, R1=0
	MOV R0, #-90
	MOV R1, #0
	ACALL max
	;; comparing
	CJNE A, #0, Error	;; if not equal go to error
	;; else
	INC R2	;; increment R2
	
;; should loop through success loop forever if everything works	
Success:
	SJMP Success

;; if something is wrong, should loop in this error loop
Error:
	SJMP Error
