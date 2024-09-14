#ifndef MYTASK_H
#define MYTASK_H
#include "struct_typedef.h"
#include "PID.h"

extern float motor_current_output[4];

void motor_task (void const *pvParameters);


#endif

