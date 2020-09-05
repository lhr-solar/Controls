#include "BSP_UART2.h"
#include "stm32f4xx.h"

#define USART2_TX_SIZE     128
#define USART2_RX_SIZE     64

static char txBuffer[USART2_TX_SIZE];
static uint32_t txPut = 0;
static uint32_t txGet = 0;

static char rxBuffer[USART2_RX_SIZE];
static uint32_t rxPut = 0;
static uint32_t rxGet = 0;
static bool lineReceived = false;

static bool TxFifo_Get(uint8_t *data);
static bool TxFifo_Put(uint8_t data);
static bool TxFifo_IsFull(void);
static bool TxFifo_IsEmpty(void);

static bool RxFifo_Get(uint8_t *data);
static bool RxFifo_Put(uint8_t data);
static bool RxFifo_RemoveLast(uint8_t *data);
static bool RxFifo_Peek(uint8_t *data);
static bool RxFifo_IsFull(void);
static bool RxFifo_IsEmpty(void);

/**
 * @brief   Initializes the UART peripheral
 */
void BSP_UART2_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef UART_InitStruct = {0};

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

/**
 * @brief   Gets one line of ASCII text that was received.
 * @pre     str should be at least 128bytes long.
 * @param   str : pointer to buffer to store the string. This buffer should be initialized
 *                  before hand.
 * @return  number of bytes that was read
 */
uint32_t BSP_UART2_ReadLine(char *str) {
    if(lineReceived) {
        USART_ITConfig(USART2, USART_IT_RXNE, RESET);
        uint8_t data = 0;
        uint32_t recvd = 0;
        RxFifo_Peek(&data);
        while(!RxFifo_IsEmpty() && data != '\r') {
            recvd += RxFifo_Get((uint8_t *)str++);
            RxFifo_Peek(&data);
        }

        RxFifo_Get(&data);

        *str = 0;

        lineReceived = false;
        USART_ITConfig(USART2, USART_IT_RXNE, SET);
        return recvd;
    }

    return 0;
}

/**
 * @brief   Transmits data to through UART line
 * @param   str : pointer to buffer with data to send.
 * @param   len : size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART2_Write(char *str, uint32_t len) {
    USART_ITConfig(USART2, USART_IT_TC, RESET);
    uint32_t sent = 0;
    while(*str != '\0' && len > 0) {
        sent += TxFifo_Put(*str);
        str++;
        len--;
    }
    USART_ITConfig(USART2, USART_IT_TC, SET);
    return sent;
}

void USART2_IRQHandler(void) {
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {

        uint8_t data = USART2->DR;
        bool removeSuccess = 1;

        if(data == '\r') {
            lineReceived = true;
        }

        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        if(data != '\b' && data != '\177') {

            // Sweet, just a "regular" key. Put it into the fifo
            // Doesn't matter if it fails. If it fails, then the data gets thrown away
            // and the easiest solution for this is to increase USART2_RX_SIZE
            RxFifo_Put(data);

        } else {

            uint8_t junk = 0;

            // Delete the last entry!
            removeSuccess = RxFifo_RemoveLast(&junk);
        }

        if(removeSuccess) {
            USART2->DR = data;
        }

    }

    if(USART_GetITStatus(USART2, USART_IT_TC) != RESET) {

        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!TxFifo_Get(&(USART2->DR))) {
            USART_ITConfig(USART2, USART_IT_TC, RESET);
        }

    }

    if(USART_GetITStatus(USART2, USART_IT_ORE) != RESET) {

    }
}


static bool TxFifo_Get(uint8_t *data) {
    if(!TxFifo_IsEmpty()) {
        *data = txBuffer[txGet];
        txGet = (txGet + 1) % USART2_TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_Put(uint8_t data) {
    if(!TxFifo_IsFull()) {
        txBuffer[txPut] = data;
        txPut = (txPut + 1) % USART2_TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_IsFull(void) {
    return (txPut + 1) % USART2_TX_SIZE == txGet;
}

static bool TxFifo_IsEmpty(void) {
    return txGet == txPut;
}

static bool RxFifo_Get(uint8_t *data) {
    if(!RxFifo_IsEmpty()) {
        *data = rxBuffer[rxGet];
        rxGet = (rxGet + 1) % USART2_RX_SIZE;
        return true;
    }

    return false;
}

static bool RxFifo_Put(uint8_t data) {
    if(!RxFifo_IsFull()) {
        rxBuffer[rxPut] = data;
        rxPut = (rxPut + 1) % USART2_RX_SIZE;
        return true;
    }

    return false;
}

static bool RxFifo_RemoveLast(uint8_t *data) {
    if(!RxFifo_IsEmpty()) {
        *data = rxBuffer[rxPut - 1];
        
        if(rxPut > 0) {
            rxPut = (rxPut - 1);
        }else {
            rxPut = USART2_RX_SIZE - 1;
        }

        return true;
    }

    return false;
}

static bool RxFifo_Peek(uint8_t *data) {
    if(!RxFifo_IsEmpty()) {
        *data = rxBuffer[rxGet];
        return true;
    }

    return false;
}

static bool RxFifo_IsFull(void) {
    return (rxPut + 1) % USART2_RX_SIZE == rxGet;
}

static bool RxFifo_IsEmpty(void) {
    return rxGet == rxPut;
}
