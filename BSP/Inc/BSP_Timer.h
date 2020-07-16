#include<stdio.h>
#include<stdlib.h>

void (*function1);    //function pointers
void (*function2);  
void BSP_Update_Time(int row);
void BSP_Timer_Init(int reload, void *func1, void *func2);
