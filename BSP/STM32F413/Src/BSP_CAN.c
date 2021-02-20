/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Pin Configuration:
 * 
 * CAN1 RX - PA11
 * CAN1 TX - PA12
 */

#include "BSP_CAN.h"
#include "stm32f4xx.h"

static CanTxMsg TxMessage;
static CanRxMsg RxMessage;
static bool RxFlag = false;

/**
 * @brief   Initializes both CAN lines to
 *          communicate with the motor controllers
 *          and other car systems
 * @param   bus the CAN line to initialize
 * @return  None
 */ 
void BSP_CAN_Init(CAN_t bus) {
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_InitTypeDef CAN_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    CAN_FilterInitTypeDef CAN_FilterInitStruct;

    /* CAN GPIO Configuration *****************************/
    
    // Clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Alternate Function 8
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF8_CAN1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF8_CAN1);

    // Pin config
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN Configuration **********************************/
    
    // Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    // CAN Cell Init
    CAN_InitStruct.CAN_TTCM = DISABLE;
    CAN_InitStruct.CAN_ABOM = DISABLE;
    CAN_InitStruct.CAN_AWUM = DISABLE;
    CAN_InitStruct.CAN_NART = DISABLE;
    CAN_InitStruct.CAN_RFLM = DISABLE;
    CAN_InitStruct.CAN_TXFP = DISABLE;
    CAN_InitStruct.CAN_Mode = CAN_Mode_Normal;

    /**
     * CAN Baudrate = 125 KBps
     * 1/(prescaler + (prescaler*(BS1+1)) + (prescaler*(BS2+1))) * Clk = CAN Baudrate
     * The AHB1 clk which drives CAN seems to be set to around 20MHz, but this should be verified.
     */
    CAN_InitStruct.CAN_BS1 = CAN_BS1_3tq;
    CAN_InitStruct.CAN_BS2 = CAN_BS2_4tq;
    CAN_InitStruct.CAN_Prescaler = 16;
    CAN_Init(CAN1, &CAN_InitStruct);

    // CAN Filter Init
    CAN_FilterInitStruct.CAN_FilterNumber = 0;
    CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStruct.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStruct.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(CAN1, &CAN_FilterInitStruct);

    // Transmit Structure
    TxMessage.ExtId = 0x5;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.DLC = 1;

    // Receive Structure
    RxMessage.StdId = 0x00;
    RxMessage.ExtId = 0x00;
    RxMessage.IDE = CAN_ID_STD;
    RxMessage.DLC = 0;
    RxMessage.FMI = 0;

    // FIFO 0 Interrupt Config
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    // Enable Rx Interrupts
    NVIC_InitStruct.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief   Writes a message to the specified CAN line
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id the hex ID for the message to be sent
 * @param   data pointer to the array containing the message
 * @param   len length of the message in bytes
 * @return  number of bytes transmitted (0 if unsuccessful)
 */
uint8_t BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t* data, uint8_t len) {
    TxMessage.StdId = id;
    TxMessage.DLC = len;
    for (uint8_t i = 0; i < len; i++) {
        TxMessage.Data[i] = data[i];
    }
    return CAN_Transmit(CAN1, &TxMessage);
}

/**
 * @brief   Reads the message on the specified CAN line
 * @param   id pointer to integer to store the 
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes (must be at least 8 bytes or hardfault may occur)
 * @return  number of bytes read (0 if unsuccessful)
 */
uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data) {
    if (RxFlag) {
        for (uint8_t i = 0; i < 8; i++) {
            data[i] = RxMessage.Data[i];
        }
        *id = RxMessage.StdId;
        RxFlag = false;
        return 1;
    }
    return 0;
}

/**
 * Interrupt Handler
 */
void CAN1_RX0_IRQHandler(void) {
   CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    if ((RxMessage.StdId == 0x005) && (RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1)) {
        RxFlag = true;
    }
}
