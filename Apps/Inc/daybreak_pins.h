/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file daybreak_pins.h
 * @brief 
 * 
 * @defgroup daybreak_pins
 * @addtogroup daybreak_pins
 * @{
 */

#ifndef __DAYBREAK_PINS_H
#define __DAYBREAK_PINS_H

#include <stm32f4xx_gpio.h>

// GPIO

#define PC13_PORT               PORTC
#define PC13                    GPIO_Pin_13

#define PC14_PORT               PORTC
#define PC14                    GPIO_Pin_14

#define BRAKE_SW_PORT           PORTC
#define BRAKE_SW                GPIO_Pin_15

#define MOTOR_PRCHG_BYPASS_PORT PORTA
#define MOTOR_PRCHG_BYPASS      GPIO_Pin_0

#define ARRAY_PRCHG_BYPASS_PORT PORTA
#define ARRAY_PRCHG_BYPASS      GPIO_Pin_1

#define BRAKE_LIGHT_PORT        PORTA
#define BRAKE_LIGHT             GPIO_Pin_4

#define OS_FAULT_PORT           PORTA
#define OS_FAULT                GPIO_Pin_5

#define BPS_HAZARD_PORT         PORTA
#define BPS_HAZARD              GPIO_Pin_6

#define CRUISE_SET_PORT         PORTA
#define CRUISE_SET              GPIO_Pin_7

#define CONTROLS_FAULT_PORT     PORTC
#define CONTROLS_FAULT          GPIO_Pin_4

#define CRUISE_ENABLE_PORT      PORTC
#define CRUISE_ENABLE           GPIO_Pin_5

#define BPS_FAULT_PORT          PORTB
#define BPS_FAULT               GPIO_Pin_0

#define IG1_PORT                PORTB
#define IG1                     GPIO_Pin_1

#define FORWARD_PORT            PORTB
#define FORWARD                 GPIO_Pin_2

#define IG2_PORT                PORTB
#define IG2                     GPIO_Pin_10

#define MOTOR_CTRL_FAULT_PORT   PORTB
#define MOTOR_CTRL_FAULT        GPIO_Pin_14

#define REVERSE_PORT            PORTB
#define REVERSE                 GPIO_Pin_15

#define TIMER_CLK_PORT          PORTC
#define TIMER_CLK               GPIO_Pin_6

#define HEARTBEAT_PORT          PORTC
#define HEARTBEAT               GPIO_Pin_7

#define CRUISE_IND_PORT         PORTC
#define CRUISE_IND              GPIO_Pin_8

#define PC9_PORT                PORTC
#define PC9                     GPIO_Pin_9

#define PA8_PORT                PORTA
#define PA8                     GPIO_Pin_8

#define MOTOR_C_SENSE_PORT      PORTA
#define MOTOR_C_SENSE           GPIO_Pin_9

#define MOTOR_CONTACTOR_PORT    PORTA
#define MOTOR_CONTACTOR         GPIO_Pin_10

#define PA15_PORT               PORTA
#define PA15                    GPIO_Pin_15

#define PC10_PORT               PORTC
#define PC10                    GPIO_Pin_10

#define PC11_PORT               PORTC
#define PC11                    GPIO_Pin_11

#define PC12_PORT               PORTC
#define PC12                    GPIO_Pin_12

#define PD2_PORT                PORTD
#define PD2                     GPIO_Pin_2

#define PB7_PORT                PORTB
#define PB7                     GPIO_Pin_7

#define ARRAY_PBC_SENSE_PORT    PORTB
#define ARRAY_PBC_SENSE         GPIO_Pin_8

#define MOTOR_PBC_SENSE_PORT    PORTB
#define MOTOR_PBC_SENSE         GPIO_Pin_9

// UART/USART

// The pins have been changed in BSP_UART.C (hopefully correctly)
// No extra_uart_init (TODO)

// USB - USART2 - PA2_TX PA3_RX
#define USB_PORT PORTA
#define USB_TX   GPIO_Pin_2
#define USB_RX   GPIO_Pin_3

// DISPLAY UART4 - PA11_RX PA12_TX
#define DISPLAY_PORT PORTA
#define DISPLAY_TX   GPIO_Pin_12
#define DISPLAY_RX   GPIO_Pin_11

// EXTRA_UART UART5 - PB12_RX PB13_TX
#define ExtraUART_PORT PORTB
#define ExtraUART_TX   GPIO_Pin_13
#define ExtraUART_RX   GPIO_Pin_12

// ADC

// no extra_adc_init (TODO)

// ACCEL_POT PC0
#define ACCEL_POT_PORT PORTC
#define ACCEL_POT GPIO_Pin_0

// BRAKE_POT PC1
#define BRAKE_POT_PORT PORTC
#define BRAKE_POT GPIO_Pin_1

// EXTRA_ADC1 PC2
#define ExtraADC1_PORT PORTC
#define ExtraADC1 GPIO_Pin_2

// EXTRA_ADC2 PC3
#define ExtraADC2_PORT PORTC
#define ExtraADC2 GPIO_Pin_3

// CAN

// The pins have been changed in the BSP

// CarCan - CAN3 PB3_RX PB4_TX
#define CAN_PORT PORTB
#define CAN_TX GPIO_Pin_4
#define CAN_RX GPIO_Pin_3

// MotorCan - CAN2 PB5_RX PB6_TX
#define MotorCAN_PORT PORTB
#define MotorCAN_TX GPIO_Pin_6
#define MotorCAN_RX GPIO_Pin_5






#endif