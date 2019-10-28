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

extern osMessageQId CustomerQueueHandle;
extern osMessageQId MessageQueueHandle;
TELLER_PARAMS_t teller_params[3]; // defined here, allocated in led_task.c


/*****************************************
teller_task() controls the blinking of an teller at a random rate (with parameters)
Note: there may be multiple instances of teller_task.
inputs
  void *parameters - a pointer to the teller_PARAMS_t parameter block for this instance
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
	uint32_t breakStartTime = xTaskGetTickCount() + (wait_ms % 30) + 30;
	uint32_t breakLength = (wait_ms % 3) + 1;
  

  while(1) {
		//Get the start time of this loop
		char timeStr[10];
		uint32_t time = xTaskGetTickCount();
		getTimeString(timeStr, time);
		
		//Break Logic
		if(breakStartTime < time){
			//go on break
			sprintf(str, "%s - Teller %d is going on break for %d minutes\r\n", timeStr, p->id, breakLength);
			xQueueSend(MessageQueueHandle, &str, 0);
			vTaskDelay(breakLength);
			
			//generate new break time
			HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);
			breakStartTime = xTaskGetTickCount() + (wait_ms % 30) + 30;
			breakLength = (wait_ms % 3) + 1;
		}
    
    //Remove a customer from the queue, if not customer, don't block
    BaseType_t success = xQueueReceive(CustomerQueueHandle, &customer, 20);
    
    //Check if customer was successfully retrieved from queue
    if(!success){
			
			if(time > END_TIME){
				//End of day and no customers are in the queue
				break;
			}
			
      //try again if not successful, (queue was empty?)
      vTaskDelay(10);
      continue;
    }
    
    sprintf(str, "%s - Teller %d received customer %d from queue.\r\n",timeStr, p->id, customer.id);
    xQueueSend(MessageQueueHandle, &str, 0);
    
    //Mark when the customer was pulled from the queue
    customer.timeExitedQueue = time;
    
    //service the customer for 30 seconds to 8 minutes
    HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);//generate the random number
    wait_ms = ((wait_ms % 750) + 50);//set it to between 50 and 800 (30 seconds to 8 minutes)
    vTaskDelay(wait_ms);
    
  }
	
	//Get the start time of this loop
	char timeStr[6];
	uint32_t time = xTaskGetTickCount();
	getTimeString(timeStr, time);
	
	sprintf(str, "%s - Teller %s is out of customers and going home.\r\n",timeStr, p->task_name);
  xQueueSend(MessageQueueHandle, &str, 20);
	
	
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
