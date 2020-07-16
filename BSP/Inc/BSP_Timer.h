#include<stdio.h>
#include<stdlib.h>

void (*function1);    //function pointers
void (*function2);  
void BSP_Update_Time(int8_t row);
void BSP_Timer_Init(int8_t reload, void *func1, void *func2);
