#include<stdio.h>
#include<stdlib.h>
#include "Timer.h"


int8_t BSP_Timer_Update(int8_t row)
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
 
 int8_t subt = reload - current ;

 return subt; 
    
    
    

}

int8_t BSP_Timer_Init(int8_t reload)
{
  FILE *TIMERdata = fopen("Timer.csv","w");
  fprintf(TIMERdata,"0x%x",reload);
  fclose(TIMERdata);
}
