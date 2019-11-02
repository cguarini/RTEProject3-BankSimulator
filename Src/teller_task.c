#include "main.h"
#include "cmsis_os.h"
#include "teller_task.h"
#include "rng.h"
#include "usart.h"
#include "string.h"
#include "queue.h"
#include "customerStruct.h"
#include "stdio.h"
#include "clock.h"

#define BREAKS_ENABLED (1)

extern osMessageQId CustomerQueueHandle;
extern osMessageQId MessageQueueHandle;
extern uint8_t bankClosedFlag;
TELLER_PARAMS_t teller_params[3]; // defined here, allocated in led_task.c
REPORT_STRUCT_t report = {0,{0,0,0},0,0,0,0,0,0,0,{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
uint8_t tellersComplete = 0;

void endOfDayReport(){
	char str[100];
	
	//Print header
	sprintf(str,"----------BEGIN END OF DAY REPORT----------\r\n");
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//print total number of customers served today
	sprintf(str,"\tTotal Number of Customers Served Today: %d\r\n", report.customersServed);
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//print total number of customers served by each teller
	for(int i = 0; i < 3; i++){
		sprintf(str,"\tTotal Number of Customers Served by Teller %d: %d\r\n",
			i, report.customersServedByTeller[i]);
		xQueueSend(MessageQueueHandle, &str, 200);
	}
	
	//print average time each customer spends waiting in the queue
	sprintf(str,"\tAverage time spent in queue: %d\r\n", report.totalTimeInQueue / report.customersServed);
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//print average time customer spends with teller
	sprintf(str,"\tAverage time spent with tellers: %d\r\n", report.totalTimeWithTellers / report.customersServed);
	xQueueSend(MessageQueueHandle, &str, 200);
  
  //print average time tellers wait for customers
	sprintf(str,"\tAverage time tellers wait for customers: %d\r\n", report.totalTimeWaitingForCustomer / report.customersServed);
	xQueueSend(MessageQueueHandle, &str, 200);
  
	
	//print maximum customer wait time in queue
	sprintf(str,"\tMaximum customer queue wait time: %d\r\n", report.maximumTimeInQueue);
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//print maximum wait time for tellers waiting for customers
	sprintf(str,"\tMaximum time tellers waited for customers: %d\r\n", report.maximumTimeWaitingForCustomer);
	xQueueSend(MessageQueueHandle, &str, 200);

	//print maximum transaction time for tellers
	sprintf(str,"\tMaximum transaction time: %d\r\n", report.maximumTransactionTime);
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//print maximum queue depth
	sprintf(str,"\tMaximum queue depth: %d\r\n", report.maximumDepthOfQueue);
	xQueueSend(MessageQueueHandle, &str, 200);
	
	//Grad Student Metrics
	if(BREAKS_ENABLED){
    //print number of breaks for each teller
    for(int i = 0; i < 3; i++){
      sprintf(str,"\tNumber of breaks for teller %d: %d\r\n",
        i, report.numberOfBreaksByTeller[i]);
      xQueueSend(MessageQueueHandle, &str, 200);
    }
    
    //print average break time for each teller
    for(int i = 0; i < 3; i++){
      sprintf(str,"\tAverage break time for teller %d: %d\r\n",
        i, report.totalBreakTime[i] / report.numberOfBreaksByTeller[i]);
      xQueueSend(MessageQueueHandle, &str, 200);
    }
      
    //print longest break time for each teller
    for(int i = 0; i < 3; i++){
      sprintf(str,"\tLongest break for teller %d: %d\r\n",
        i, report.longestBreakTime[i]);
      xQueueSend(MessageQueueHandle, &str, 200);
    }
    
    //print shortest break time for each teller
      //print longest break time for each teller
    for(int i = 0; i < 3; i++){
      sprintf(str,"\tShortest break for teller %d: %d\r\n",
        i, report.shortestBreakTime[i]);
      xQueueSend(MessageQueueHandle, &str, 200);
    }
  }
}

/*****************************************
teller_task() retrieves customers from the customer queue and services them for 30 seconds
to 8 minutes
Note: there may be multiple instances of teller_task.
inputs
  void *parameters - a pointer to the TELLER_PARAMS_t parameter block for this instance
outputs
  none
*******************************************/
void teller_task(void *parameters)
{
  uint32_t wait_ms;
  TELLER_PARAMS_t *p = (TELLER_PARAMS_t *)parameters;
  
  CustomerStruct_t customer;
  char str[100];
	
	//Generate initial break time
	HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);
	uint32_t breakStartTime = xTaskGetTickCount() + (wait_ms % 3000) + 3000;
	uint32_t breakLength = (wait_ms % 300) + 100;
	uint32_t timeWaitingForCustomer = 0;
  

  while(1) {
		//Get the start time of this loop
		char timeStr[10];
		uint32_t time = xTaskGetTickCount();
		getTimeString(timeStr, time);
		
		//Break Logic
		if(BREAKS_ENABLED && breakStartTime < time){
			//go on break
			sprintf(str, "%s - Teller %d is going on break for %d minutes\r\n", timeStr, p->id, breakLength/100);
			xQueueSend(MessageQueueHandle, &str, 200);
			
			//Report the break
			report.numberOfBreaksByTeller[p->id]++;
			report.totalBreakTime[p->id] += breakLength;
			report.longestBreakTime[p->id] < breakLength ? 
				report.longestBreakTime[p->id] = breakLength : report.longestBreakTime[p->id];
			breakLength < report.shortestBreakTime[p->id] || !report.shortestBreakTime[p->id] ? 
				report.shortestBreakTime[p->id] = breakLength : report.shortestBreakTime[p->id];
			vTaskDelay(breakLength);
			
			//generate new break time
			HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);
			breakStartTime = time + ((wait_ms % 3000) + 3000);
			breakLength = (wait_ms % 300) + 100;
      continue;
		}
    
    //Remove a customer from the queue, if not customer, don't block
    BaseType_t success = xQueueReceive(CustomerQueueHandle, &customer, 200);
    time = xTaskGetTickCount();//Time the customer is retrieved from the queue

    //Check if customer was successfully retrieved from queue
    if(!success){
			
			if(bankClosedFlag){
				//End of day and no customers are in the queue
				break;
			}
			
			timeWaitingForCustomer += 10;
			
      //try again if not successful, (queue was empty?)
			report.totalTimeWaitingForCustomer += 10;
			report.maximumTimeWaitingForCustomer < timeWaitingForCustomer ?
				report.maximumTimeWaitingForCustomer = timeWaitingForCustomer : report.maximumTimeWaitingForCustomer;
      vTaskDelay(10);
      continue;
    }
		
		timeWaitingForCustomer = 0;
    sprintf(str, "%s - Teller %d received customer %d from queue.\r\n",timeStr, p->id, customer.id);
    xQueueSend(MessageQueueHandle, &str, 200);
    
    //Mark when the customer was pulled from the queue
    customer.timeExitedQueue = time;
    
    //service the customer for 30 seconds to 8 minutes
    HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);//generate the random number
    wait_ms = ((wait_ms % 750) + 50);//set it to between 50 and 800 (30 seconds to 8 minutes)
    
		uint32_t timeInQueue = customer.timeExitedQueue - customer.timeEnteredQueue;
		
		//Gather Statistics
		report.customersServed++;
		report.customersServedByTeller[p->id]++;
		report.totalTimeInQueue += timeInQueue;
		report.totalTimeWithTellers += wait_ms;
		report.maximumTimeInQueue < timeInQueue ? 
			report.maximumTimeInQueue = timeInQueue : report.maximumTimeInQueue;
		report.maximumTransactionTime < wait_ms ?
			report.maximumTransactionTime = wait_ms : report.maximumTransactionTime;
		report.maximumDepthOfQueue < customer.maximumDepthOfQueue ?
			report.maximumDepthOfQueue = customer.maximumDepthOfQueue : report.maximumDepthOfQueue;
      
		//Service the customer
		vTaskDelay(wait_ms);
    
  }
	
	//Get the start time of this loop
	char timeStr[6];
	uint32_t time = xTaskGetTickCount();
	getTimeString(timeStr, time);
	
	sprintf(str, "%s - Teller %s is out of customers and going home.\r\n",timeStr, p->task_name);
  xQueueSend(MessageQueueHandle, &str, 200);
	
	tellersComplete++;
	//run the end of day report if teller is last one standing.
	if(tellersComplete == 3){
		endOfDayReport();
	}
	
	
	//Never return
	while(1){
		vTaskDelay(1000);
	}
	
}

/*****************************************
teller_task_init() initializes the teller_params control block and creates a task.
inputs
  int id - 0 or 1, used to differentiate task instance
  char *name - a unique human readable name for the task
  int base_ms - a base amount of time to blink an teller
  int max_jitter_ms - max variability in the blink time
outputs
  none
*******************************************/
void teller_task_init(int id, char *task_name)
{
  TELLER_PARAMS_t *p = &teller_params[id];   // get pointer to THIS instance of parameters (one for each task)
  p->id = id;                           // initialize members of this structure for this task
  strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
  xTaskCreate( teller_task, task_name, 256, (void *)p, 2, &p->handle); // go ahead and create the task 
}
