#include "BSP_Timer.h"

/** 
 * @brief   Test function to see if the first Timer works
 * @param   Had to pass a parameter in order for pointer to work in BSP_Timer.c
 * @return  none (will print a string on console)
 */  
void functionOne(int i)
{
  printf("First Function worked\n");

}

/** 
 * @brief   Test function to see of the second Timer works 
 * @param   Had to pass a parameter in order for pointer to work in BSP_Timer.c
 * @return  none (will print a string on console)
 */  
void functionTwo(int j)
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
