#include "BSP_ADC.h"
#include "BSP_CAN.h"
#include "BSP_Contactors.h"
#include "BSP_Switches.h"
#include "BSP_UART.h"


int main()
{
    //ADC Test 1
    //random ADC value = 256
    int16_t x = BSP_ADC_Get_Value(ACCELERATOR);
    printf("Input: Accelerator Expected: 756  Real: %d",x);
    if(x == 756)
    {
       printf("Passed Test");
    }
    else
    {
       printf("Failed Test");
    }
    
    
    x = BSP_ADC_Get_Millivoltage(ACCELERATOR);
    printf("Input: Accelerator Expected: 756  Real: %d",x);
    if(x == 609)
    {
       printf("Passed Test");
    }
    else
    {
       printf("Failed Test");
    }



    //CAN Test 
    uint8_t array[2]={25,45};
    uint8_t y = BSP_CAN_Write(CAN_1, 0x222, array, 2);
    if(y == 2)
    {
       printf("Test Passed");
    }
    else
    {
       printf("Test Failed");
    }

    int ID;
    int array[2];
    y= BSP_CAN_Read(CAN_1, &ID, array);
     if(y == 2)
    {
       printf("Test Passed");
    }
    else
    {
       printf("Test Failed");
    }

    //Contractors Test
    state_t x;
    x = BSP_Contactors_Get(MOTOR);
     if(x == ON)
    {
       printf("Test Passed");
    }
    else
    {
       printf("Test Failed");
    }
  

    BSP_Contactors_Set(MOTOR,OFF);
    FILE *fp = fopen("COntractors.csv","r");
    fscanf(fp,"%d",&y);
    fclose(fp);
    if(y == OFF)
    {
       printf("Test Passed");
    }
    else
    {
       printt("Test Failed");
    }

    //Switches Test
    x = BSP_Switches_Read(RT);
    if(x == ON)
    {
       printf("Test Passed");
    }
    else
    {
      printf("Test Failed");
    }

    //UART Test
    char string[]="Hello there!";
    uint32_t t = BSP_UART_Write(string, 13);
    if(t == 13)
    {
       printf("Test Passed");
    }
    else
    {
       printf("Test Failed");
    }

    
    char buffer[13];

    t = BSP_UART_Read(buffer);
    int flag = 0;
    if(t==13)
    {
       for(int i = 0;i<13; i++)
       {
          if(string[i]!=buffer[i])
          {
             printf("Test Failed");
             flag=1;
             break;
          }
       }
       if(flag == 0)
       {
          printf("Test Passed");
       }
    }
    else
    {
       printf("Test Failed");
    }
    


}
