// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

#ifndef LCD_H
#define LCD_H

#include <stdio.h>

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
          
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTB	// port connected to pins 4 and 6 of LCD disp.
#define RS 0				// pin number of uC connected to pin 4 of LCD disp.
#define E 1					// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void delay_ms(int miliSec) { //for 8 Mhz crystal
	int i,j;
	for(i=0;i<miliSec;i++) {
		for(j=0;j<775;j++) {
			asm("nop");
		}
	}
}

/*-------------------------------------------------------------------------*/

void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {
	delay_ms(100); //wait for 100 ms for LCD to power up
	LCD_WriteCommand(0x38);//0011 1000
	LCD_WriteCommand(0x06);//0000 0110
	//LCD_WriteCommand(0x0f);//0000 1111
	//LCD_WriteCommand(0x01);//0000 0001
	LCD_WriteCommand(0x0C);//0000 1100 hide cursor
	delay_ms(10);						 
}

void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x2 LCD: column < 17; 16x1 LCD: column < 9
		LCD_WriteCommand(0x80 + column - 1);
		} else { // 6x2 LCD: column - 9; 16x1 LCD: column - 1
		LCD_WriteCommand(0xB8 + column - 9);
	}
}

void LCD_DisplayString( unsigned char column, char* string) {
	//LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}


//Input:
// location: location where you want to store
// 0,1,2,....7
// ptr: Pointer to pattern data
//
//Usage:
// pattern[8]={0x04,0x0E,0x0E,0x0E,0x1F,0x00,0x04,0x00};
// LCD_build(1,pattern);
//
//LCD Ports are same as discussed in previous sections

void LCD_build(unsigned char location, unsigned char *ptr){
	unsigned char i;
	if(location<8){
		LCD_WriteCommand(0x40+(location*8));
		for(i=0;i<8;i++)
		LCD_WriteData(ptr[ i ]);
	}
	
}


#endif // LCD_H

