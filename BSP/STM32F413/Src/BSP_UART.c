/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_UART.h"
#include "stm32f4xx.h"

#define TX_SIZE     128
#define RX_SIZE     64

/* Combined Tx/Rx Fifo structure */
typedef struct UartFifo {
    char txBuffer[TX_SIZE];
    uint32_t txPut;
    uint32_t txGet;

    char rxBuffer[RX_SIZE];
    uint32_t rxPut;
    uint32_t rxGet;
    bool lineReceived;
} UartFifo_t;

// Statically declare the fifos for any UARTs we may use.
static UartFifo_t fifos[NUM_UART];

static void Fifo_Init(UartFifo_t *fifo);

static bool TxFifo_Get(UartFifo_t *fifo, uint8_t *data);
static bool TxFifo_Put(UartFifo_t *fifo, uint8_t data);
static bool TxFifo_IsFull(UartFifo_t *fifo);
static bool TxFifo_IsEmpty(UartFifo_t *fifo);

static bool RxFifo_Get(UartFifo_t *fifo, uint8_t *data);
static bool RxFifo_Put(UartFifo_t *fifo, uint8_t data);
static bool RxFifo_RemoveLast(UartFifo_t *fifo, uint8_t *data);
static bool RxFifo_Peek(UartFifo_t *fifo, uint8_t *data);
static bool RxFifo_IsFull(UartFifo_t *fifo);
static bool RxFifo_IsEmpty(UartFifo_t *fifo);

static void getUSARTReferences(UART_t uartSelect, USART_TypeDef **uart, UartFifo_t **fifo) {
    *fifo = &fifos[uartSelect];

    if(UART_2 == uartSelect) {
        *uart = USART2;
    } else if(UART_3 == uartSelect) {
        *uart = USART3;
    }
}

/**
 * @brief   Initializes the UART peripheral
 */
static void BSP_UART2_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    Fifo_Init(&fifos[UART_2]);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART2);

    UART_InitStruct.USART_BaudRate = 9600;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &UART_InitStruct);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);

    USART_Cmd(USART2, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    setvbuf(stdout, NULL, _IONBF, 0);
}

#define NUCLEO_TARGET
#ifdef NUCLEO_TARGET
/**
 * @brief   Initializes the UART peripheral
 */
static void BSP_UART3_Init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    Fifo_Init(&fifos[UART_3]);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    UART_InitStruct.USART_BaudRate = 115200;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &UART_InitStruct);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);

    USART_Cmd(USART3, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    setvbuf(stdout, NULL, _IONBF, 0);
}
#else
/**
 * @brief   Initializes the UART peripheral
 */
static void BSP_UART3_Init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

    Fifo_Init(&fifos[UART_3]);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

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

    UART_InitStruct.USART_BaudRate = 115200;
    UART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    UART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    UART_InitStruct.USART_Parity = USART_Parity_No;
    UART_InitStruct.USART_StopBits = USART_StopBits_1;
    UART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &UART_InitStruct);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);

    USART_Cmd(USART3, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    setvbuf(stdout, NULL, _IONBF, 0);
}
#endif

void BSP_UART_Init(UART_t uartSelect) {
    if(UART_2 == uartSelect) {
        BSP_UART2_Init();
    } else if(UART_3 == uartSelect) {
        BSP_UART3_Init();
    }
}

/**
 * @brief   Gets one line of ASCII text that was received 
 *          from a specified UART device
 * @pre     str should be at least 128bytes long.
 * @param   uart device selected
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(UART_t uartSelect, char *str) {
    USART_TypeDef *uart;
    UartFifo_t *fifo;
    getUSARTReferences(uartSelect, &uart, &fifo);

    if(fifo->lineReceived) {
        USART_ITConfig(uart, USART_IT_RXNE, RESET);
        uint8_t data = 0;
        uint32_t recvd = 0;
        RxFifo_Peek(fifo, &data);
        while(!RxFifo_IsEmpty(fifo) && data != '\r') {
            recvd += RxFifo_Get(fifo, (uint8_t *)str++);
            RxFifo_Peek(fifo, &data);
        }

        RxFifo_Get(fifo, &data);

        *str = 0;

        fifo->lineReceived = false;
        USART_ITConfig(uart, USART_IT_RXNE, SET);
        return recvd;
    }

    return 0;
}

/**
 * @brief   Transmits data to through a specific 
 *          UART device (represented as a line of data 
 *          in csv file).
 * @param   uart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART_Write(UART_t uartSelect, char *str, uint32_t len) {
    USART_TypeDef *uart;
    UartFifo_t *fifo;
    getUSARTReferences(uartSelect, &uart, &fifo);

    USART_ITConfig(uart, USART_IT_TC, RESET);
    uint32_t sent = 0;
    while(*str != '\0' && len > 0) {
        sent += TxFifo_Put(fifo, *str);
        str++;
        len--;
    }
    USART_ITConfig(uart, USART_IT_TC, SET);
    return sent;
}

void USART2_IRQHandler(void) {
    UartFifo_t *fifo = &fifos[UART_2];

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {

        uint8_t data = USART2->DR;
        bool removeSuccess = 1;

        if(data == '\r') {
            fifo->lineReceived = true;
        }

        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        if(data != '\b' && data != '\177') {

            // Sweet, just a "regular" key. Put it into the fifo
            // Doesn't matter if it fails. If it fails, then the data gets thrown away
            // and the easiest solution for this is to increase USART2_RX_SIZE
            RxFifo_Put(fifo, data);

        } else {

            uint8_t junk = 0;

            // Delete the last entry!
            removeSuccess = RxFifo_RemoveLast(fifo, &junk);
        }

        if(removeSuccess) {
            USART2->DR = data;
        }

    }

    if(USART_GetITStatus(USART2, USART_IT_TC) != RESET) {

        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!TxFifo_Get(fifo, (uint8_t*)&(USART2->DR))) {
            USART_ITConfig(USART2, USART_IT_TC, RESET);
        }

    }

    if(USART_GetITStatus(USART2, USART_IT_ORE) != RESET) {

    }
}

void USART3_IRQHandler(void) {
    UartFifo_t *fifo = &fifos[UART_3];

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {

        uint8_t data = USART3->DR;
        bool removeSuccess = 1;

        if(data == '\r') {
            fifo->lineReceived = true;
        }

        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        if(data != '\b' && data != '\177') {

            // Sweet, just a "regular" key. Put it into the fifo
            // Doesn't matter if it fails. If it fails, then the data gets thrown away
            // and the easiest solution for this is to increase RX_SIZE
            RxFifo_Put(fifo, data);

        } else {

            uint8_t junk = 0;

            // Delete the last entry!
            removeSuccess = RxFifo_RemoveLast(fifo, &junk);
        }

        if(removeSuccess) {
            USART3->DR = data;
        }

    }

    if(USART_GetITStatus(USART3, USART_IT_TC) != RESET) {

        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!TxFifo_Get(fifo, (uint8_t*)&(USART3->DR))) {
            USART_ITConfig(USART3, USART_IT_TC, RESET);
        }

    }

    if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET) {

    }
}

static void Fifo_Init(UartFifo_t *fifo) {
    fifo->lineReceived = false;

    fifo->rxGet = 0;
    fifo->rxPut = 0;

    fifo->txGet = 0;
    fifo->txPut = 0;
}

static bool TxFifo_Get(UartFifo_t *fifo, uint8_t *data) {
    if(!TxFifo_IsEmpty(fifo)) {
        *data = fifo->txBuffer[fifo->txGet];
        fifo->txGet = (fifo->txGet + 1) % TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_Put(UartFifo_t *fifo, uint8_t data) {
    if(!TxFifo_IsFull(fifo)) {
        fifo->txBuffer[fifo->txPut] = data;
        fifo->txPut = (fifo->txPut + 1) % TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_IsFull(UartFifo_t *fifo) {
    return (fifo->txPut + 1) % TX_SIZE == fifo->txGet;
}

static bool TxFifo_IsEmpty(UartFifo_t *fifo) {
    return fifo->txGet == fifo->txPut;
}

static bool RxFifo_Get(UartFifo_t *fifo, uint8_t *data) {
    if(!RxFifo_IsEmpty(fifo)) {
        *data = fifo->rxBuffer[fifo->rxGet];
        fifo->rxGet = (fifo->rxGet + 1) % RX_SIZE;
        return true;
    }

    return false;
}

static bool RxFifo_Put(UartFifo_t *fifo, uint8_t data) {
    if(!RxFifo_IsFull(fifo)) {
        fifo->rxBuffer[fifo->rxPut] = data;
        fifo->rxPut = (fifo->rxPut + 1) % RX_SIZE;
        return true;
    }

    return false;
}

static bool RxFifo_RemoveLast(UartFifo_t *fifo, uint8_t *data) {
    if(!RxFifo_IsEmpty(fifo)) {
        *data = fifo->rxBuffer[fifo->rxPut - 1];
        
        if(fifo->rxPut > 0) {
            fifo->rxPut = (fifo->rxPut - 1);
        }else {
            fifo->rxPut = RX_SIZE - 1;
        }

        return true;
    }

    return false;
}

static bool RxFifo_Peek(UartFifo_t *fifo, uint8_t *data) {
    if(!RxFifo_IsEmpty(fifo)) {
        *data = fifo->rxBuffer[fifo->rxGet];
        return true;
    }

    return false;
}

static bool RxFifo_IsFull(UartFifo_t *fifo) {
    return (fifo->rxPut + 1) % RX_SIZE == fifo->rxGet;
}

static bool RxFifo_IsEmpty(UartFifo_t *fifo) {
    return fifo->rxGet == fifo->rxPut;
}
