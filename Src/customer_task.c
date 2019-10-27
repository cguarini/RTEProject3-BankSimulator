#include "main.h"
#include "cmsis_os.h"
#include "led_task.h"
#include "rng.h"
#include "usart.h"
#include "customerStruct.h"
#include "stdio.h"

#include "string.h"

TaskHandle_t customer_handle = NULL;
TaskHandle_t teller_handle = NULL;
extern osMessageQId CustomerQueueHandle;
extern osMessageQId MessageQueueHandle;


void customer_data_task(void *parameters)
{
	int i = 0;
  uint32_t delay_ms;

	TickType_t val;
  
  CustomerStruct_t customer;

	

  for(;;){	

    CustomerStruct_t customer;
    char str[100];

    // To create a random delay for the customer arrival:
		
  
    HAL_RNG_GenerateRandomNumber(&hrng, &delay_ms);
		delay_ms = 100 + (delay_ms % 300); 
    vTaskDelay(delay_ms);
		
		customer.id = i;
		customer.timeEnteredQueue = val;
    customer.timeExitedQueue = 0;
    sprintf(str, "Placing Customer %d in queue\r\n", customer.id);
    xQueueSend(MessageQueueHandle, &str, 0);
    BaseType_t success = xQueueSend( CustomerQueueHandle, &customer, 20 );
    if(!success){
      sprintf(str, "Unable to place Customer %d in queue\r\n", customer.id);
      xQueueSend(MessageQueueHandle, &str, 0);
    }

		i++;
  }
}

void customer_task_init()
	
{
  xTaskCreate( customer_data_task, "Customer Task", 256, (void *) 0, 2,customer_handle); // go ahead and create the task 
	//xTaskCreate( teller_task, "teller Task", 256, (void *) 0, 1,teller_handle);
}
