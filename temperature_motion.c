#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//includes for lcd and Get/Set bit
#include "includes/bit.h"
#include "includes/lcd.h"

//includes for scheduler
#include "includes/tomsched.h" 
#include "includes/timer.h"

//includes for temperature sensor
#include "includes/onewire_ds18b20.h"
#include "includes/onewire.h"

//includes for uart debug
#include "includes/uout.h"

//includes for SPI
#include "includes/spi.h"

// Global
unsigned char sensor;	//input from motion sensor
char* motionSensorMsg;	//lcd message
static int temperature;

char led_ac, led_fan, led_heat;

char digMode = 0;
char digOn = 0;
char temp = 0;
char d0 = -1;
char d1 = -1;
char remoteTimeout = -1;
char kill;
char enable = 0;
char fanMode = 0;

//includes for remote
#define MAX_TEMP 90
#define MIN_TEMP 60
#define NRF_CHANNEL 7
#include "nrf.h"
#include "radio.c"



// Task definitions 
//====================== lcd machine ==========================
//Prints information about the system to the LCD. The cstrings constructed in
//the lightControl machine are output to the LCD through here.
enum lcdControl_states {lcd_init, s1,} lcdControl_state;
int lcdDisplayTick(task* t){

	//actions	
	switch(t->state){
		case lcd_init:
			LCD_init();							//initialize LCD
			LCD_ClearScreen();					//clear screen of any artifacts
			break;
		case s1:
			//LCD_ClearScreen();					//clear screen of any artifacts
			//LCD_DisplayString(1,motionSensorMsg);	//display motion sensor message
			
				LCD_Cursor(14);
				
				if(fanMode)
				{
				    LCD_WriteData(1);
			    }
			    else if(led_fan && !kill && enable && temp)
			    {
			        LCD_WriteData(2);
			    }
			    else
			    {
			        LCD_WriteData(' ');
			    }
			    
				LCD_Cursor(12);
			    
			    if(enable)
			    {
			        LCD_WriteData(3);
			    }
			    else
			    {
			        LCD_WriteData(4);
			    }
			
			break;
		default:
			break;
	}

	//transitions
	switch(t->state){
		case lcd_init:
			t->state = s1;
			break;
		case s1:
			t->state = s1;
			break;
		default:
			break;
	}
	
	return 0;
}


//Movement Detector
//updates globals "sensor" variable
enum motionSensor_states {sensorInit, sensorOn} motionSensor_state;
int motionSensorTick(task* t)
{
static unsigned char initCounter = 0;
static char int_buffer[10];
const unsigned char personImg = 0x00;
static unsigned char cursorPos;
static struct OnewireDevice therm;
	// Actions
    switch(t->state)
    {
        case sensorInit:
			motionSensorMsg = "Initializing";

			//wait 5 seconds before starting detection. This gives the 
			//motion sensor time to initialize
            if(initCounter <= 50){
				initCounter++;
			} else {
				initCounter = 0;
			}
			ds18b20_setup_device(&therm, &PORTA, 0);
			ds18b20_set_resolution(&therm, LOW);

        break;
        
        case sensorOn:
			if( sensor ){

				LCD_Cursor(16);
				LCD_WriteData(personImg);

			} else {

				LCD_DisplayString(16," ");
				

			}


			//**************Build Temperature Message**************
			LCD_DisplayString(1, "R:");
			
			LCD_Cursor(3);
			
			if(d1 >= 0 && d1 <= 9)
			{
			    LCD_WriteData(d1 + '0');
			}
			else
			{
			    LCD_WriteData('-');
			}
			
			if(d0 >= 0 && d0 <= 9)
			{
			    LCD_WriteData(d0 + '0');
			}
			else
			{
			    LCD_WriteData('-');
			}
			
			LCD_DisplayString(17,"Temp:");
			
			//get temp sensor reading
			//temperature = (((TEMP_SENSOR + 2))*1.8) + 32;
			temperature = (int)ds18b20_get_temperature(&therm);
			
			itoa(temperature, int_buffer,10);
			cursorPos = 22;
			LCD_Cursor(cursorPos);

			//print out contents of the temp sensor reading
			//LCD_ClearScreen();			
			unsigned char i;
			for(i=0; int_buffer[i]; i++){
				LCD_WriteData(int_buffer[i]);
				cursorPos++;
			}

			//print out Fahrenhiet degree symbols
			LCD_DisplayString(24,"F");
			LCD_Cursor(25);
			LCD_WriteData((char)223);
			//**************Build Temperature Message**************

        break;
        default:
        break;
    }
    
    // Transitions
    switch(t->state)
    {   
        case sensorInit:
			if(initCounter <= 50){
				t->state = sensorInit;
			} else {
            	t->state = sensorOn;
			}
        break;
        
        case sensorOn:
            t->state = sensorOn;
        break;
        
        default:
        break;
    }

    return 0;

}



/* Radio control interface */
int menuTask(task* t)
{
    if(USART_HasReceived(0))
    {
        char uInput = USART_Receive(0);
        
        switch(uInput)
        {
            // Fan
            case 'f':
                tempProcess(0x3F); // emulate "?" function button
                tempProcess(0x32); // emulate "2" function button
            break;
            
            // Enable
            case 'e':
                tempProcess(0x3F); // emulate "?" function button
                tempProcess(0x31); // emulate "2" function button
            break;

            default:
                if(uInput >= '0' && uInput <= '9')
                {
                    tempProcess(0x30 | (uInput - '0'));
                }
            break;
        }
    }
    
    return 0;
    
}


int remoteTimeoutTask(task* t)
{
    if(remoteTimeout >= 0)
    {
        remoteTimeout++;
    }
    
    if(remoteTimeout > 3)
    {
        remoteTimeout = -1;
        uoutSend("Remote Timeout \n\r");
        tempCancel();
    }
    
    return 0;
}



int hvacTick(task *t)
{

    statusPrint();

    // Action
    switch(t->state)
    {
        case 0:
            temp = temperature;
            led_ac = 0;
            led_fan = 0;
            led_heat = 0;
        break;
        
        case 1:
            led_ac = 0;
            led_fan = 0;
            led_heat = 0;
        break;
        
        case 2:
            led_ac = 0;
            led_fan = 1;
            led_heat = 1;
        break;
        
        case 3:
            led_ac = 1;
            led_fan = 1;
            led_heat = 0;
        break;
    
        default:
        break;
    }
    
    // Transitions
    switch(t->state)
    {
        // Waiting...
        
        default:
            if( temp > temperature )
            {
                t->state = 2;
            }
            else if( temp < temperature)
            {
                t->state = 3;
            }
            else
            {
                t->state = 1;
            }
        
        break;

    }
    
    if(enable && !kill && temp)
    {
        if(fanMode)
        {
            PORTA = SetBit(PORTA, 2, 1);
        }
        else
        {
            PORTA = SetBit(PORTA, 2, led_fan);
        }
        PORTA = SetBit(PORTA, 3, led_ac);
        PORTA = SetBit(PORTA, 4, led_heat);
    }
    else
    {   
        
        PORTA = SetBit(PORTA, 2, fanMode);
        PORTA = SetBit(PORTA, 3, 0);
        PORTA = SetBit(PORTA, 4, 0);
    }
    return 0;
}



int sensorTimeoutTick(task *t)
{
    static char stime = 0;

    // Action
    switch(t->state)
    {
        case 0:
            kill = 1;
            stime = 0;
        break;
    
        case 1:
        
            kill = 0;
        
            if(stime < 100)
            {
                stime++;
            }
            
            if(sensor)
            {
                stime = 0;
            }

        
        break;
        
        default:
        break;
    }

    // transition
    switch(t->state)
    {
        case 0:
            if(sensor)
            {
                t->state = 1;
            }
        break;
        
        case 1:
            if(stime > 5)
            {
                t->state = 0;
            }
        break;
    
        default:
        break;
    }

    return 0;
}




/* End task definitions */



int main(void)
{

    // PORT config
	DDRC = 0xFF; PORTC = 0x00;	//1111 1111 : 0000 0000
	//DDRB = 0xFF; PORTB = 0x00; 	//1111 1111 : 0000 0000
    DDRB = 0x03; PORTB = 0x00;
	DDRA = 0xFD; PORTA = 0x02;	//1111 1101 : 0000 0010
	DDRD = 0x0a; PORTD = 0x05;

	//scheduler setup
    static task motionSensorPoll, lcdDisplay, uoutTask, RT_Task, M_Task, hvacTask, sensorTimeoutTask;
    task *tasks[] = { &lcdDisplay, &motionSensorPoll, &uoutTask, &RT_Task, &M_Task, &hvacTask, &sensorTimeoutTask };
    
    unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    //USART task
    uoutTask.period = 50;
    uoutTask.TickFn = &uoutTick;
    
    //Remote timeout task
    RT_Task.period = 1000;
    RT_Task.TickFn = &remoteTimeoutTask;

    //Menu task
    M_Task.period = 100;
    M_Task.TickFn = &menuTask;
    
	//LCD display task
    lcdDisplay.period = 100;
    lcdDisplay.TickFn = &lcdDisplayTick;

	//Motion sensor polling task
	motionSensorPoll.period = 100;
	motionSensorPoll.TickFn = &motionSensorTick;
	
	//HVAC
	hvacTask.period = 500;
	hvacTask.TickFn = &hvacTick;
	
	// sensor timeout
	sensorTimeoutTask.period = 500;
	sensorTimeoutTask.TickFn = &sensorTimeoutTick;

	// Init radio
    spiInitMaster();
    nrfMaster();
    enableINT0();
	
	// Init USART
    uoutInit(0);
    delay_ms(20);
    uoutSend("\n\r\n\r\n\r");
    uoutSend("  Welcome to thermostat!\n\r");
    uoutSend("==========================\n\r");
    	
    unsigned short gcd = tasksInit(tasks, numTasks);
    
    // Timer
    TimerSet(gcd);
    TimerOn();

	//custom character for lcd
	unsigned char personPattern[8] = {0x04, 0x0A, 0x04, 0x1f, 0x04, 0x04, 0x0A, 0x11};
	unsigned char fanOnPattern[8] = {0x00,0x06,0x1A,0x15,0x0B,0x0C,0x00,0x1F};
	unsigned char fanAutoPattern[8] = {0x00,0x06,0x1A,0x15,0x0B,0x0C,0x00,0x00};
	unsigned char onPattern[8] = {0x10, 0x12, 0x16, 0x1C, 0x18, 0x10, 0x10, 0x10};
	unsigned char offPattern[8] = {0x10, 0x10, 0x10, 0x18, 0x1C, 0x16, 0x12, 0x10};
	
	LCD_build(0,personPattern);
	LCD_build(1,fanOnPattern);
	LCD_build(2,fanAutoPattern); 
	LCD_build(3,onPattern); 
	LCD_build(4,offPattern); 

    while(1)
    {
		sensor = GetBit(PINA,1);

        tasksTick(tasks, numTasks);

        while(!TimerFlag); // Wait for a GCD period  
        TimerFlag = 0;
     
    }
    
    return 0;
}

