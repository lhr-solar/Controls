/**
 * Test file for library to interact with the pedal driver file
 * 
 * Run this test in conjunction with the simulator 
 * GUI. As you move the accelerator and brake on the GUI, the respective
 * pressed/slided percentage will change from '0' to '100' on the terminal and display
 * to show that sliding the pedals is read by the BSP
 * 
 */ 

#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "StatusLeds.h"
#include "BSP_GPIO.h"
#include <bsp.h>


int main() {
    Pedals_Init();
    Status_Leds_Init();
    // BSP_GPIO_Init(PORTC, 0x0, INPUT, true);
    // BSP_GPIO_Init(BRAKE_POT_PORT, BRAKE_POT, INPUT, true);
    // BSP_GPIO_Init(ACCEL_POT_PORT, ACCEL_POT, INPUT, true);
    BSP_UART_Init(USB);

    while(1) {
        // uint8_t brake_gpio = BSP_GPIO_Read_Pin(BRAKE_POT_PORT, BRAKE_POT);
        // printf("Brake Digital Val: %d\n\r", (brake_gpio));
        int16_t brake_adc = BSP_ADC_Get_Value(BRAKE);
        printf("Brake Raw ADC Val: %d\n\r", brake_adc);
        int16_t brake_mV = BSP_ADC_Get_Millivoltage(BRAKE);
        printf("Brake mV: %d\n\r", brake_mV);
        int8_t brake_percent = Pedals_Read(BRAKE);
        printf("Brake Percent: %d\n\r", brake_percent);
        for(int i = 0; i < 500000; i++){}
    }

    // while(1) {
    //     printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r", 
    //         Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
    // }
}

