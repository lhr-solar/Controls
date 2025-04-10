#include "daybreak_pins.h"
#include "BSP_GPIO.h"

int main(void)
{

    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);
    BSP_GPIO_Init(OS_FAULT_PORT, OS_FAULT, OUTPUT, false);

    while (1)
    {
        volatile uint32_t waitTimeMain = 0;
        while (waitTimeMain <= 99999)
        {
            volatile uint32_t waitTime = 0;
            BSP_GPIO_Toggle_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT);
            BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
            while (waitTime <= 11000)
            {
                waitTime++;
            }
            waitTimeMain++;
        }

        waitTimeMain = 0;
        while (waitTimeMain <= 99999)
        {
            volatile uint32_t waitTime = 0;
            BSP_GPIO_Toggle_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT);
            BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
            while (waitTime <= 11000)
            {
                waitTime++;
            }
            waitTimeMain++;
        }
        // BSP_GPIO_Toggle_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT);
        // BSP_GPIO_Toggle_Pin(OS_FAULT_PORT, OS_FAULT);
        // waitTime = 0;
        // while (waitTime <= 9999)
        // {
        //     waitTime++;
        // }
    }
}