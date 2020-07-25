#include "BSP_Timer.h"

static void (*function1);
static void (*function2);
int row=0; //keeps track of row number in CSV file
/** 

 * @brief   Updates the time by reading the CSV file and calling relevant functions at the right time 

 * @param   none

 * @return  none

 */  

void BSP_Timer_Update()
{
    FILE *file = fopen("Timer.csv", "r");
    char arr[300];

    int counter=0;

    int current1;

    int current2;


    while(fgets(arr,sizeof(arr),file))
    {
        counter++;
        if(counter == row)
        {
            fscanf(file,"%x , %x",&current1,&current2);
            row++;
            break;

        }

    }
 fclose(file);
 

     if(current1==0)
      {
          (*function1);//sendDashboard func called
      }
      if(current2==0)
      {
         (*function2); //sendMC func called
      }
    

    

}
/** 

 * @brief   Intializes the Timer

 * @param   2 reload values for two timers, Two function pointers 

 * @return  none

 */  

void BSP_Timer_Init(int Timer1Reload,int Timer2Reload,void *func1,void *func2)
{
  FILE *TimerData = fopen("Timer.csv","w");
  fprintf(TimerData,"0x%x , 0x%x",Timer1Reload,Timer2Reload);
  fclose(TimerData);
  function1 = func1;
  function2 = func2;
}
