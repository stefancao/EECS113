/*
// Name: Stefan Cao
// Date: 5/1/2016
// Assignment 3
// Status: Works!
// Comments: The best Update frequency to use is 100 with Pulse mode. If using 
//		higher update frequency, can result a number to repeated multiple times due
//		to the the Keypad thinking it is getting a new value before the user has 
//		been able to remove the click. When clicking on keypad
// 		hold it for a little bit for it to register. 
//
//		After clicking the equal sign, with update frequency of 100, it will 
//		take some time for the answer to be outputed since it's converting
//		and displaying each character individually
//
//		Using LCD E P3.2 and LCD RS P3.3 as sample lcd.c given.
//
//		Works with 4 digit numbers.
*/

#include <8051.h>

#define DB P1
#define RS P3_3
#define E P3_2

#define clear P2_0  // SW0
#define msg1 P2_1   // SW1
#define msg2 P2_2   // SW2

// Functions for LCD
void returnHome(void);
void entryModeSet();
void displayOnOffControl(__bit display, __bit cursor, __bit blinking);
void cursorOrDisplayShift(__bit sc, __bit rl);
void functionSet(void);
void clearDisplay(void);
void setDdRamAddress(char address);
void sendChar(char c);
void sendString(char* str);
void delay(void);
void big_delay(void);


void IntToChar(unsigned int i, unsigned int lenA, unsigned int lenB);
unsigned int powerOfTen(unsigned int i);
unsigned int countDigits(int n);

// main function
void Main(void) {
	unsigned char row;   // Contains the required mask to clear one of the rows at a time
	unsigned char col;   // Contains the output of the columns
	unsigned char keypad[4][3] = {   // This array contains the ASCII of the keypad keys
		{'3','2','1'},
		{'6','5','4'},
		{'9','8','7'},
		{'=','0','+'}
	};

	// char array for operands A and B 
	unsigned char operandA[5];
	unsigned char operandB[5];

	// length of operands A and B
	unsigned int operandALen = 0;
	unsigned int operandBLen = 0;

	// integer type if operand A and B
	unsigned int operandAInt = 0;
	unsigned int operandBInt = 0;
	
	// boolean indicating if '+' has been clicked
	unsigned char isAdd = 0;

	/* since this program is only doing addition of two operands, 
	 	by default isOperandA indicates that the numbers that the user 
	 	enters is of the first Operand until user clicks '+' which will 
	 	set isOperandA = 0, indicating that it is now Operand B */
	unsigned char isOperandA = 1;
	
	// variables used for calculations
	unsigned int tmp = 0;
	unsigned int sum = 0;
	unsigned int len = 0;

	unsigned char i;

	// character which user clicks on the keypad
	unsigned char gotkey = 0;

	// setting up LCD
	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 1, 1); // display on, cursor on and blinking on


	while(1){

		/* checking if a keypad has been clicked and the return char value is 
			store in gotkey */
		while(1){
			gotkey = 0;
			row = 0xf7;  // The first row (connected to P0.3) will be zero
			
			for (i=0;i<4;++i){ // loop over the 4 rows
				
				P0 = 0xff;     // Initialize the 4 rows to '1' and set the column ports to inputs
				P0 = P0 & row; // clear one row at a time
				col = P0 & 0x70;  // Read the 3 columns
				if (col != 0x70){ // If any column is zero i.e. a key is pressed
					col = (~col) & 0x70;  // because the selected column returns zero
					col = col >> 5;       // The column variable now contain the number of the selected column
					gotkey = keypad[i][col]; // Get the ASCII of the corresponding pressed key
					break;  // Since a key was detected -> Exit the for loop
				}
				row = row >> 1;   // No key is detected yet, try the next row
				row = row | 0xf0; // Only one of the least 4 significant bits is '0' at a time 
				
			}
			
			if (gotkey != 0){break;}  // Since a key was detected -> exit the while loop
		}
		
		// '*' indicates an addition
		if(gotkey == '+') {
			sendChar(gotkey);

			// set isOperandA to false indicating the next input from user is operandB
			isOperandA = 0;
		}

		// '#' indicates an equal sign
		else if(gotkey == '=') {
			sendChar(gotkey);

			/* converting operandA into an integer type
			 	by going through each character in operandA char array
			 	and converting each to the corresponding int value and sum them
				up to the right value */
			len = operandALen;
			for(i=0; i < operandALen; i++){
				tmp = (unsigned int)operandA[i] - 48;	// converting from ascii to integer
				len--;

				// multiplying to the corresponding 'weight' and add them up to get the correct int value
				operandAInt += tmp*powerOfTen(len);	
			}

			// Doing same for operand B
			len = operandBLen;
			for(i=0; i < operandBLen; i++){
				tmp = (unsigned int)operandB[i] - 48;
				len--;
				operandBInt += tmp*powerOfTen(len);
			}

			// add them together to get the sum
			sum = operandAInt + operandBInt;

			// calling IntToChar which converts it to char and display on LCD
			IntToChar(sum, operandALen, operandBLen);
		}

		// else the user has clicked on a number
		else {

			// display the number
			sendChar(gotkey);

			// OperandA
			if(isOperandA){

				// storing in operandA char array
				operandA[operandALen] = gotkey;
				operandALen++;
			}

			// else OperandB
			else{

				// storing in operandB char array
				operandB[operandBLen] = gotkey;
				operandBLen++;
			}
		}

	}	// end of while
}	// end of main


// Converting the sum to char and display it on the LCD one digit at a time
void IntToChar(unsigned int i, unsigned int lenA, unsigned int lenB){

	unsigned int tmp = 0;
	int j = 0;
	unsigned int len = 0;
	unsigned char result[5];

	// getting individual digits and store it in result char array (reverse order)
	while(i) {
		tmp = i % 10;

		// adding 48 to get the char value of that digit
		tmp += 48;
		result[len] = (unsigned char) tmp;

		i /= 10;
		len++;
	}

	// if the result cannot fit on the first line move the the next line
	if(lenA+lenB+countDigits(len) >= 11){
		setDdRamAddress(0x40); // set address to start of second line
	}

	// print out the result in reverse order to get the right order
	for(j = len-1; j >= 0; j--){
		sendChar(result[j]);
	}
}

// Power of 10 function
unsigned int powerOfTen(unsigned int i){
	unsigned int j = 0;

	if(i == 0){
		return 1;
	}
	else{
		unsigned int result = 10;
		for(j = 0; j < (i-1); j++){
			result *= 10;
		}
		return result;
	}
}

// counting number of digits
unsigned int countDigits(int n){

	unsigned int count = 0;
    if (n < 10) {
        return 1;
    }
    
    while (n > 0) {
        n /= 10;
        count++;
    }
    return count;
}


// LCD Module instructions -------------------------------------------

void returnHome(void) {
	RS = 0;
	P1 = 0x02; // LCD command to return home (the first location of the first lcd line)
	E = 1;
	E = 0;
	big_delay(); // This operation needs a bigger delay
}	

void entryModeSet() {
	RS = 0;
	P1 = 0x06;  
	E = 1;
	E = 0;
	delay();
}

void displayOnOffControl(__bit display, __bit cursor, __bit blinking) {
	P1_7 = 0;
	P1_6 = 0;
	P1_5 = 0;
	P1_4 = 0;
	P1_3 = 1;
	P1_2 = display;
	P1_1 = cursor;
	P1_0 = blinking;
	E = 1;
	E = 0;
	delay();
}

void cursorOrDisplayShift(__bit sc, __bit rl) {
	RS = 0;
	P1_7 = 0;
	P1_6 = 0;
	P1_5 = 0;
	P1_4 = 1;
	P1_3 = sc;
	P1_2 = rl;
	P1_1 = 0;
	P1_0 = 0;
	E = 1;
	E = 0;
	delay();
}

void functionSet(void) {
	RS = 0;
	P1 = 0x38; // 8-bit mode, 2 lines LCD
	E = 1;
	E = 0;
	delay();
}

void clearDisplay(void) {
	RS = 0;
	P1 = 0x01; // command to clear LCD and return the cursor to the home position
	E = 1;
	E = 0;
	big_delay(); // This operation needs a bigger delay
}	

void setDdRamAddress(char address) {  // Determine the place to place the next character - First LCD location address in 00h for line 1 and 40h for line 2
	RS = 0;
	P1 = address | 0x80;  // set the MSB to detect that this is an address
	E = 1;
	E = 0;
	delay();
}

// --------------------------------------------------------------------

void sendChar(char c) {  // Function to send one character to be displayed on the LCD
	RS = 1;
	P1 = c;
	E = 1;
	E = 0;
	delay();
}

void sendString(char* str) {  // Function to send a string of characters to be displayed on the lCD
	char c;
	while (c = *str++) {
		sendChar(c);
	}
}

void delay(void) {
	char c;
	for (c = 0; c < 50; c++);
}

void big_delay(void) {
	unsigned char c;
	for (c = 0; c < 255; c++);
}