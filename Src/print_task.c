#include "FreeRTOS.h"
#include "usart.h"
#include "string.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os.h"
#include "print_task.h"

extern osMessageQId MessageQueueHandle;
PRINT_PARAMS_t print_params[3]; // defined here, allocated in led_task.c


void print_task(void *parameters)
{
  while(1){
    
    char str[100];
    
    BaseType_t success = xQueueReceive(MessageQueueHandle, &str, 0);
    
    if(!success){
      //USART_Printf("Unable to remove print from queue\r\n");
      vTaskDelay(20);
      continue;
    }
    
    USART_Printf(str);
    
  }
}

/*****************************************
print_task_init() initializes the print_params control block and creates a task.
inputs
  int id - 0 or 1, used to differentiate task instance
  char *name - a unique human readable name for the task
  int base_ms - a base amount of time to blink an print
  int max_jitter_ms - max variability in the blink time
outputs
  none
*******************************************/
void print_task_init(int id, char *task_name)
{
  PRINT_PARAMS_t *p = &print_params[id];   // get pointer to THIS instance of parameters (one for each task)
  p->id = id;                           // initialize members of this structure for this task
  strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
  xTaskCreate( print_task, task_name, 256, (void *)p, 2, &p->handle); // go ahead and create the task 
}
