#include "BSP_Timer.h"

static void (*Timer1Handler)(int);
static void (*Timer2Handler)(int);
//had to pass some sort of parameter to the pointer or it doesn't work

/** 
* @brief   Updates the time by reading the CSV file and calling relevant functions at the right time 
* @param   none
* @return  none
*/  
void BSP_Timer_Update()
{
    FILE *file = fopen("Timer.csv", "r");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    int timeCurrent[2];
    int current;
    int reload;
    int i=0;
    int index=0;
    
    while(i<2)
    {
        fscanf(file,"%d , %d",&current,&reload);
        timeCurrent[index]=current;
        index++;
        i++;
        
    }
    flock(fno, LOCK_UN);
    fclose(file);
 

     if(timeCurrent[0]==0)
      {
          Timer1Handler(0);
      }
      if(timeCurrent[1]==0)
      {
          Timer2Handler(0); 
      }
  }

/** 
* @brief   Intializes the Timer
* @param   First reload value for Timer one
* @param   Second reload value for Timer two
* @param   First function pointer
* @param   Second function pointer
* @return  none
 */  

void BSP_Timer_Init(int Timer1Reload,int Timer2Reload,void *func1,void *func2)
{
  FILE *TimerData = fopen("Timer.csv","w");
  fprintf(TimerData,"%d,%d",Timer1Reload,Timer1Reload);
  fprintf(TimerData,"\n%d,%d",Timer2Reload,Timer2Reload);
  fclose(TimerData);
  Timer1Handler = func1;
  Timer2Handler = func2;
}
