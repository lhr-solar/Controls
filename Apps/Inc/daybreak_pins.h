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

#define MOTOR_PRCHG_BYPASS_PORT PORTA
#define MOTOR_PRCHG_BYPASS      GPIO_Pin_0

#define ARRAY_PRCHG_BYPASS_PORT PORTA
#define ARRAY_PRCHG_BYPASS      GPIO_Pin_1

// Lights
#define RIGHT_BLINK_PORT         PORTC
#define RIGHT_BLINK              GPIO_Pin_9

#define LEFT_BLINK_PORT         PORTA
#define LEFT_BLINK              GPIO_Pin_8

// don't ask me why BRAKE_LIGHT_PIN is a diff naming convention
#define BRAKE_LIGHT_PORT        PORTC
#define BRAKE_LIGHT_PIN         GPIO_Pin_6

#define RIGHT_INDICATOR_PORT    PORTA
#define RIGHT_INDICATOR         GPIO_Pin_15

#define LEFT_INDICATOR_PORT    PORTC
#define LEFT_INDICATOR         GPIO_Pin_10

#define OS_FAULT_PORT           PORTA
#define OS_FAULT                GPIO_Pin_5

#define BPS_HAZARD_PORT         PORTA
#define BPS_HAZARD              GPIO_Pin_6

#define CRUISE_SET_PORT         PORTA
#define CRUISE_SET              GPIO_Pin_7

#define MOTOR_C_SENSE_PORT      PORTA
#define MOTOR_C_SENSE           GPIO_Pin_9

#define MOTOR_CONTACTOR_PORT    PORTA
#define MOTOR_CONTACTOR         GPIO_Pin_10

#define RIGHT_IND_PORT          PORTA
#define RIGHT_IND               GPIO_Pin_15

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

#define CONTROLS_FAULT_PORT     PORTC
#define CONTROLS_FAULT          GPIO_Pin_4

#define CRUISE_ENABLE_PORT      PORTC
#define CRUISE_ENABLE           GPIO_Pin_5

#define HEARTBEAT_PORT          PORTC
#define HEARTBEAT_PIN           GPIO_Pin_7

#define CRUISE_IND_PORT         PORTC
#define CRUISE_IND              GPIO_Pin_8

#define LEFT_IND_PORT           PORTC
#define LEFT_IND                GPIO_Pin_10

#define BRAKE_SW_PORT           PORTC
#define BRAKE_SW                GPIO_Pin_15

// UART/USART

// USB - USART2 - PA2_TX PA3_RX
#define USB_GPIO                GPIOA
#define USB_UART                USART2
#define USB_IRQ                 USART2_IRQn
#define USB_AF                  GPIO_AF_USART2
#define USB_APB1_UART           RCC_APB1Periph_USART2
#define USB_AHB1_GPIO           RCC_AHB1Periph_GPIOA
#define USB_TX_Pinsource        GPIO_PinSource2
#define USB_TX                  GPIO_Pin_2
#define USB_RX_Pinsource        GPIO_PinSource3
#define USB_RX                  GPIO_Pin_3

// DISPLAY UART4 - PA11_RX PA12_TX
#define DISPLAY_GPIO            GPIOA
#define DISPLAY_UART            UART4
#define DISPLAY_IRQ             UART4_IRQn
#define DISPLAY_AF              GPIO_AF11_UART4
#define DISPLAY_APB1_UART       RCC_APB1Periph_UART4
#define DISPLAY_AHB1_GPIO       RCC_AHB1Periph_GPIOA
#define DISPLAY_TX_Pinsource    GPIO_PinSource12
#define DISPLAY_TX              GPIO_Pin_12
#define DISPLAY_RX_Pinsource    GPIO_PinSource11
#define DISPLAY_RX              GPIO_Pin_11

// EXTRA_UART UART5 - PB12_RX PB13_TX
#define ExtraUART_GPIO          GPIOB
#define ExtraUART_UART          UART5
#define ExtraUART_IRQ           UART5_IRQn
#define ExtraUART_AF            GPIO_AF11_UART5
#define ExtraUART_APB1_UART     RCC_APB1Periph_UART5
#define ExtraUART_AHB1_GPIO     RCC_AHB1Periph_GPIOB
#define ExtraUART_TX_Pinsource  GPIO_PinSource13
#define ExtraUART_TX            GPIO_Pin_13
#define ExtraUART_RX_Pinsource  GPIO_PinSource12
#define ExtraUART_RX            GPIO_Pin_12

#define ExtraUART_PORT          PORTB
#define ExtraUART_TX            GPIO_Pin_13
#define ExtraUART_RX            GPIO_Pin_12

// ADC

#define ADC1_APB1               RCC_APB2Periph_ADC1
#define ADC1_AHB1_GPIO          RCC_AHB1Periph_GPIOC
#define ADC1_GPIO               GPIOC

// ACCEL_POT PC0
#define ACCEL_POT_PORT          PORTC
#define ACCEL_POT_CHANNEL       ADC_Channel_10
#define ACCEL_POT               GPIO_Pin_0

// BRAKE_POT PC1
#define BRAKE_POT_PORT          PORTC
#define BRAKE_POT_CHANNEL       ADC_Channel_11
#define BRAKE_POT               GPIO_Pin_1

// EXTRA_ADC1 PC2
#define ExtraADC_1_PORT         PORTC
#define ExtraADC_1_CHANNEL      ADC_Channel_12
#define ExtraADC_1              GPIO_Pin_2

// EXTRA_ADC2 PC3
#define ExtraADC_2_PORT         PORTC
#define ExtraADC_2_CHANNEL      ADC_Channel_13
#define ExtraADC_2              GPIO_Pin_3

// CAN

// The pins have been changed in the BSP

// CarCan - CAN3 PB3_RX PB4_TX
#define CarCAN_PORT             PORTB
#define CarCAN_GPIO             GPIOB
#define CarCAN                  CAN3
#define CarCAN_AHB1_GPIO        RCC_AHB1Periph_GPIOB
#define CarCAN_APB1_CAN         RCC_APB1Periph_CAN3
#define CarCAN_AF               GPIO_AF11_CAN3
#define CarCAN_TX_Pinsource     GPIO_PinSource4
#define CarCAN_TX               GPIO_Pin_4
#define CarCAN_TX_IRQ           CAN3_TX_IRQn
#define CarCAN_RX_Pinsource     GPIO_PinSource3
#define CarCAN_RX               GPIO_Pin_3
#define CarCAN_RX_IRQ           CAN3_RX0_IRQn

// MotorCan - CAN2 PB5_RX PB6_TX
// CAN2 uses CAN1 as master
#define MotorCAN_PORT           PORTB
#define MotorCAN_GPIO           GPIOB
#define MotorCAN                CAN2
#define MotorCANMaster          CAN1 // CAN1 and CAN2 use dual CAN setup
#define MotorCAN_AHB1_GPIO      RCC_AHB1Periph_GPIOB
#define MotorCAN_APB1_CAN       RCC_APB1Periph_CAN2
#define MotorCANMaster_APB1_CAN RCC_APB1Periph_CAN1 // CAN1 is master
#define MotorCAN_AF             GPIO_AF_CAN2
#define MotorCAN_TX_Pinsource   GPIO_PinSource6
#define MotorCAN_TX             GPIO_Pin_6
#define MotorCAN_TX_IRQ         CAN2_TX_IRQn
#define MotorCAN_RX_Pinsource   GPIO_PinSource5
#define MotorCAN_RX             GPIO_Pin_5
#define MotorCAN_RX_IRQ         CAN2_RX0_IRQn

#endif