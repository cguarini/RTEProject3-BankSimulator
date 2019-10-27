#include "main.h"
#include "cmsis_os.h"
#include "led_task.h"
#include "rng.h"
#include "usart.h"
#include "customerStruct.h"

#include "string.h"

TaskHandle_t customer_handle = NULL;
TaskHandle_t teller_handle = NULL;
extern osMessageQId CustomerQueueHandle;


void customer_data_task(void *parameters)
{
	int i = 0;
  uint32_t delay_ms;

	TickType_t val;
  
  CustomerStruct_t customer;

	

for(int i=0; i < 10; i++){	

    CustomerStruct_t customer;

    // To create a random delay for the customer arrival:
		
  /*
    HAL_RNG_GenerateRandomNumber(&hrng, &delay_ms);
		delay_ms = 100 + (delay_ms % 300);  
		USART_Printf("Customer will arrive in % 4d milli seconds\r\n", delay_ms);
    vTaskDelay(delay_ms);
		val = xTaskGetTickCount( );
		USART_Printf("Tick Count : %ld \r\n", val);
		*/
		customer.id = i;
		customer.timeEnteredQueue = val;
    customer.timeExitedQueue = 0;
    USART_Printf("Placing Customer %d in queue\r\n", customer.id);
    vTaskDelay(1000);
    BaseType_t success = xQueueSend( CustomerQueueHandle, &customer, 0 );
    if(!success){
      USART_Printf("Unable to place Customer %d in queue\r\n", customer.id);
    }

		
  }
}

void customer_task_init()
	
{
  xTaskCreate( customer_data_task, "Customer Task", 256, (void *) 0, 2,customer_handle); // go ahead and create the task 
	//xTaskCreate( teller_task, "teller Task", 256, (void *) 0, 1,teller_handle);
}
