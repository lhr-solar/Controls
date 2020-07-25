#include "BSP_Timer.h"

//temporary test function
void functionOne()
{
  printf("First Function worked");

}

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
