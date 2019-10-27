#include "cmsis_os.h"
#include "queue.h"

//void led_toggle(int id);
void customer_data_task(void *parameters);
void customer_task_init();
void teller_task_init();