#ifndef __led_task_H    // header files should include guards
#define __led_task_H

#include "cmsis_os.h"

typedef struct {
  int id;
  TaskHandle_t handle;  // generated by FreeRTOS, saved for later use (task synchronization, etc.)
  int base_ms;
  int max_jitter_ms;
  char task_name[16];
} LED_PARAMS_t;

extern LED_PARAMS_t led_parpams[2]; // defined here, allocated in led_task.c

void led_toggle(int id);
void led_task(void *parameters);
void led_task_init(int id, char *task_name, int base_ms, int max_jitter_ms);

#endif
