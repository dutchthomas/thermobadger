// File: adc.h
// Author: Nicholas Gingerella
#ifndef ADC_H
#define ADC_H

void ADC_init() {
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);
	//ADCSRA |= (1 << ADATE);
	ADCSRA |= (1 << ADIE);
	ADCSRA |= (1 << ADEN);
	// ADPS[2:0] = 110: prescaler 64 -> 8000kHz/64 = 125kHz
	// ADIE: setting this bit enables ADC interrupts. This interrupt is called
	//	     whenever a conversion finishes
	// ADEN: setting this bit enables analog-to-digital conversion.
	
	ADCSRA |= (1 << ADSC); // start first conversion
}



void ADC_SetReadPort(unsigned char portA_num){
	
	// if the user enters a port number that doesn't
	// exist, just default to port A7
	if(portA_num > 7){
		portA_num = 7;
	}
	
	// 0 out MUX0-MUX4 bits in ADMUX register and replace
	// them with the value of portA_num, which should be a
	// value from 0-7 (A0, A1,... , A7)
	ADMUX = (ADMUX & 0xE0) | (portA_num & 0x1F);
}



// variables for adc analog inputs
volatile uint16_t TEMP_SENSOR;	//A3: temperature from temp sensor

ISR(ADC_vect){
	// Check which port is currently active
	// for ADC conversion: 0000 xxxx <-- I only care about [2:0]
	uint8_t admux_MuxNibble = (ADMUX & 0x0F);
	uint8_t theLow = ADCL;
	uint16_t result = ADCH << 8 | theLow;
	
	// depending on the current conversion port, update the
	// variable corresponding to that port's sensor
	switch(admux_MuxNibble){
		case 0:
			// update analog input variable on A0
			TEMP_SENSOR = result;
			ADC_SetReadPort(0);
			break;
		case 1:
			// update analog input variable on A1
			break;
		case 2:
			// update analog input variable on A2
			break;
		case 3:
			break;
		case 4:
			// update analog input variable on A4
			break;
		case 5:
			// update analog input variable on A5
			break;
		case 6:
			// update analog input variable on A6
			break;
		case 7:
			// update analog input variable on A7
			break;
		default:
			break;
	}
	
	ADCSRA |= (1 << ADSC); // next conversion?
}






#endif
