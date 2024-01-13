/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_UART.h"

#include "os.h"
#include "stm32f4xx.h"

#define TX_SIZE 128
#define RX_SIZE 64
#define NUM_UART 2

// Initialize the FIFOs

#define FIFO_TYPE char
#define FIFO_SIZE TX_SIZE
#define FIFO_NAME TxFifo
#include "fifo.h"
static TxFifo usb_tx_fifo;
static TxFifo display_tx_fifo;

#define FIFO_TYPE char
#define FIFO_SIZE RX_SIZE
#define FIFO_NAME RxFifo
#include "fifo.h"
static RxFifo usb_rx_fifo;
static RxFifo display_rx_fifo;

static bool usb_line_received = false;
static bool display_line_received = false;

static Callback usb_rx_callback = NULL;
static Callback usb_tx_callback = NULL;
static Callback display_rx_callback = NULL;
static Callback display_tx_callback = NULL;

static RxFifo *rx_fifos[NUM_UART] = {&usb_rx_fifo, &display_rx_fifo};
static TxFifo *tx_fifos[NUM_UART] = {&usb_tx_fifo, &display_tx_fifo};
static bool *line_recvd_global[NUM_UART] = {&usb_line_received,
                                            &display_line_received};
static USART_TypeDef *handles[NUM_UART] = {USART2, USART3};

static void usartDisplayInit() {
    display_tx_fifo = TxFifoNew();
    display_rx_fifo = RxFifoNew();

    GPIO_InitTypeDef gpio_init_struct = {0};
    USART_InitTypeDef uart_init_struct = {0};

    // Initialize clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // Initialize pins
    gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);
    gpio_init_struct.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOC, &gpio_init_struct);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_USART3);

    // Initialize UART3
    uart_init_struct.USART_BaudRate = 115200;  // NOLINT
    uart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart_init_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    uart_init_struct.USART_Parity = USART_Parity_No;
    uart_init_struct.USART_StopBits = USART_StopBits_1;
    uart_init_struct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &uart_init_struct);

    // Enable interrupts

    USART_Cmd(USART3, ENABLE);

    // Enable NVIC
    NVIC_InitTypeDef nvic_init_structure;
    nvic_init_structure.NVIC_IRQChannel = USART3_IRQn;
    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init_structure.NVIC_IRQChannelSubPriority = 0;
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_structure);
}

static void usartUsbInit() {
    usb_tx_fifo = TxFifoNew();
    usb_rx_fifo = RxFifoNew();

    GPIO_InitTypeDef gpio_init_struct = {0};
    USART_InitTypeDef uart_init_struct = {0};

    // Initialize clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Initialize pins
    gpio_init_struct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA, &gpio_init_struct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    // Initialize UART2
    uart_init_struct.USART_BaudRate = 115200;  // NOLINT
    uart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart_init_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    uart_init_struct.USART_Parity = USART_Parity_No;
    uart_init_struct.USART_StopBits = USART_StopBits_1;
    uart_init_struct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &uart_init_struct);

    USART_Cmd(USART2, ENABLE);

    // Enable NVIC
    NVIC_InitTypeDef nvic_init_structure;
    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init_structure.NVIC_IRQChannelSubPriority = 0;
    nvic_init_structure.NVIC_IRQChannel = USART2_IRQn;
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_structure);

    setvbuf(stdout, NULL, _IONBF, 0);
}

/**
 * @brief   Initializes the UART peripheral
 */
static void bspUartInitInternal(Callback rx_callback, Callback tx_callback,
                                Uart uart) {
    switch (uart) {
        case kUart2:  // their UART_USB
            usartUsbInit();
            usb_rx_callback = rx_callback;
            usb_tx_callback = tx_callback;
            break;
        case kUart3:  // their UART_DISPLAY
            usartDisplayInit();
            display_rx_callback = rx_callback;
            display_tx_callback = tx_callback;
            break;
        default:
            // Error
            break;
    }
}

void BspUartInit(Uart uart) {
    // Not using callbacks for now
    bspUartInitInternal(NULL, NULL, uart);
}

/**
 * @brief   Gets one line of ASCII text that was received. The '\n' and '\r'
 * characters will not be stored (tested on Putty on Windows)
 * @pre     str should be at least 128bytes long.
 * @param   str : pointer to buffer to store the string. This buffer should be
 * initialized before hand.
 * @param   usart : which usart to read from (2 or 3)
 * @return  number of bytes that was read
 */
uint32_t BspUartRead(Uart uart, char *str) {
    char data = 0;
    uint32_t recvd = 0;

    bool *line_recvd = line_recvd_global[uart];

    while (*line_recvd == false) {
        BspUartWrite(
            uart, "",
            0);  // needs to be in. Otherwise, usbLineReceived will not update
    }

    USART_TypeDef *usart_handle = handles[uart];

    USART_ITConfig(usart_handle, USART_IT_RXNE, (FunctionalState)RESET);

    RxFifo *fifo = rx_fifos[uart];

    RxFifoPeek(fifo, &data);
    while (!RxFifoIsEmpty(fifo) && data != '\r') {
        recvd += RxFifoGet(fifo, (char *)str++);
        RxFifoPeek(fifo, &data);
    }

    RxFifoGet(fifo, &data);
    *str = 0;
    *line_recvd = false;
    USART_ITConfig(usart_handle, USART_IT_RXNE, (FunctionalState)SET);

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
uint32_t BspUartWrite(Uart uart, char *str, uint32_t len) {
    uint32_t sent = 0;

    USART_TypeDef *usart_handle = handles[uart];

    USART_ITConfig(usart_handle, USART_IT_TC, (FunctionalState)RESET);

    TxFifo *fifo = tx_fifos[uart];

    while (sent < len) {
        if (!TxFifoPut(fifo, str[sent])) {
            // Allow the interrupt to fire
            USART_ITConfig(usart_handle, USART_IT_TC, (FunctionalState)SET);
            // Wait for space to open up
            while (TxFifoIsFull(fifo)) {
                ;
            }
            // Disable the interrupt again
            USART_ITConfig(usart_handle, USART_IT_TC, (FunctionalState)RESET);
        } else {
            sent++;
        }
    }

    USART_ITConfig(usart_handle, USART_IT_TC, (FunctionalState)SET);

    return sent;
}

void UsarT2IrqHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
        uint8_t data = USART2->DR;
        bool remove_success = 1;
        if (data == '\r' || data == '\n') {
            usb_line_received = true;
            if (usb_rx_callback != NULL) {
                usb_rx_callback();
            }
        }
        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        else if (data != '\b' && data != '\177') {
            RxFifoPut(&usb_rx_fifo, (char)data);
        }
        // Sweet, just a "regular" key. Put it into the fifo
        // Doesn't matter if it fails. If it fails, then the data gets thrown
        // away and the easiest solution for this is to increase RX_SIZE
        else {
            char junk = 0;
            // Delete the last entry!
            remove_success = RxFifoPopBack(&usb_rx_fifo, &junk);

            USART_SendData(USART2,
                           0x7F);  // NOLINT
                                   // TODO: Not sure if the backspace works.
                                   // Need to test
        }
        if (remove_success) {
            USART2->DR = data;
        }
    }
    if (USART_GetITStatus(USART2, USART_IT_TC) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn
        // off the TX interrupt
        if (!TxFifoGet(&usb_tx_fifo, (char *)&(USART2->DR))) {
            USART_ITConfig(USART2, USART_IT_TC,
                           (FunctionalState)RESET);  // Turn off the interrupt
            if (usb_tx_callback != NULL) {
                usb_tx_callback();  // Callback
            }
        }
    }
    if (USART_GetITStatus(USART2, USART_IT_ORE) != RESET) {
    }

    OSIntExit();
}

void UsarT3IrqHandler(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t data = USART3->DR;
        bool remove_success = 1;
        if (data == '\r') {
            display_line_received = true;
            if (display_rx_callback != NULL) {
                display_rx_callback();
            }
        }
        // Check if it was a backspace.
        // '\b' for minicmom
        // '\177' for putty
        if (data != '\b' && data != '\177') {
            RxFifoPut(&display_rx_fifo, (char)data);
            // Sweet, just a "regular" key. Put it into the fifo
            // Doesn't matter if it fails. If it fails, then the data gets
            // thrown away and the easiest solution for this is to increase
            // RX_SIZE
        } else {
            char junk = 0;
            // Delete the last entry!
            remove_success = RxFifoPopBack(&display_rx_fifo, &junk);
        }
        if (remove_success) {
            USART3->DR = data;
        }
    }
    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET) {
        // If getting data from fifo fails i.e. the tx fifo is empty, then turn
        // off the TX interrupt
        if (!TxFifoGet(&display_tx_fifo, (char *)&(USART3->DR))) {
            USART_ITConfig(USART3, USART_IT_TC, (FunctionalState)RESET);
            if (display_tx_callback != NULL) {
                display_tx_callback();
            }
        }
    }
    if (USART_GetITStatus(USART3, USART_IT_ORE) != RESET) {
    }

    OSIntExit();
}
