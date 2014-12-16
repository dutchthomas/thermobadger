
typedef struct task
{
signed char state;
unsigned long period;
unsigned long ticks;
unsigned long elapsedTime;
int (*TickFn)( struct task*);
} task;

unsigned short tasksGCD(task* tasks[], unsigned short numTasks);
unsigned short tasksInit(task* tasks[], unsigned short numTasks);
void tasksTick(task* tasks[], unsigned short numTasks);

unsigned short tasksInit(task* tasks[], unsigned short numTasks)
{
    int i;
    
    unsigned short gcd = tasksGCD(tasks, numTasks);
    
    for ( i = 0; i < numTasks; i++ )
    {
        // Initialize state to 0 = INIT
        tasks[i]->state = 0;
        
        // Normalize period
        tasks[i]->ticks = tasks[i]->period / gcd; 
        
        // Initialize elapsed to period so that it runs immediately
        tasks[i]->elapsedTime = tasks[i]->ticks;
    }
    
    return gcd;
}

void tasksTick(task* tasks[], unsigned short numTasks)
{
    int i;   
    
    // Scheduler
    for ( i = 0; i < numTasks; i++ )
    {
   
        if( tasks[i]->elapsedTime >= tasks[i]->ticks )
        {
            // Run machine, set state
            tasks[i]->TickFn(tasks[i]);
            
            // Reset timer
            tasks[i]->elapsedTime = 0;
           
        }
        
        // Increment timer
        tasks[i]->elapsedTime++;
        
    }

}

unsigned short tasksGCD(task* tasks[], unsigned short numTasks)
{
	unsigned long a, b, c;
	
    int i;
    
    if(numTasks == 0)
    {
        return 0;
    }
    
    if(numTasks == 1)
    {
        return tasks[0]->period;
    }
    
    b = tasks[0]->period;
    
    for(i = 1; i < numTasks; i++)
    {
        a = tasks[i]->period;
        c = 1;
        
	    while(c != 0)
	    {
		    c = a % b;

		    if(c)
		    {
		        a = b;
		        b = c;
		    }
	    }
	}

    return b;

}
