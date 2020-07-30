#include "BSP_Timer.h"

#define FILE_NAME DATA_PATH(TIMER_CSV)

static void (*Timer1Handler)(void);
static void (*Timer2Handler)(void);
int TempValue = 0xFFFFFF; //variable to store reload value written to CSV previously because everyhtime soemthing new is written previous values are erased


/** 
* @brief   Updates the time by reading the CSV file and calling relevant functions at the right time 
* @param   none
* @return  none
*/  
void BSP_Timer_Update()
{
    FILE *file = fopen(FILE_NAME, "r");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    int timeCurrent[2];
    int reload;
    int index=0;
    
    for(int i=0;i<2;i++)
    {
        fscanf(file,"%d , %d",timeCurrent+index,&reload);
        index++;
    }
    flock(fno, LOCK_UN);
    fclose(file);
 

     if(timeCurrent[0]==0)
      {
          Timer1Handler();
      }
      if(timeCurrent[1]==0)
      {
          Timer2Handler(); 
      }
  }

/** 
* @brief   Intializes the Timer
* @param   Timer Reload value 
* @param   Function Pointer
* @param   Specefic Timer (Timer 1 = 0 and Timer 2 = 1)
* @return  none
 */  

void BSP_Timer_Init(int TimerReload,void *func,Time_t time)
{
  if(time==Time1)
  {
    FILE *file = fopen(FILE_NAME,"w");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    fprintf(file,"%d,%d",TimerReload,TimerReload);
    fprintf(file,"\n%d,%d",TempValue,TempValue);
    flock(fno, LOCK_UN);
    fclose(file);
    Timer1Handler = func;
    TempValue=TimerReload;
  }
  else
  {
    FILE *file = fopen(FILE_NAME,"w");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    fprintf(file,"%d,%d",TempValue,TempValue);
    fprintf(file,"\n%d,%d",TimerReload,TimerReload);
    flock(fno, LOCK_UN);
    fclose(file);
    Timer2Handler = func;
    TempValue=TimerReload;
    
  }
}
