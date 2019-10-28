#include "main.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"

char const* getTimeString( char * str, uint32_t time ){
	uint32_t hours = ((time / 100) / 60) + 9;
	uint32_t minutes = (time / 100) % 60;
	hours > 12 ? hours -= 12 : hours;
	sprintf(str, "%02d:%02d", hours, minutes);
	return str;
}