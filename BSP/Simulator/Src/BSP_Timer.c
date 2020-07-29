#include "BSP_Timer.h"

static void (*Timer1Handler)(void);
static void (*Timer2Handler)(void);
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
* @param   First reload value for Timer one
* @param   Second reload value for Timer two
* @param   First function pointer
* @param   Second function pointer
* @return  none
 */  

void BSP_Timer_Init(int Timer1Reload,int Timer2Reload,void *func1,void *func2)
{
  FILE *file = fopen("Timer.csv","w");
  int fno = fileno(file);
  flock(fno, LOCK_EX);
  fprintf(file,"%d,%d",Timer1Reload,Timer1Reload);
  fprintf(file,"\n%d,%d",Timer2Reload,Timer2Reload);
  flock(fno, LOCK_UN);
  fclose(file);
  Timer1Handler = func1;
  Timer2Handler = func2;
}
