/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_CAN.h"

#include "os.h"
#include "stm32f4xx.h"

#define NUM_CAN_LINES 2

// The message information that we care to receive
typedef struct Msg {
    uint32_t id;
    uint8_t data[BSP_CAN_DATA_LENGTH];
} Msg;

// Set up a fifo for receiving
#define FIFO_TYPE Msg
#define FIFO_SIZE 25
#define FIFO_NAME MsgQueue
#include "fifo.h"

#define NUM_FILTER_REGS \
    4  // Number of 16 bit registers for ids in one CAN_FilterInit struct

// return error if someone tries to call from motor can

static MsgQueue g_rx_queue[NUM_CAN_LINES];

// Required for receiving CAN messages
static CanTxMsg g_tx_message[NUM_CAN_LINES];
static CanRxMsg g_rx_message[NUM_CAN_LINES];

// User parameters for CAN events
static Callback g_rx_event[NUM_CAN_LINES];
static Callback g_tx_end[NUM_CAN_LINES];

void BspCan1Init(const uint16_t* id_whitelist, uint8_t id_whitelist_size);
void BspCan3Init(const uint16_t* id_whitelist, uint8_t id_whitelist_size);

/**
 * @brief   Initializes the CAN module that communicates with the rest of the
 * electrical system.
 * @param   rxEvent : the function to execute when recieving a message. NULL for
 * no action.
 * @param   txEnd   : the function to execute after transmitting a message. NULL
 * for no action.
 * @return  None
 */

void BspCanInit(Can bus, Callback rx_event, Callback tx_end,
                uint16_t* id_whitelist, uint8_t id_whitelist_size) {
    // Configure event handles
    g_rx_event[bus] = rx_event;
    g_tx_end[bus] = tx_end;

    if (bus == kCan1) {
        BspCan1Init(id_whitelist, id_whitelist_size);
    } else {
        BspCan3Init(id_whitelist, id_whitelist_size);
    }
}

void BspCan1Init(const uint16_t* id_whitelist, uint8_t id_whitelist_size) {
    GPIO_InitTypeDef gpio_init_struct;
    CAN_InitTypeDef can_init_struct;
    NVIC_InitTypeDef nvic_init_struct;
    CAN_FilterInitTypeDef can_filter_init_struct;

    // Initialize the queue
    g_rx_queue[0] = MsgQueueNew();

    /* CAN GPIOs configuration
     * **************************************************/

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Alternate Function 9
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);

    /* Configure CAN RX and TX pins */
    gpio_init_struct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio_init_struct);

    /* CAN configuration
     * ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* CAN cell init */
    can_init_struct.CAN_TTCM = DISABLE;
    can_init_struct.CAN_ABOM = DISABLE;
    can_init_struct.CAN_AWUM = DISABLE;
    can_init_struct.CAN_NART = DISABLE;
    can_init_struct.CAN_RFLM = DISABLE;
    can_init_struct.CAN_TXFP = ENABLE;
#ifdef CAR_LOOPBACK
    CAN_InitStruct.CAN_Mode = CAN_Mode_LoopBack;
#else
    can_init_struct.CAN_Mode = CAN_Mode_Normal;
#endif
    can_init_struct.CAN_SJW = CAN_SJW_1tq;

    /* CAN Baudrate = 125 KBps
     * 1/(prescalar + (prescalar*BS1) + (prescalar*BS2)) * Clk = CAN Baudrate
     * The CAN clk is currently set to 20MHz (APB1 clock set to 20MHz in
     * BSP_PLL_Init())
     */
    can_init_struct.CAN_BS1 = CAN_BS1_3tq;
    can_init_struct.CAN_BS2 = CAN_BS2_4tq;
    can_init_struct.CAN_Prescaler = 16;  // NOLINT
    CAN_Init(CAN1, &can_init_struct);

    /* CAN filter init
     * Initializes hardware filter banks to be used for filtering CAN IDs
     * (whitelist)
     */
    if (id_whitelist == NULL) {
        // No filtering. All IDs can pass through.
        can_filter_init_struct.CAN_FilterNumber = 0;
        can_filter_init_struct.CAN_FilterMode = CAN_FilterMode_IdMask;
        can_filter_init_struct.CAN_FilterScale = CAN_FilterScale_32bit;
        can_filter_init_struct.CAN_FilterIdHigh = 0x0000;
        can_filter_init_struct.CAN_FilterIdLow = 0x0000;
        can_filter_init_struct.CAN_FilterMaskIdHigh = 0x0000;
        can_filter_init_struct.CAN_FilterMaskIdLow = 0x0000;
        can_filter_init_struct.CAN_FilterFIFOAssignment = 0;
        can_filter_init_struct.CAN_FilterActivation = ENABLE;
        CAN_FilterInit(CAN1, &can_filter_init_struct);
    } else {
        // Filter CAN IDs
        // So far, if we shift whatever id we need by 5, it works
        // If whitelist is passed but no valid ID exists inside of it, filter
        // nothing i.e. no ID gets through MAXIMUM CAN ID ALLOWED IS 2047

        can_filter_init_struct.CAN_FilterMode =
            CAN_FilterMode_IdList;  // list mode
        can_filter_init_struct.CAN_FilterScale = CAN_FilterScale_16bit;
        can_filter_init_struct.CAN_FilterFIFOAssignment = 0;
        can_filter_init_struct.CAN_FilterActivation = ENABLE;
        uint16_t valid_id_counter = 0;

        uint16_t* filter_struct_ptr = (uint16_t*)&(
            can_filter_init_struct);  // address of CAN Filter Struct
        for (uint8_t i = 0; i < id_whitelist_size; i++) {
            if (id_whitelist[i] == 0) {  // zero ID check
                continue;
            }

            can_filter_init_struct.CAN_FilterNumber =
                i /
                NUM_FILTER_REGS;  // determines filter number based on CAN ID
            *(filter_struct_ptr + (i % NUM_FILTER_REGS)) = id_whitelist[i]
                                                           << 5;  // NOLINT
            valid_id_counter++;

            if (i % NUM_FILTER_REGS ==
                NUM_FILTER_REGS - 1) {  // if four elements have been written to
                                        // a filter call CAN_FilterInit()
                CAN_FilterInit(CAN1, &can_filter_init_struct);
            } else if (i == id_whitelist_size - 1) {  // we are out of elements,
                                                      // call CAN_FilterInit()
                for (uint8_t j = i % NUM_FILTER_REGS + 1;
                     j <= NUM_FILTER_REGS - 1;
                     j++) {  // Set unfilled filter registers to 0
                    *(filter_struct_ptr + j) = 0x0000;
                }

                CAN_FilterInit(CAN1, &can_filter_init_struct);
            } else if (valid_id_counter >
                       112) {  // NOLINT
                               // All filter banks are to be filled and there is
                               // no point in filtering
                for (uint8_t filter = 0; filter < 28;  // NOLINT
                     filter++) {  // Therefore, let all IDs through (no
                                  // filtering)
                    can_filter_init_struct.CAN_FilterNumber = filter;
                    can_filter_init_struct.CAN_FilterActivation = DISABLE;
                    CAN_FilterInit(CAN1, &can_filter_init_struct);
                }
            }
        }
    }

    /* Transmit Structure preparation */
    g_tx_message[0].ExtId = 0x5;  // NOLINT
    g_tx_message[0].RTR = CAN_RTR_DATA;
    g_tx_message[0].IDE = CAN_ID_STD;
    g_tx_message[0].DLC = 1;

    /* Receive Structure preparation */
    g_rx_message[0].StdId = 0x00;
    g_rx_message[0].ExtId = 0x00;
    g_rx_message[0].IDE = CAN_ID_STD;
    g_rx_message[0].DLC = 0;
    g_rx_message[0].FMI = 0;

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    // Enable Rx interrupts
    nvic_init_struct.NVIC_IRQChannel =
        CAN1_RX0_IRQn;  // TODO: CHECK IRQ CHANNELS
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);

    if (NULL != g_tx_end[0]) {
        // Enable Tx Interrupts
        CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);
        nvic_init_struct.NVIC_IRQChannel = CAN1_TX_IRQn;
        nvic_init_struct.NVIC_IRQChannelPreemptionPriority =
            0x0;  // TODO: assess both of these priority settings
        nvic_init_struct.NVIC_IRQChannelSubPriority = 0x0;
        nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvic_init_struct);
    }
}

void BspCan3Init(const uint16_t* id_whitelist, uint8_t id_whitelist_size) {
    GPIO_InitTypeDef gpio_init_struct;
    CAN_InitTypeDef can_init_struct;
    NVIC_InitTypeDef nvic_init_struct;
    CAN_FilterInitTypeDef can_filter_init_struct;

    // Initialize the queue
    g_rx_queue[1] = MsgQueueNew();

    /* CAN GPIOs configuration
     * **************************************************/

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Alternate Function 9
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF11_CAN3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF11_CAN3);

    /* Configure CAN RX and TX pins */
    gpio_init_struct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_8;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio_init_struct);

    /* CAN configuration
     * ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN3, ENABLE);

    /* CAN cell init */
    can_init_struct.CAN_TTCM = DISABLE;
    can_init_struct.CAN_ABOM = DISABLE;
    can_init_struct.CAN_AWUM = DISABLE;
    can_init_struct.CAN_NART = DISABLE;
    can_init_struct.CAN_RFLM = DISABLE;
    can_init_struct.CAN_TXFP = ENABLE;
#ifdef MOTOR_LOOPBACK
    CAN_InitStruct.CAN_Mode = CAN_Mode_LoopBack;
#else
    can_init_struct.CAN_Mode = CAN_Mode_Normal;
#endif
    can_init_struct.CAN_SJW = CAN_SJW_1tq;

    /* CAN Baudrate = 125 KBps
     * 1/(prescalar + (prescalar*BS1) + (prescalar*BS2)) * Clk = CAN Baudrate
     * The CAN clk is currently set to 20MHz (APB1 clock set to 20MHz in
     * BSP_PLL_Init())
     */
    can_init_struct.CAN_BS1 = CAN_BS1_3tq;
    can_init_struct.CAN_BS2 = CAN_BS2_4tq;
    can_init_struct.CAN_Prescaler = 16;  // NOLINT
    CAN_Init(CAN3, &can_init_struct);

    /* CAN filter init
     * Initializes hardware filter banks to be used for filtering CAN IDs
     * (whitelist)
     */
    if (id_whitelist == NULL) {
        // No filtering. All IDs can pass through.
        can_filter_init_struct.CAN_FilterNumber = 0;
        can_filter_init_struct.CAN_FilterMode = CAN_FilterMode_IdMask;
        can_filter_init_struct.CAN_FilterScale = CAN_FilterScale_32bit;
        can_filter_init_struct.CAN_FilterIdHigh = 0x0000;
        can_filter_init_struct.CAN_FilterIdLow = 0x0000;
        can_filter_init_struct.CAN_FilterMaskIdHigh = 0x0000;
        can_filter_init_struct.CAN_FilterMaskIdLow = 0x0000;
        can_filter_init_struct.CAN_FilterFIFOAssignment = 0;
        can_filter_init_struct.CAN_FilterActivation = ENABLE;
        CAN_FilterInit(CAN3, &can_filter_init_struct);
    } else {
        // Filter CAN IDs
        can_filter_init_struct.CAN_FilterMode =
            CAN_FilterMode_IdList;  // list mode
        can_filter_init_struct.CAN_FilterScale = CAN_FilterScale_16bit;
        can_filter_init_struct.CAN_FilterFIFOAssignment = 0;
        can_filter_init_struct.CAN_FilterActivation = ENABLE;

        uint16_t* filter_struct_ptr = (uint16_t*)&(
            can_filter_init_struct);  // address of CAN Filter Struct
        for (uint8_t i = 0; i < id_whitelist_size; i++) {
            can_filter_init_struct.CAN_FilterNumber =
                i /
                NUM_FILTER_REGS;  // determines filter number based on CAN ID
            *(filter_struct_ptr + (i % NUM_FILTER_REGS)) = id_whitelist[i];

            if (i % NUM_FILTER_REGS ==
                NUM_FILTER_REGS - 1) {  // if four elements have been written to
                                        // a filter call CAN_FilterInit()
                CAN_FilterInit(CAN3, &can_filter_init_struct);
            } else if (i == id_whitelist_size - 1) {  // we are out of elements,
                                                      // call CAN_FilterInit()
                for (uint8_t j = i % NUM_FILTER_REGS + 1;
                     j <= NUM_FILTER_REGS - 1;
                     j++) {  // Set unfilled filter registers to 0
                    *(filter_struct_ptr + j) = 0x0000;
                }

                CAN_FilterInit(CAN3, &can_filter_init_struct);
            }
        }
    }

    // CAN_SlaveStartBank(CAN1, 0);

    /* Transmit Structure preparation */
    g_tx_message[1].ExtId = 0x5;  // NOLINT
    g_tx_message[1].RTR = CAN_RTR_DATA;
    g_tx_message[1].IDE = CAN_ID_STD;
    g_tx_message[1].DLC = 1;

    /* Receive Structure preparation */
    g_rx_message[1].StdId = 0x00;
    g_rx_message[1].ExtId = 0x00;
    g_rx_message[1].IDE = CAN_ID_STD;
    g_rx_message[1].DLC = 0;
    g_rx_message[1].FMI = 0;

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN3, CAN_IT_FMP0, ENABLE);

    // TODO: Double check preemption priority and subpriority
    //  Enable Rx interrupts
    nvic_init_struct.NVIC_IRQChannel = CAN3_RX0_IRQn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);

    // Enable Tx interrupts
    if (NULL != g_tx_end[1]) {
        CAN_ITConfig(CAN3, CAN_IT_TME, ENABLE);
        nvic_init_struct.NVIC_IRQChannel = CAN3_TX_IRQn;
        nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x00;
        nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
        nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvic_init_struct);
    }
}

/**
 * @brief   Transmits the data onto the CAN bus with the specified id
 * @param   id : Message of ID. Also indicates the priority of message. The
 * lower the value, the higher the priority.
 * @param   data : data to be transmitted. The max is 8 bytes.
 * @param   length : num of bytes of data to be transmitted. This must be <= 8
 * bytes or else the rest of the message is dropped.
 * @return  ERROR if module was unable to transmit the data onto the CAN bus.
 * SUCCESS indicates data was transmitted.
 */
ErrorStatus BspCanWrite(Can bus, uint32_t id,
                        const uint8_t data[BSP_CAN_DATA_LENGTH],
                        uint8_t length) {
    memset(g_tx_message[bus].Data, 0, sizeof g_tx_message[bus].Data);
    g_tx_message[bus].StdId = id;
    g_tx_message[bus].DLC = length;
    for (int i = 0; i < length; i++) {
        g_tx_message[bus].Data[i] = data[i];
    }

    uint8_t ret_val =
        (CAN_Transmit(bus == kCan1 ? CAN1 : CAN3, &g_tx_message[bus]) != 0);
    if (ret_val == CAN_TxStatus_NoMailBox) {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @brief   Gets the data that was received from the CAN bus.
 * @note    Non-blocking statement
 * @pre     The data parameter must be at least 8 bytes or hardfault may occur.
 * @param   id : pointer to store id of the message that was received.
 * @param   data : pointer to store data that was received. Must be 8bytes or
 * bigger.
 * @return  ERROR if nothing was received so ignore id and data that was
 * received. SUCCESS indicates data was received and stored.
 */
ErrorStatus BspCanRead(Can bus, uint32_t* id, uint8_t* data) {
    // If the queue is empty, return err
    if (MsgQueueIsEmpty(&g_rx_queue[bus])) {
        return ERROR;
    }

    // Get the message
    Msg msg;
    MsgQueueGet(&g_rx_queue[bus], &msg);

    // Transfer the message to the provided pointers
    for (int i = 0; i < BSP_CAN_DATA_LENGTH; i++) {  // NOLINT
        data[i] = msg.data[i];
    }
    *id = msg.id;

    return SUCCESS;
}

void Can3RX0IrqHandler() {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Take any pending messages into a queue
    while (CAN_MessagePending(CAN3, CAN_FIFO0)) {
        CAN_Receive(CAN3, CAN_FIFO0, &g_rx_message[1]);

        Msg rx_msg;
        rx_msg.id = g_rx_message[1].StdId;
        memcpy(&rx_msg.data[0], g_rx_message[1].Data, BSP_CAN_DATA_LENGTH);

        // Place the message in the queue
        if (MsgQueuePut(&g_rx_queue[1], rx_msg)) {
            // If the queue was not already full...
            // Call the driver-provided function, if it is not null
            if (g_rx_event[1] != NULL) {
                g_rx_event[1]();
            }
        } else {
            // If the queue is already full, then we can't really do anything
            // else
            break;
        }
    }

    OSIntExit();  // Signal to uC/OS
}

void Can1RX0IrqHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Take any pending messages into a queue
    while (CAN_MessagePending(CAN1, CAN_FIFO0)) {
        CAN_Receive(CAN1, CAN_FIFO0, &g_rx_message[0]);

        Msg rx_msg;
        rx_msg.id = g_rx_message[0].StdId;
        memcpy(&rx_msg.data[0], g_rx_message[0].Data, BSP_CAN_DATA_LENGTH);

        // Place the message in the queue
        if (MsgQueuePut(&g_rx_queue[0], rx_msg)) {
            // If the queue was not already full...
            // Call the driver-provided function, if it is not null
            if (g_rx_event[0] != NULL) {
                g_rx_event[0]();
            }
        } else {
            // If the queue is already full, then we can't really do anything
            // else
            break;
        }
    }

    OSIntExit();  // Signal to uC/OS
}

void Can3TxIrqHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // Acknowledge
    CAN_ClearFlag(CAN3, CAN_FLAG_RQCP0 | CAN_FLAG_RQCP1 | CAN_FLAG_RQCP2);

    // Call the function provided
    g_tx_end[1]();

    OSIntExit();  // Signal to uC/OS
}

void Can1TxIrqHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();
    // Call the function provided
    CAN_ClearFlag(CAN1, CAN_FLAG_RQCP0 | CAN_FLAG_RQCP1 | CAN_FLAG_RQCP2);
    g_tx_end[0]();

    OSIntExit();  // Signal to uC/OS
}
