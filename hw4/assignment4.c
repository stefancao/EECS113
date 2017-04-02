/*
// Name: Stefan Cao
// Date: 5/19/2016
// Assignment 4
// Status: Works
// NOTE: the validation check during interrupt thus, it doesn't allow invalid numbers
// For example, if you want 31-01-20. And the current month is February. You have to change 
// February first before you can change the date. Otherwise the valdiaiton will see it as false
*/
#include <8051.h>

// LCD Data
#define DB P1 	// DB
#define RS P2_3   // RS
#define E P2_2   // E
#define SW0 P3_2  // SW0
#define SW1 P3_3   // SW1

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

void timer_delay(void);

void initialize();

void updateHourArray();
void updateMinuteArray();
void updateSecondArray();
void updateDDArray();
void updateMMArray();
void updateYYArray();

void printHour(void);
void printMinute(void);
void printSecond(void);
void printDD(void);
void printMM(void);
void printYY(void);

int DaysInMonth();
char getKeypad(void);
int checkTimeInput();
int checkDateInput();
int powerOfTen(int i);
void settingArray(char *array1, char *array2);
int getIntValue(char pos);
void refresh();

// global variables
int hour, minute, second, DD, MM, YY;
unsigned char hourArray[2], minuteArray[2], secondArray[2], DDArray[2], MMArray[2], YYArray[2];
unsigned char tempPos1Array[2],tempPos2Array[2],tempPos3Array[2];


//interrupt to change time
void int0_isr (void) __interrupt (0) __using (1){

	char keypressed;
	char currCursorPos = 0x00;	// setting position of cursor

	// initializing arrays
	settingArray(tempPos1Array, hourArray);
	settingArray(tempPos2Array, minuteArray);
	settingArray(tempPos3Array, secondArray);

	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 1, 1); // display on, cursor on and blinking on
	returnHome();
	setDdRamAddress(currCursorPos);	// hour position
	
	// while SW is pushed down
	while(SW0 == 0){
		
		// gets character from keypad
		keypressed = getKeypad();

		if(SW0 == 1){break;} //jumping out of interrupt after keypad

		// move cursor to the left
		if(keypressed == '*') {

			// wraps around
			if (currCursorPos == 0x00){
				currCursorPos = 0x07;	
			}
			else{
				currCursorPos--;	// move cursor to the left

				// skip positions where it's ':'
				if(currCursorPos == 0x02 || currCursorPos == 0x05) {
					currCursorPos--;
				}
			}
			setDdRamAddress(currCursorPos);
		}

		// move cursor the the right
		else if (keypressed == '#') {

			// wraps around
			if (currCursorPos == 0x07) {
				currCursorPos = 0x00;
			}
			else{
				currCursorPos++;	// move cursor to the right

				// skip positions where it's ':'
				if(currCursorPos == 0x02 || currCursorPos == 0x05){
					currCursorPos++;
				}
			}
			setDdRamAddress(currCursorPos);
		}
		else {

			// set the input key to the temp arrays
			switch(currCursorPos) {
				case 0x00:
					tempPos1Array[0] = keypressed;
					break;
				case 0x01:
					tempPos1Array[1] = keypressed;
					break;
				case 0x03:
					tempPos2Array[0] = keypressed;
					break;
				case 0x04:
					tempPos2Array[1] = keypressed;
					break;
				case 0x06:
					tempPos3Array[0] = keypressed;
					break;
				case 0x07:
					tempPos3Array[1] = keypressed;
			}

			currCursorPos++;	// move cursor to the right

			// wrap around
			if(currCursorPos == 0x08) {
				currCursorPos = 0x00;
			}

			// skip positions where it's ':'
			else if(currCursorPos == 0x02 || currCursorPos == 0x05){
				currCursorPos++;
			}
			sendChar(keypressed);
			setDdRamAddress(currCursorPos);
		}

		// checking if time input is valid
		if(checkTimeInput() == 1) {
			setDdRamAddress(0x09);
			sendString("Error");

			// wait for a keypress
			getKeypad();

			clearDisplay();

			// setting the arrays to original
			settingArray(tempPos1Array, hourArray);
			settingArray(tempPos2Array, minuteArray);
			settingArray(tempPos3Array, secondArray);

			refresh();

			currCursorPos = 0x00;
			setDdRamAddress(currCursorPos);

		}

		// the input is valid set needToCheck to false again
		else {

			// replace the new array with temp
			hour = getIntValue('hh');
			minute = getIntValue('mm');
			second = getIntValue('ss');

			updateHourArray();
			updateMinuteArray();
			updateSecondArray();

		}
	}

	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on

	refresh();
}
	


// interrupt to change date
void int1_isr (void) __interrupt (2) __using (1){
	char keypressed;
	char currCursorPos = 0x40;

	// initlaizing arrays
	settingArray(tempPos1Array, DDArray);
	settingArray(tempPos2Array, MMArray);
	settingArray(tempPos3Array, YYArray);

	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 1, 1); // display on, cursor on and blinking on
	returnHome();
	setDdRamAddress(currCursorPos);	// hour position
	
	// while SW1 is still pressed
	while(SW1 == 0){
		
		// gets character from keypad
		keypressed = getKeypad();

		if(SW1 == 1){break;} //jumping out of interrupt after keypad

		// move the the left
		if(keypressed == '*') {

			// wraps around
			if (currCursorPos == 0x40){
				currCursorPos = 0x47;
	
			}
			else{
				currCursorPos--;
				if(currCursorPos == 0x42 || currCursorPos == 0x45) {
					currCursorPos--;
				}
			}
			setDdRamAddress(currCursorPos);
		}

		// move to the right
		else if (keypressed == '#') {

			// wraps around
			if (currCursorPos == 0x47) {
				currCursorPos = 0x40;
			}
			else{
				currCursorPos++;
				if(currCursorPos == 0x42 || currCursorPos == 0x45){
					currCursorPos++;
				}
			}
			setDdRamAddress(currCursorPos);
		}
		else {

			// set the input
			switch(currCursorPos) {
				case 0x40:
					tempPos1Array[0] = keypressed;
					break;
				case 0x41:
					tempPos1Array[1] = keypressed;
					break;
				case 0x43:
					tempPos2Array[0] = keypressed;
					break;
				case 0x44:
					tempPos2Array[1] = keypressed;
					break;
				case 0x46:
					tempPos3Array[0] = keypressed;
					break;
				case 0x47:
					tempPos3Array[1] = keypressed;
			}

			currCursorPos++;
			if(currCursorPos == 0x48) {
				currCursorPos = 0x40;
			}
			else if(currCursorPos == 0x42 || currCursorPos == 0x45){
				currCursorPos++;
			}
			sendChar(keypressed);
			setDdRamAddress(currCursorPos);
		}

		// check if date is valid
		if(checkDateInput() == 1) {
			setDdRamAddress(0x49);
			sendString("Error");
			// wait for a keypress
			getKeypad();

			clearDisplay();

			// setting temp arrays to original
			settingArray(tempPos1Array, DDArray);
			settingArray(tempPos2Array, MMArray);
			settingArray(tempPos3Array, YYArray);

			refresh();

			currCursorPos = 0x40;
			setDdRamAddress(currCursorPos);

		}

		else {

			// replace the new array with temp
			DD = getIntValue('hh');
			MM = getIntValue('mm');
			YY = getIntValue('ss');

			updateDDArray();
			updateMMArray();
			updateYYArray();

		}
	}

	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on

	refresh();
}
	

void settingArray(char *array1, char *array2) {
	int i = 0;

	for(i=0; i<2; i++) {
		array1[i] = array2[i];
	}
}

void main(void) {

	IT0 = 1;   
	IT1 = 1;	
	IE = 0x85;  

	initialize();

	while(1) {
	
		timer_delay();
		second++; 	// increment second

		if (second >= 60) {
			second = 0;
			minute++;
			updateSecondArray();
			updateMinuteArray();
		}
		else {
			updateSecondArray();
		}

		if (minute >= 60) {
			minute = 0;
			hour++;
			updateMinuteArray();
			updateHourArray();
		}

		if (hour >= 24) {
			hour = 0;
			DD++;
			updateHourArray();
			updateDDArray();
		}

		if (DD >= DaysInMonth()) {
			DD = 1;
			MM++;
			updateDDArray();
			updateMMArray();
		}

		if (MM >= 12) {
			MM = 1;
			YY++;
			updateMMArray();
			updateYYArray();
		}

		if (YY >= 100) {
			YY = 0;
			updateYYArray();
		}

		functionSet();
		entryModeSet(); // increment and no shift
		displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on

		// refreshing
		refresh();

	}


}


void initialize() {

	hour = 0;
	hourArray[0] = '0';
	hourArray[1] = '0';

	minute = 0;
	minuteArray[0] = '0';
	minuteArray[1] = '0';


	second = 0;
	secondArray[0] = '0';
	secondArray[1] = '0';

	DD = 1;
	DDArray[0] = '0';
	DDArray[1] = '1';

	MM = 1;
	MMArray[0] = '0';
	MMArray[1] = '1';

	YY = 0;
	YYArray[0] = '0';
	YYArray[1] = '0';

	// initializing
	functionSet();
	entryModeSet(); // increment and no shift
	displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on

	refresh();
	
}

void refresh (){
	printHour();
	sendChar(':');
	printMinute();
	sendChar(':');
	printSecond();
	printDD();
	sendChar('-');
	printMM();
	sendChar('-');
	printYY();
}

int checkTimeInput() {
	int i = 0;
	int tmp = 0;
	int len = 2;
	int IntValue = 0;

	IntValue = getIntValue('hh');
	if(IntValue >= 24) {
		return 1;
	}

	IntValue = getIntValue('mm');
	if(IntValue >= 60) {
		return 1;
	}

	IntValue = getIntValue('ss');
	if(IntValue >= 60) {
		return 1;
	}

	return 0;

}

int checkDateInput() {
	int i = 0;
	int tmp = 0;
	int len = 2;
	int IntValue = 0;

	IntValue = getIntValue('ss');
	if(IntValue > 100) {
		return 1;
	}

	IntValue = getIntValue('mm');
	if(IntValue > 12) {
		return 1;
	}

	IntValue = getIntValue('hh');
	if(IntValue > DaysInMonth()) {
		return 1;
	}

	return 0;
}


int getIntValue(char pos) {
	char tmpArray[2];
	int i = 0;
	int tmp = 0;
	int len = 2;
	int IntValue = 0;

	switch(pos) {
		case 'hh':
			settingArray(tmpArray, tempPos1Array);
			break;
		case 'mm':
			settingArray(tmpArray, tempPos2Array);
			break;
		case 'ss':
			settingArray(tmpArray, tempPos3Array);
	}

	for (i = 0; i < 2; i++) {
		tmp = (int)tmpArray[i] - 48;	// converting from ascii to integer
		len--;

		// multiplying to the corresponding 'weight' and add them up to get the correct int value
		IntValue += tmp*powerOfTen(len);	
	}

	return IntValue;
}

int powerOfTen(int i) {
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


int DaysInMonth() {
	int days = 0;

	switch (MM) {
		case 1:
			days = 31;
			break;
		case 2:
			days = 28;
			break;
		case 3:
			days = 31;
			break;
		case 4:
			days = 30;
			break;
		case 5:
			days = 31;
			break;
		case 6:
			days = 30;
			break;
		case 7:
			days = 31;
			break;
		case 8:
			days = 31;
			break;
		case 9:
			days = 30;
			break;
		case 10:
			days = 31;
			break;
		case 11:
			days = 30;
			break;
		case 12:
			days = 31;
	}

	return days;
}

void updateHourArray() {
	int tmpHour = hour;
	int tmp = 0;
	int len = 1;

	if(tmpHour < 10) {
		hourArray[0] = '0';
	}

	while(tmpHour) {
		tmp = tmpHour % 10;
		tmp += 48;
		hourArray[len] = (unsigned char) tmp;
		tmpHour /= 10;
		len--;
	}
}

void updateMinuteArray() {
	int tmpMinute = minute;
	int tmp = 0;
	int len = 1;

	if(tmpMinute < 10) {
		minuteArray[0] = '0';
	}

	while(tmpMinute) {
		tmp = tmpMinute % 10;
		tmp += 48;
		minuteArray[len] = (unsigned char) tmp;
		tmpMinute /= 10;
		len--;
	}
}

void updateSecondArray() {
	int tmpSecond = second;
	int tmp = 0;
	int len = 1;

	if(tmpSecond < 10) {
		secondArray[0] = '0';
	}

	while(tmpSecond) {
		tmp = tmpSecond % 10;
		tmp += 48;
		secondArray[len] = (unsigned char) tmp;
		tmpSecond /= 10;
		len--;
	}
}

void updateDDArray() {
	int tmpDD = DD;
	int tmp = 0;
	int len = 1;

	if(tmpDD < 10) {
		DDArray[0] = '0';
	}

	while(tmpDD) {
		tmp = tmpDD % 10;
		tmp += 48;
		DDArray[len] = (unsigned char) tmp;
		tmpDD /= 10;
		len--;
	}
}

void updateMMArray() {
	int tmpMM = MM;
	int tmp = 0;
	int len = 1;

	if(tmpMM < 10) {
		MMArray[0] = '0';
	}

	while(tmpMM) {
		tmp = tmpMM % 10;
		tmp += 48;
		MMArray[len] = (unsigned char) tmp;
		tmpMM /= 10;
		len--;
	}
}

void updateYYArray() {
	int tmpYY = YY;
	int tmp = 0;
	int len = 1;

	if(tmpYY < 10) {
		YYArray[0] = '0';
	}

	while(tmpYY) {
		tmp = tmpYY % 10;
		tmp += 48;
		YYArray[len] = (unsigned char) tmp;
		tmpYY /= 10;
		len--;
	}
}


void printHour(void) {
	int i = 0;
	setDdRamAddress(0x00);	// hour position
	for(i = 0; i < 2; i++){
		sendChar(hourArray[i]);
	}
}

void printMinute(void) {
	int i = 0;
	setDdRamAddress(0x03);	// minute position
	for(i = 0; i < 2; i++){
		sendChar(minuteArray[i]);
	}
}

void printSecond(void) {
	int i = 0;
	setDdRamAddress(0x06);	// second position
	for(i = 0; i < 2; i++){
		sendChar(secondArray[i]);
	}
}

void printDD(void) {
	int i = 0;
	setDdRamAddress(0x40); // day position
	for(i = 0; i < 2; i++){
		sendChar(DDArray[i]);
	}
}

void printMM(void) {
	int i = 0;
	setDdRamAddress(0x43);	// month position
	for(i = 0; i < 2; i++){
		sendChar(MMArray[i]);
	}
}

void printYY(void) {
	int i = 0;
	setDdRamAddress(0x46);	// year position
	for(i = 0; i < 2; i++){
		sendChar(YYArray[i]);
	}
}

// time delay 1 second
void timer_delay(void){
	 
	unsigned char a=2; 
	TMOD = 0x01;
	while (a != 0){
		TL0 = 0xAF;
		TH0 = 0x3C;
		TR0 = 1;
		while (!TF0) ;
		TR0 = 0;
		TF0 = 0;
		a = a-1;

	}
		
}

char getKeypad(void){
		unsigned char row;   // Contains the required mask to clear one of the rows at a time
	unsigned char col;   // Contains the output of the columns
	unsigned char keypad[4][3] = {   // This array contains the ASCII of the keypad keys
		{'3','2','1'},
		{'6','5','4'},
		{'9','8','7'},
		{'#','0','*'}
		};
	unsigned char i;
	unsigned char gotkey = 0;
	while(1){
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
		if (SW1 == 1 && SW0 == 1){break;} // if user decide to get out of set things without pressing a button
	}

	while ((P0 & 0x70) != 0x70){} // wait for key release
	return gotkey;  // Send the detected key to the output
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

// To make the interrupt work correctly with Edsim51
void _sdcc_gsinit_startup(void) {

	__asm
		mov sp, #0x5F
	__endasm;
	main();
}