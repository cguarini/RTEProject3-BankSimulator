#include "main.h"
#include "cmsis_os.h"
#include "led_task.h"
#include "rng.h"
#include "usart.h"
#include "customerStruct.h"
#include "stdio.h"
#include "clock.h"

#include "string.h"

TaskHandle_t customer_handle = NULL;
TaskHandle_t teller_handle = NULL;
extern osMessageQId CustomerQueueHandle;
extern osMessageQId MessageQueueHandle;
uint8_t bankClosedFlag = 0;


void customer_data_task(void *parameters)
{
	int i = 0;
  uint32_t delay_ms;

  for(;;){	

    CustomerStruct_t customer;
    char str[100];
		
		
		uint32_t time = xTaskGetTickCount();
		//After 4:00, everyone go home
		if(time > END_TIME){
			break;
		}
		

    //create a random delay for the customer arrival:
    HAL_RNG_GenerateRandomNumber(&hrng, &delay_ms);
		delay_ms = 100 + (delay_ms % 300); 
    vTaskDelay(delay_ms);
		
		//Get the time string
		char timeStr[10];
		getTimeString(timeStr, time);
    
		customer.id = i;
		customer.timeEnteredQueue = time;
    customer.timeExitedQueue = 0;
    customer.maximumDepthOfQueue = (uxQueueMessagesWaiting(CustomerQueueHandle) + 1);
    sprintf(str, "%s - Placing Customer %d in queue\r\n", timeStr, customer.id);
    xQueueSend(MessageQueueHandle, &str, 200);
    BaseType_t success = xQueueSend( CustomerQueueHandle, &customer, 20 );
    if(!success){
      sprintf(str, "%s - Unable to place Customer %d in queue\r\n", timeStr, customer.id);
      xQueueSend(MessageQueueHandle, &str, 0);
    }

		i++;
  }
	
	//Loop is broken, end of day.
	char str[100];
	sprintf(str, "----------End of Day, no more customers may enter the bank.----------\r\n");
	xQueueSend(MessageQueueHandle, &str, 20);
	
	bankClosedFlag = 1;

	//Delete the customer task
	vTaskDelete(customer_handle);
	
	//NEVER RETURN
	while(1);
	
}

void customer_task_init()
	
{
  xTaskCreate( customer_data_task, "Customer Task", 256, (void *) 0, 2,customer_handle); // go ahead and create the task 
	//xTaskCreate( teller_task, "teller Task", 256, (void *) 0, 1,teller_handle);
}
