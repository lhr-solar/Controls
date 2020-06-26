#include "BSP_UART.h"

#define TX_SIZE     128
#define RX_SIZE     64

static char txBuffer[TX_SIZE];
static uint32_t txPut = 0;
static uint32_t txGet = 0;

static char rxBuffer[RX_SIZE];
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
void BSP_UART_Init(void) {
    // TODO: Initialize the UART port with interrupts for transmit and receive
}

/**
 * @brief   Gets one line of ASCII text that was received.
 * @pre     str should be at least 128bytes long.
 * @param   str : pointer to buffer to store the string. This buffer should be initialized
 *                  before hand.
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_ReadLine(char *str) {
    if(lineReceived) {
        // TODO: Disable UART Receive interrupts HERE
        uint8_t data = 0;
        RxFifo_Peek(&data);
        while(!RxFifo_IsEmpty() && data != '\r') {
            RxFifo_Get(str++);
            RxFifo_Peek(&data);
        }

        RxFifo_Get(&data);

        *str = 0;

        lineReceived = false;
        // TODO: Enable UART Receive interrupts HERE
        return true;
    }

    return false;
}

/**
 * @brief   Transmits data to through UART line
 * @param   str : pointer to buffer with data to send.
 * @param   len : size of buffer
 * @return  numer of bytes that were sent
 */
uint32_t BSP_UART_Write(char *str, uint32_t len) {
    // TODO: Disble UART Transmit interrupts HERE
    while(*str != '\0' && len > 0) {
        TxFifo_Put(*str);
        str++;
        len--;
    }
    // TODO: Enable UART Transmit interrupts HERE
    return true;
}


void USART3_IRQHandler(void) {
    // TODO: Check for Receive Interrupt
    if(0) {

        // TODO: Set data to byte of data received on UART line
        uint8_t data = 0;
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
            // and the easiest solution for this is to increase RX_SIZE
            RxFifo_Put(data);

        } else {

            uint8_t junk = 0;

            // Delete the last entry!
            removeSuccess = RxFifo_RemoveLast(&junk);
        }

        if(removeSuccess) {
            // TODO: echo data. transmit data
        }

    }

    // TODO: Check for Transmit interrupt flag
    if(0) {

        // TODO: Set data to byte of data that will be transmitted
        uint8_t data = 0;

        // If getting data from fifo fails i.e. the tx fifo is empty, then turn off the TX interrupt
        if(!TxFifo_Get(&data)) {
            // TODO: Disable transmit interrupt flag
        }

    }

}

static bool TxFifo_Get(uint8_t *data) {
    if(!TxFifo_IsEmpty()) {
        *data = txBuffer[txGet];
        txGet = (txGet + 1) % TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_Put(uint8_t data) {
    if(!TxFifo_IsFull()) {
        txBuffer[txPut] = data;
        txPut = (txPut + 1) % TX_SIZE;
        return true;
    }

    return false;
}

static bool TxFifo_IsFull(void) {
    return (txPut + 1) % TX_SIZE == txGet;
}

static bool TxFifo_IsEmpty(void) {
    return txGet == txPut;
}

static bool RxFifo_Get(uint8_t *data) {
    if(!RxFifo_IsEmpty()) {
        *data = rxBuffer[rxGet];
        rxGet = (rxGet + 1) % RX_SIZE;
        return true;
    }

    return false;
}

static bool RxFifo_Put(uint8_t data) {
    if(!RxFifo_IsFull()) {
        rxBuffer[rxPut] = data;
        rxPut = (rxPut + 1) % RX_SIZE;
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
            rxPut = RX_SIZE - 1;
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
    return (rxPut + 1) % RX_SIZE == rxGet;
}

static bool RxFifo_IsEmpty(void) {
    return rxGet == rxPut;
}
