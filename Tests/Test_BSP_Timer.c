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

    BSP_Timer_Init(100,10,functOnepointer,functTwopointer);

    while(1){
      BSP_Timer_Update();
    }
    
}
