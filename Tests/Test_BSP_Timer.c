#include "BSP_Timer.h"

/** 
 * @brief   Test function to see if the first Timer works
 * @param   none
 * @return  none (will print a string on console)
 */  
void functionOne()
{
  printf("First Function worked");

}

/** 
 * @brief   Test function to see of the second Timer works 
 * @param   none
 * @return  none (will print a string on console)
 */  
void functionTwo()
{
  printf("Second Function worked");
}


int main()
{
    void (*functOnepointer)= &functionOne;
    void (*functTwopointer)= &functionTwo;
    int count=0;


    BSP_Timer_Init(100,10,functOnepointer,functTwopointer);

    while(1){
      BSP_Timer_Update();
    }
    
}
