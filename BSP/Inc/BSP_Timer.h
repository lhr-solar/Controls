#include<stdio.h>
#include<stdlib.h>

int (*function1);    //function pointers
int (*function2);  
void BSP_Update_Time(int row);
void BSP_Timer_Init(int reload, int *func1, int *func2);
