#include "main.h"
#include "cmsis_os.h"
#include "teller_task.h"
#include "rng.h"
#include "usart.h"
#include "string.h"
#include "queue.h"
#include "customerStruct.h"

extern osMessageQId CustomerQueueHandle;
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
  

  while(1) {
    
    //Remove a customer from the queue, if not customer, don't block
    xQueueReceive(CustomerQueueHandle, &customer, 0);
    //Mark when the customer was pulled from the queue
    customer.timeExitedQueue = xTaskGetTickCount();
    
    //service the customer for 30 seconds to 8 minutes
    
    //Leftover from LED_TASK
    HAL_RNG_GenerateRandomNumber(&hrng, &wait_ms);
    wait_ms = p->base_ms + (wait_ms % p->max_jitter_ms);  // 1000-1256
    USART_Printf("Teller %s was hit after %d\r\n", p->task_name, wait_ms);
    vTaskDelay(wait_ms);
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
void teller_task_init(int id, char *task_name, int base_ms, int max_jitter_ms)
{
  TELLER_PARAMS_t *p = &teller_params[id];   // get pointer to THIS instance of parameters (one for each task)
  p->id = id;                           // initialize members of this structure for this task
  p->base_ms = base_ms;
  p->max_jitter_ms = max_jitter_ms;
  strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
  xTaskCreate( teller_task, "teller TASK2", 256, (void *)p, 2, &p->handle); // go ahead and create the task 
}
