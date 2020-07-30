#include "BSP_Timer.h"

/** 
 * @brief   Test function to see if the first Timer works
 * @param   None
 * @return  none (will print a string on console)
 */  
void functionOne()
{
  printf("First Function worked\n");

}

/** 
 * @brief   Test function to see of the second Timer works 
 * @param   None
 * @return  none (will print a string on console)
 */  
void functionTwo()
{
  printf("Second Function worked\n");
}


int main()
{

    BSP_Timer_Init(1000,functionOne,TIMER_1);
    BSP_Timer_Init(100,functionTwo,TIMER_2);
    while(1){
      BSP_Timer_Update();
    }
    
}
