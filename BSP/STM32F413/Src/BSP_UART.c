/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_UART.h"
#include "stm32f4xx.h"
#include "os.h"

#define TX_SIZE     128
#define RX_SIZE     64

// Initialize the FIFOs

#define FIFO_TYPE char
#define FIFO_SIZE TX_SIZE
#define FIFO_NAME txfifo
#include "fifo.h"
static txfifo_t usbTxFifo;
static txfifo_t displayTxFifo;

#define FIFO_TYPE char
#define FIFO_SIZE RX_SIZE
#define FIFO_NAME rxfifo
#include "fifo.h"
static rxfifo_t usbRxFifo;
static rxfifo_t displayRxFifo;

static bool usbLineReceived = false;
static bool displayLineReceived = false;

static callback_t usbRxCallback = NULL;
static callback_t usbTxCallback = NULL;
static callback_t displayRxCallback = NULL;
static callback_t displayTxCallback = NULL;

static rxfifo_t *rx_fifos[NUM_UART]     = {&usbRxFifo, &displayRxFifo};
static txfifo_t *tx_fifos[NUM_UART]     = {&usbTxFifo, &displayTxFifo};
static bool     *lineRecvd[NUM_UART]    = {&usbLineReceived, &displayLineReceived};
static USART_TypeDef *handles[NUM_UART] = {USART2, UART4};

// Originally, used USART3, now uses UART4 for Daybreak (hopefully display doesn't need USART)
static void USART_DISPLAY_Init() {
    displayTxFifo = txfifo_new();
    displayRxFifo = rxfifo_new();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    // Initialize clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);

    // Initialize pins
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF11_UART4);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF11_UART4);

    //Initialize UART4
    UART_InitStruct.USART_BaudRate = 115200;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(UART4, &UART_InitStruct);

    // Enable interrupts

    USART_Cmd(UART4, ENABLE);

    // Enable NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void USART_USB_Init() {
    usbTxFifo = txfifo_new();
    usbRxFifo = rxfifo_new();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    // Initialize clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);

    // Initialize pins
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    //Initialize UART2
    UART_InitStruct.USART_BaudRate = 115200;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &UART_InitStruct);

    USART_Cmd(USART2, ENABLE);

    // Enable NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    setvbuf(stdout, NULL, _IONBF, 0);
}

/**
 * @brief   Initializes the UART peripheral
 */
static void BSP_UART_Init_Internal(callback_t rxCallback, callback_t txCallback, UART_t uart) {
    switch(uart){
    case USB: // their UART_USB
        USART_USB_Init();
        usbRxCallback = rxCallback;
        usbTxCallback = txCallback;
        break;
    case DISPLAY: // their UART_DISPLAY
        USART_DISPLAY_Init();
        displayRxCallback = rxCallback;
        displayTxCallback = txCallback;
        break;
    default:
        // Error
        break;
    }
}

void BSP_UART_Init(UART_t uart) {
    // Not using callbacks for now
    BSP_UART_Init_Internal(NULL, NULL, uart);
}

/**
 * @brief   Gets one line of ASCII text that was received. The '\n' and '\r' characters will not be stored (tested on Putty on Windows)
 * @pre     str should be at least 128bytes long.
 * @param   str : pointer to buffer to store the string. This buffer should be initialized
 *                  before hand.
 * @param   usart : which usart to read from (2 or 3)
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(UART_t usart, char *str) {
    char data = 0;
    uint32_t recvd = 0;

    bool *line_recvd = lineRecvd[usart];
    
    while(*line_recvd == false){
        BSP_UART_Write(usart, "", 0);   // needs to be in. Otherwise, usbLineReceived will not update
    }

    USART_TypeDef *usart_handle = handles[usart];

    USART_ITConfig(usart_handle, USART_IT_RXNE, RESET);
    
    rxfifo_t *fifo = rx_fifos[usart];
    
    rxfifo_peek(fifo, &data);
    while(!rxfifo_is_empty(fifo) && data != '\r') {
        recvd += rxfifo_get(fifo, (char*)str++);
        rxfifo_peek(fifo, &data);
    }

    rxfifo_get(fifo, &data);
    *str = 0;
    *line_recvd = false;
    USART_ITConfig(usart_handle, USART_IT_RXNE, SET);

    return recvd;
}

/**
 * @brief   Transmits data to through UART line
 * @param   str : pointer to buffer with data to send.
 * @param   len : size of buffer
 * @param   usart : which usart to read from (2 or 3)
 * @return  number of bytes that were sent
 * 
 * @note This function uses a fifo to buffer the write. If that
 *       fifo is full, this function may block while waiting for
 *       space to open up. Do not call from timing-critical
 *       sections of code.
 */
uint32_t BSP_UART_Write(UART_t usart, char *str, uint32_t len) {
    uint32_t sent = 0;

    USART_TypeDef *usart_handle = handles[usart];

    USART_ITConfig(usart_handle, USART_IT_TC, RESET);

    txfifo_t *fifo = tx_fifos[usart];

    while(sent < len) {
        if(!txfifo_put(fifo, str[sent])) {
            // Allow the interrupt to fire
            USART_ITConfig(usart_handle, USART_IT_TC, SET);
            // Wait for space to open up
            while(txfifo_is_full(fifo));
            // Disable the interrupt again
            USART_ITConfig(usart_handle, USART_IT_TC, RESET);
        } else {
            sent++;  
        }
    }

    USART_ITConfig(usart_handle, USART_IT_TC, SET);

    return sent;
}

void USART2_IRQHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
        uint8_t data = USART2->DR;
        bool removeSuccess = 1;
        if(data == '\r' || data == '\n'){
            usbLineReceived = true;
            if(usbRxCallback != NULL)
                usbRxCallback();
        }
        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        else if(data != '\b' && data != '\177') {
            rxfifo_put(&usbRxFifo, data);
        }
        // Sweet, just a "regular" key. Put it into the fifo
        // Doesn't matter if it fails. If it fails, then the data gets thrown away
        // and the easiest solution for this is to increase RX_SIZE
        else {
            char junk;
            // Delete the last entry!
            removeSuccess = rxfifo_popback(&usbRxFifo, &junk); 

            USART_SendData(USB, 0x7F);   // TODO: Not sure if the backspace works. Need to test
        }
        if(removeSuccess) {
            USART2->DR = data;
        }
    }
    if(USART_GetITStatus(USART2, USART_IT_TC) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!txfifo_get(&usbTxFifo, (char*)&(USART2->DR))) {
            USART_ITConfig(USART2, USART_IT_TC, RESET); // Turn off the interrupt
            if(usbTxCallback != NULL)
                usbTxCallback();    // Callback
        }
    }
    if(USART_GetITStatus(USART2, USART_IT_ORE) != RESET);


    OSIntExit();

}


// (TODO) This should be UART4 IRQ Handler???? IDK BSP HELPPPPP
// This was USART3_IRQHandler... I tried some stuff, likely to be wrong
void UART4_IRQHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) {
        uint8_t data = UART4->DR;
        bool removeSuccess = 1;
        if(data == '\r'){
            displayLineReceived = true;
            if(displayRxCallback != NULL)
                displayRxCallback();
        }
        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        if(data != '\b' && data != '\177') rxfifo_put(&displayRxFifo, data);
        // Sweet, just a "regular" key. Put it into the fifo
        // Doesn't matter if it fails. If it fails, then the data gets thrown away
        // and the easiest solution for this is to increase RX_SIZE
        else {
            char junk;
            // Delete the last entry!
            removeSuccess = rxfifo_popback(&displayRxFifo, &junk);
        }
        if(removeSuccess) {
            UART4->DR = data;
        }
    }
    if(USART_GetITStatus(UART4, USART_IT_TC) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!txfifo_get(&displayTxFifo, (char*)&(UART4->DR))) {
            USART_ITConfig(UART4, USART_IT_TC, RESET);
            if(displayTxCallback != NULL)
                displayTxCallback();
        }
    }
    if(USART_GetITStatus(UART4, USART_IT_ORE) != RESET);

    OSIntExit();
}
