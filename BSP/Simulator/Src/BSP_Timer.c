#include "BSP_Timer.h"

#define FILE_NAME DATA_PATH(TIMER_CSV)

static void (*Timer1Handler)(void);
static void (*Timer2Handler)(void);


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
  int reload;
  int current;
  int storage[4];
  int index=0;
  FILE *file1 = fopen(FILE_NAME,"r");
  int sfno = fileno(file1);
  flock(sfno, LOCK_EX);
  for(int i=0;i<2;i++)
  {
     fscanf(file1,"%d,%d",&current,&reload);
     storage[index]=current;
     storage[index+1]=reload;
     index+=2;
  }
  flock(sfno, LOCK_UN);
  fclose(file1);
    
  if(time==TIME_1)
  {
    FILE *file = fopen(FILE_NAME,"w");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    fprintf(file,"%d,%d",TimerReload,TimerReload);
    fprintf(file,"\n%d,%d",storage[2],storage[3]);
    flock(fno, LOCK_UN);
    fclose(file);
    Timer1Handler = func;
  }
  else
  {
    FILE *file = fopen(FILE_NAME,"w");
    int fno = fileno(file);
    flock(fno, LOCK_EX);
    fprintf(file,"%d,%d",storage[0],storage[1]);
    fprintf(file,"\n%d,%d",TimerReload,TimerReload);
    flock(fno, LOCK_UN);
    fclose(file);
    Timer2Handler = func;
    
  }
}
