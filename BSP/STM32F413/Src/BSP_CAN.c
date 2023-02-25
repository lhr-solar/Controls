/* Copyright (c) 2020 UT Longhorn Racing Solar */
#include "BSP_CAN.h"
#include "stm32f4xx.h"
#include "os.h"

// The message information that we care to receive
typedef struct _msg
{
    uint32_t id;
    uint8_t data[8];
} msg_t;

// Set up a fifo for receiving
#define FIFO_TYPE msg_t
#define FIFO_SIZE 25
#define FIFO_NAME msg_queue
#include "fifo.h"

#define NUM_FILTER_REGS 4   // Number of 16 bit registers for ids in one CAN_FilterInit struct

//return error if someone tries to call from motor can

static msg_queue_t gRxQueue[2];

// Required for receiving CAN messages
static CanTxMsg gTxMessage[2];
static CanRxMsg gRxMessage[2];

// User parameters for CAN events
static callback_t gRxEvent[2];
static callback_t gTxEnd[2];

void BSP_CAN1_Init(uint16_t* idWhitelist, uint8_t idWhitelistSize);
void BSP_CAN3_Init(uint16_t* idWhitelist, uint8_t idWhitelistSize);

/**
 * @brief   Initializes the CAN module that communicates with the rest of the electrical system.
 * @param   rxEvent : the function to execute when recieving a message. NULL for no action.
 * @param   txEnd   : the function to execute after transmitting a message. NULL for no action.
 * @return  None
 */

void BSP_CAN_Init(CAN_t bus, callback_t rxEvent, callback_t txEnd, uint16_t* idWhitelist, uint8_t idWhitelistSize) {

    // Configure event handles
    gRxEvent[bus] = rxEvent;
    gTxEnd[bus] = txEnd;

    if (bus == CAN_1)
    {
        BSP_CAN1_Init(idWhitelist, idWhitelistSize);
    }
    else
    {
        BSP_CAN3_Init(idWhitelist, idWhitelistSize);
    }
}

void BSP_CAN1_Init(uint16_t* idWhitelist, uint8_t idWhitelistSize) {
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_InitTypeDef CAN_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    CAN_FilterInitTypeDef CAN_FilterInitStruct;

    // Initialize the queue
    gRxQueue[0] = msg_queue_new();

    /* CAN GPIOs configuration **************************************************/

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Alternate Function 9
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);

    /* Configure CAN RX and TX pins */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN configuration ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* CAN cell init */
    CAN_InitStruct.CAN_TTCM = DISABLE;
    CAN_InitStruct.CAN_ABOM = DISABLE;
    CAN_InitStruct.CAN_AWUM = DISABLE;
    CAN_InitStruct.CAN_NART = DISABLE;
    CAN_InitStruct.CAN_RFLM = DISABLE;
    CAN_InitStruct.CAN_TXFP = ENABLE;
    CAN_InitStruct.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStruct.CAN_SJW = CAN_SJW_1tq;

    /* CAN Baudrate = 125 KBps
     * 1/(prescalar + (prescalar*BS1) + (prescalar*BS2)) * Clk = CAN Baudrate
     * The CAN clk is currently set to 20MHz (APB1 clock set to 20MHz in BSP_PLL_Init())
     */
    CAN_InitStruct.CAN_BS1 = CAN_BS1_3tq;
    CAN_InitStruct.CAN_BS2 = CAN_BS2_4tq;
    CAN_InitStruct.CAN_Prescaler = 16;
    CAN_Init(CAN1, &CAN_InitStruct);

    /* CAN filter init 
     * Initializes hardware filter banks to be used for filtering CAN IDs (whitelist)
     */
    if(idWhitelist == NULL){
        // No filtering. All IDs can pass through.
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
    } else{
        // Filter CAN IDs
        //So far, if we shift whatever id we need by 5, it works

        // // DEBUG FOR FILTERING
        // // Added in to attempt to debug hardware filtering
        // CAN_FilterInitStruct.CAN_FilterNumber = 0;
        // CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList;
        // CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_16bit;
        // CAN_FilterInitStruct.CAN_FilterIdHigh = 268 << 5;
        // CAN_FilterInitStruct.CAN_FilterIdLow = 2 << 5;
        // CAN_FilterInitStruct.CAN_FilterMaskIdHigh = 262 << 5;
        // CAN_FilterInitStruct.CAN_FilterMaskIdLow = 267 << 5;
        // CAN_FilterInitStruct.CAN_FilterFIFOAssignment = 0;
        // CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
        // CAN_FilterInit(CAN1, &CAN_FilterInitStruct);

        CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList; //list mode
        CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_16bit;
        CAN_FilterInitStruct.CAN_FilterFIFOAssignment = 0;
        CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
        
        uint16_t* FilterStructPtr = (uint16_t*)&(CAN_FilterInitStruct); //address of CAN Filter Struct
        for(uint8_t i = 0; i < idWhitelistSize; i++){
            if (idWhitelist[i] == 0){ //zero ID check
                continue;
            }
                
            CAN_FilterInitStruct.CAN_FilterNumber = i / NUM_FILTER_REGS; //determines filter number based on CAN ID
            *(FilterStructPtr + (i%NUM_FILTER_REGS)) = idWhitelist[i] << 5;

            if(i % NUM_FILTER_REGS == NUM_FILTER_REGS - 1){ //if four elements have been written to a filter call CAN_FilterInit()
                CAN_FilterInit(CAN1, &CAN_FilterInitStruct);
            }
            else if(i == idWhitelistSize - 1){ //we are out of elements, call CAN_FilterInit()
                for(uint8_t j = i%NUM_FILTER_REGS + 1; j <= NUM_FILTER_REGS - 1; j++)   // Set unfilled filter registers to 0
                    *(FilterStructPtr + j) = 0x0000;

                CAN_FilterInit(CAN1, &CAN_FilterInitStruct);
            }
        }
    }

    /* Transmit Structure preparation */
    gTxMessage[0].ExtId = 0x5;
    gTxMessage[0].RTR = CAN_RTR_DATA;
    gTxMessage[0].IDE = CAN_ID_STD;
    gTxMessage[0].DLC = 1;

    /* Receive Structure preparation */
    gRxMessage[0].StdId = 0x00;
    gRxMessage[0].ExtId = 0x00;
    gRxMessage[0].IDE = CAN_ID_STD;
    gRxMessage[0].DLC = 0;
    gRxMessage[0].FMI = 0;

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    // Enable Rx interrupts
    NVIC_InitStruct.NVIC_IRQChannel = CAN1_RX0_IRQn; // TODO: CHECK IRQ CHANNELS
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    if(NULL != gTxEnd[0]) {
        // Enable Tx Interrupts
        CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);
        NVIC_InitStruct.NVIC_IRQChannel = CAN1_TX_IRQn; 
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0; // TODO: assess both of these priority settings
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
}

void BSP_CAN3_Init(uint16_t* idWhitelist, uint8_t idWhitelistSize)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_InitTypeDef CAN_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    CAN_FilterInitTypeDef CAN_FilterInitStruct;

    // Initialize the queue
    gRxQueue[1] = msg_queue_new();

    /* CAN GPIOs configuration **************************************************/

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Alternate Function 9
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF11_CAN3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF11_CAN3);

    /* Configure CAN RX and TX pins */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN configuration ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN3, ENABLE);

    /* CAN cell init */
    CAN_InitStruct.CAN_TTCM = DISABLE;
    CAN_InitStruct.CAN_ABOM = DISABLE;
    CAN_InitStruct.CAN_AWUM = DISABLE;
    CAN_InitStruct.CAN_NART = DISABLE;
    CAN_InitStruct.CAN_RFLM = DISABLE;
    CAN_InitStruct.CAN_TXFP = ENABLE;
    CAN_InitStruct.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStruct.CAN_SJW = CAN_SJW_1tq;

    /* CAN Baudrate = 125 KBps
     * 1/(prescalar + (prescalar*BS1) + (prescalar*BS2)) * Clk = CAN Baudrate
     * The CAN clk is currently set to 20MHz (APB1 clock set to 20MHz in BSP_PLL_Init())
     */
    CAN_InitStruct.CAN_BS1 = CAN_BS1_3tq;
    CAN_InitStruct.CAN_BS2 = CAN_BS2_4tq;
    CAN_InitStruct.CAN_Prescaler = 16;
    CAN_Init(CAN3, &CAN_InitStruct);

    /* CAN filter init 
     * Initializes hardware filter banks to be used for filtering CAN IDs (whitelist)
     */
    if(idWhitelist == NULL){
        // No filtering. All IDs can pass through.
        CAN_FilterInitStruct.CAN_FilterNumber = 0;
        CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdMask;
        CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
        CAN_FilterInitStruct.CAN_FilterIdHigh = 0x0000;
        CAN_FilterInitStruct.CAN_FilterIdLow = 0x0000;
        CAN_FilterInitStruct.CAN_FilterMaskIdHigh = 0x0000;
        CAN_FilterInitStruct.CAN_FilterMaskIdLow = 0x0000;
        CAN_FilterInitStruct.CAN_FilterFIFOAssignment = 0;
        CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
        CAN_FilterInit(CAN3, &CAN_FilterInitStruct);
    } else{
        // Filter CAN IDs
        CAN_FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList; //list mode
        CAN_FilterInitStruct.CAN_FilterScale = CAN_FilterScale_16bit;
        CAN_FilterInitStruct.CAN_FilterFIFOAssignment = 0;
        CAN_FilterInitStruct.CAN_FilterActivation = ENABLE;
        
        uint16_t* FilterStructPtr = (uint16_t*)&(CAN_FilterInitStruct); //address of CAN Filter Struct
        for(uint8_t i = 0; i < idWhitelistSize; i++){
            CAN_FilterInitStruct.CAN_FilterNumber = i / NUM_FILTER_REGS; //determines filter number based on CAN ID
            *(FilterStructPtr + (i%NUM_FILTER_REGS)) = idWhitelist[i];

            if(i % NUM_FILTER_REGS == NUM_FILTER_REGS - 1){ //if four elements have been written to a filter call CAN_FilterInit()
                CAN_FilterInit(CAN3, &CAN_FilterInitStruct);
            }
            else if(i == idWhitelistSize - 1){ //we are out of elements, call CAN_FilterInit()
                for(uint8_t j = i%NUM_FILTER_REGS + 1; j <= NUM_FILTER_REGS - 1; j++)   // Set unfilled filter registers to 0
                    *(FilterStructPtr + j) = 0x0000;

                CAN_FilterInit(CAN3, &CAN_FilterInitStruct);
            }
        }
    }

    // CAN_SlaveStartBank(CAN1, 0);

    /* Transmit Structure preparation */
    gTxMessage[1].ExtId = 0x5;
    gTxMessage[1].RTR = CAN_RTR_DATA;
    gTxMessage[1].IDE = CAN_ID_STD;
    gTxMessage[1].DLC = 1;

    /* Receive Structure preparation */
    gRxMessage[1].StdId = 0x00;
    gRxMessage[1].ExtId = 0x00;
    gRxMessage[1].IDE = CAN_ID_STD;
    gRxMessage[1].DLC = 0;
    gRxMessage[1].FMI = 0;

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN3, CAN_IT_FMP0, ENABLE);

    //TODO: Double check preemption priority and subpriority
    // Enable Rx interrupts
    NVIC_InitStruct.NVIC_IRQChannel = CAN3_RX0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // Enable Tx interrupts
    if(NULL != gTxEnd[1]){ 
        CAN_ITConfig(CAN3,CAN_IT_TME,ENABLE);
        NVIC_InitStruct.NVIC_IRQChannel = CAN3_TX_IRQn; 
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00; 
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
}

/**
 * @brief   Transmits the data onto the CAN bus with the specified id
 * @param   id : Message of ID. Also indicates the priority of message. The lower the value, the higher the priority.
 * @param   data : data to be transmitted. The max is 8 bytes.
 * @param   length : num of bytes of data to be transmitted. This must be <= 8 bytes or else the rest of the message is dropped.
 * @return  ERROR if module was unable to transmit the data onto the CAN bus. SUCCESS indicates data was transmitted.
 */
ErrorStatus BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t data[8], uint8_t length)
{

    gTxMessage[bus].StdId = id;
    gTxMessage[bus].DLC = length;
    for (int i = 0; i < length; i++)
    {
        gTxMessage[bus].Data[i] = data[i];
    }

    uint8_t retVal = (CAN_Transmit(bus == CAN_1 ? CAN1 : CAN3, &gTxMessage[bus]) != 0);
    if (retVal == CAN_TxStatus_NoMailBox)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @brief   Gets the data that was received from the CAN bus.
 * @note    Non-blocking statement
 * @pre     The data parameter must be at least 8 bytes or hardfault may occur.
 * @param   id : pointer to store id of the message that was received.
 * @param   data : pointer to store data that was received. Must be 8bytes or bigger.
 * @return  ERROR if nothing was received so ignore id and data that was received. SUCCESS indicates data was received and stored.
 */
ErrorStatus BSP_CAN_Read(CAN_t bus, uint32_t *id, uint8_t *data)
{
    // If the queue is empty, return err
    if (msg_queue_is_empty(&gRxQueue[bus]))
    {
        return ERROR;
    }

    // Get the message
    msg_t msg;
    msg_queue_get(&gRxQueue[bus], &msg);

    // Transfer the message to the provided pointers
    for (int i = 0; i < 8; i++)
    {
        data[i] = msg.data[i];
    }
    *id = msg.id;

    return SUCCESS;
}

void CAN3_RX0_IRQHandler()
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Take any pending messages into a queue
    while (CAN_MessagePending(CAN3, CAN_FIFO0))
    {
        CAN_Receive(CAN3, CAN_FIFO0, &gRxMessage[1]);

        msg_t rxMsg;
        rxMsg.id = gRxMessage[1].StdId;
        memcpy(&rxMsg.data[0], gRxMessage[1].Data, 8);

        // Place the message in the queue
        if (msg_queue_put(&gRxQueue[1], rxMsg))
        {
            // If the queue was not already full...
            // Call the driver-provided function, if it is not null
            if (gRxEvent[1] != NULL)
            {
                gRxEvent[1]();
            }
        }
        else
        {
            // If the queue is already full, then we can't really do anything else
            break;
        }
    }

    OSIntExit(); // Signal to uC/OS
}

void CAN1_RX0_IRQHandler(void)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Take any pending messages into a queue
    while (CAN_MessagePending(CAN1, CAN_FIFO0))
    {
        CAN_Receive(CAN1, CAN_FIFO0, &gRxMessage[0]);

        msg_t rxMsg;
        rxMsg.id = gRxMessage[0].StdId;
        memcpy(&rxMsg.data[0], gRxMessage[0].Data, 8);

        // Place the message in the queue
        if (msg_queue_put(&gRxQueue[0], rxMsg))
        {
            // If the queue was not already full...
            // Call the driver-provided function, if it is not null
            if (gRxEvent[0] != NULL)
            {
                gRxEvent[0]();
            }
        }
        else
        {
            // If the queue is already full, then we can't really do anything else
            break;
        }
    }

    OSIntExit(); // Signal to uC/OS
}

void CAN3_TX_IRQHandler(void)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Acknowledge
    CAN_ClearFlag(CAN3, CAN_FLAG_RQCP0 | CAN_FLAG_RQCP1 | CAN_FLAG_RQCP2);

    // Call the function provided
    gTxEnd[1]();

    OSIntExit(); // Signal to uC/OS
}

void CAN1_TX_IRQHandler(void)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();
    // Call the function provided
    CAN_ClearFlag(CAN1, CAN_FLAG_RQCP0 | CAN_FLAG_RQCP1 | CAN_FLAG_RQCP2);
    gTxEnd[0]();

    OSIntExit(); // Signal to uC/OS
}
