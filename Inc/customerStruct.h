#include "cmsis_os.h"

typedef struct {
  
  //Time the customer entered the queue
  uint32_t timeEnteredQueue;
  //Time the customer left the queue
  uint32_t timeExitedQueue;
  
  
} CustomerStruct_t;