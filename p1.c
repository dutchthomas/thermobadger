/*
 * Debug
 * 
 *
 * I acknowledge all content contained herein, excluding 
 * template or example code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "includes/io.c"

#include "includes/tomsched.h" 
#include "includes/timer.h" 

#include "includes/uout.h"
#include "includes/spi.h"

#include "nrf.h"

#define NRF_CHANNEL 7

// Global

char digOn = 0;
char temp = 0;
char d0 = -1;
char d1 = -1;
char remoteTimeout = -1;

#define MAX_TEMP 90
#define MIN_TEMP 60

#include "radio.c"

/* Task definitions */

/* Radio control interface */
int menuTask(task* t)
{
    static unsigned char uInput;

    if(USART_HasReceived(0))
    {
        uInput = USART_Receive(0);
        
        switch(uInput)
        {
            case 'r':
                nrfPrintReg();
            break;
            
            case 'm':
                nrfMaster();
            break;

            default:
            break;
        }
    }            

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
}

/* End task definitions */

int main(void)
{
    uoutInit(0);
    
    uoutSend("Master start\n\r");

    // PORT config
    DDRB = 0x01; PORTB = 0x02;
    
    // Spi as master
    spiInitMaster();
    
    // Radio init
    nrfMaster();
    
    enableINT0();
    
    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3};
    
    unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    task1.period = 50;
    task1.TickFn = &uoutTick;
    
    task2.period = 1000;
    task2.TickFn = &remoteTimeoutTask;


    task3.period = 100;
    task3.TickFn = &menuTask;
    
    unsigned short gcd = tasksInit(tasks, numTasks);
    
    // Timer
    TimerSet(gcd);
    TimerOn();
    
    unsigned short i; // Iterator
    while(1)
    {
        tasksTick(tasks, numTasks);
        
        while(!TimerFlag); // Wait for a GCD period  
        TimerFlag = 0;
     
    }
    
    return 0;
}

