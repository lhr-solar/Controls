#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include "BSP_Timer.h"

//temporary test function
void functionOne()
{
  std::cout<<"First Function worked";

}

void functionTwo()
{
  std::cout<<"Second Function worked";
}


int main()
{
    void (*functOnepoiner)= &functionOne;
    void (*functTwopoiner)= &functionTwo;
    int8_t count=0;


    BSP_Timer_Init(1000,functOnepoiner,functTwopoiner);

    while(1){
      count++;
      BSP_Update_Time(count);
    }
    
}
