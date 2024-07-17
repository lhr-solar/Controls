/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_UART.h"
#include "stm32f4xx.h"
#include "os.h"

#define TX_SIZE     128
#define RX_SIZE     64

// Initialize the FIFOs

#define FFIFO_TYPE char
#define FFIFO_SIZE TX_SIZE
#define FFIFO_NAME txfifo
#include "fastfifo.h"
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
static USART_TypeDef *handles[NUM_UART] = {USART2, USART3};

static void USART_DISPLAY_Init() {
    txfifo_renew(&displayTxFifo);
    displayRxFifo = rxfifo_new();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    // Initialize clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,  ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,  ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // Initialize pins
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_USART3);

    //Initialize UART3
    UART_InitStruct.USART_BaudRate = 115200;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &UART_InitStruct);

    // Enable interrupts

    USART_Cmd(USART3, ENABLE);

    // Enable NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void USART_USB_Init() {
    txfifo_renew(&usbTxFifo);
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
    case UART_2: // their UART_USB
        USART_USB_Init();
        usbRxCallback = rxCallback;
        usbTxCallback = txCallback;
        break;
    case UART_3: // their UART_DISPLAY
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

    txfifo_t *fifo = tx_fifos[usart];
    
    // wait for enough space to send
    volatile uint32_t space;    // volatile as ISR pulls from fifo
    do {
        // nasty hack to make sure fifo len is up to date
        space = TX_SIZE - *((volatile int *)&(fifo->len));
    } while (len > space);

    // add to fifo and enable interrupt -- disable first to prevent race
    USART_ITConfig(usart_handle, USART_IT_TXE, RESET);
    if (USART_GetITStatus(usart_handle, USART_IT_TXE) != RESET) {
        while (1);
    }
    txfifo_put(fifo, str, len);
    USART_ITConfig(usart_handle, USART_IT_TXE, SET);

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

            USART_SendData(UART_2, 0x7F);   // TODO: Not sure if the backspace works. Need to test
        }
        if(removeSuccess) {
            USART2->DR = data;
        }
    }
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!txfifo_get(&usbTxFifo, (char*)&(USART2->DR), 1)) {
            USART_ITConfig(USART2, USART_IT_TXE, RESET);
            if(usbTxCallback != NULL)
                usbTxCallback();
        }
    }
    if(USART_GetITStatus(USART2, USART_IT_ORE) != RESET);


    OSIntExit();

}

void USART3_IRQHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t data = USART3->DR;
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
            USART3->DR = data;
        }
    }
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!txfifo_get(&usbTxFifo, (char*)&(USART3->DR), 1)) {
            USART_ITConfig(USART3, USART_IT_TXE, RESET);
            if(displayTxCallback != NULL)
                displayTxCallback();
        }
    }
    if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET);

    OSIntExit();
}
