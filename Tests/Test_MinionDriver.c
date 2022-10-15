#include "Minions.h"
#include <bsp.h>
#include "config.h"
#include "common.h"

int test1(void);
void test2(void);

int main(void){
    test1();
    test2();
    while(1){}
}


int test1(void){

        BSP_UART_Init(UART_3);
        Minion_Init();
        bool lightState=false;

    while(1){
        char str[128];
        bool regen = Minion_Read_Input(REGEN_SW);
        bool forward = Minion_Read_Input(FOR_SW);
        bool reverse = Minion_Read_Input(REV_SW);
        bool enable = Minion_Read_Input(CRUZ_EN);
        bool set = Minion_Read_Input(CRUZ_ST);


    
        int size = sprintf(str, "regen = %d, forward = %d, reverse = %d, enable = %d, set = %d \r\n", regen, forward,reverse,enable,set);
        BSP_UART_Write(UART_3, str, size);
        
        
        lightState=!lightState;
        Minion_Write_Output(BRAKELIGHT, lightState);

        for(int i = 0; i < 999999; i++){}//should be half a second
    }
}


void test2(void){
    BSP_UART_Init(UART_3);
    Minion_Init();
    bool lightState=false;

    bool output = Minion_Write_Output(REV_SW, lightState);
    bool input = Minion_Read_Input(BRAKELIGHT);

    printf("This should print 0: %d", input);
    printf("This should print 0: %d", output);

    lightState = !lightState;
    for(int i = 0; i < 999999; i++){}//should be half a second
}