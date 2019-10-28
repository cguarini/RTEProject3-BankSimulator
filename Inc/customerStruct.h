#include "cmsis_os.h"

typedef struct {
  
  //customer id
  uint32_t id;
  //Time the customer entered the queue
  uint32_t timeEnteredQueue;
  //Time the customer left the queue
  uint32_t timeExitedQueue;
	//Maximum depth of the queue when the customer entered it
	uint32_t maximumDepthOfQueue;
  
} CustomerStruct_t;