#include<stdio.h>
#include<stdlib.h>
#include "Timer.h"


int update_Time(int row)
{
    FILE *file = fopen("Timer.csv", "r");
    char arr[256];

    int counter=0;

    int current;

    int reload;


    while(fgets(arr,sizeof(arr),file))
    {
        counter++;
        if(counter == row)
        {
            fscanf(file,"%x , %x",&current,&reload);
            break;

        }

    }
 
 int subt = reload - current ;

 return subt; 
    
    
    

}

int writeReload(int reload)
{
  FILE *CANdata = fopen("CAN.csv","w");
  fprintf(CANdata,"0x%x,%d",reload,reload);
  fclose(CANdata);
}
