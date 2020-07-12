#include<stdio.h>
#include<stdlib.h>
#include "Timer.h"


// function keeps updating time and will interrupt at the right time to go to specefic functions
void BSP_Update_Time(int8_t row)           
{
    FILE *file = fopen("Timer.csv", "r");
    char arr[256];

    int8_t counter=0;

    int8_t current;

    int8_t reload;


    while(fgets(arr,sizeof(arr),file))
    {
        counter++;
        if(counter == row)
        {
            fscanf(file,"%x , %x",&current,&reload);
            break;

        }

    }
 fclose(file);
 
 int8_t subt = reload - current ;

     if(subt==100)
      {
          (*function1);//sendDashboard func called
      }
      if(subt==10)
      {
         (*function2); //sendMC func called
      }
    
    
    

}
//Function for timer intialization in which the reload value is written on CSV and the function pointers are stored in global variables to be accessed later by Update function
void BSP_Timer_Init(int8_t reload,int *func1,int *func2)
{
  FILE *TimerData = fopen("Timer.csv","w");
  fprintf(TimerData,"0x%x,%d",reload,reload);
  fclose(TimerData);
  function1 = func1;
  function2 = func2;   //function pointers declared in Timer.h file
}
