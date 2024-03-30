#include "BSP_GPIO.h"

int main(void) {
    uint16_t input_port_a[] = {GPIO_Pin_0,
                               GPIO_Pin_1,
                               GPIO_Pin_4,
                               GPIO_Pin_5,
                               GPIO_Pin_6,
                               GPIO_Pin_7};
    
    // input pin for port B is GPIO_Pin_4
    // output pin for port B is GPIO_Pin_5

    uint8_t data;
                
    printf("\n\n-------INIT PINS-------\n\n");

    printf("Initializing PORT A input pins (all)\n");
    BSP_GPIO_Init(PORTA, 0x00F3, INPUT);
    printf("INIT SUCCESS\n");

    printf("Initializing PORT B input pin\n");
    BSP_GPIO_Init(PORTB, 0x0010, INPUT);
    printf("INIT SUCCESS\n");

    printf("Initializing PORT B output pin\n");
    BSP_GPIO_Init(PORTB, 0x0020, OUTPUT);
    printf("INIT SUCCESS\n");
    
    printf("\n\n-------INPUT PINS-------\n\n");
    for (int i = 0; i < 6; i++) {
        printf("Reading port A, pin %X\n", input_port_a[i]);
        data = BSP_GPIO_Read_Pin(PORTA, input_port_a[i]);
        printf("Pin: %X, data read: %X\n", input_port_a[i], data);
    }

    printf("\nReading port B, pin %X\n", GPIO_Pin_4);
    data = BSP_GPIO_Read_Pin(PORTB, GPIO_Pin_4);
    printf("Pin: %X, data read: %X\n", GPIO_Pin_4, data);


    printf("\n\n-------OUTPUT PINS-------\n\n");

    bool write = 0;
    printf("Writing port B, pin %X\n", GPIO_Pin_5);
    BSP_GPIO_Write_Pin(PORTB, GPIO_Pin_5, write);
    printf("Pin: %X, data write: %d\n", GPIO_Pin_5, write);

    printf("Get state port B, pin %X\n", GPIO_Pin_5);
    data = BSP_GPIO_Get_State(PORTB, GPIO_Pin_5);
    printf("Pin: %X, data read: %X\n", GPIO_Pin_5, data);

    printf("Type 'n' and hit enter to continue.\n");
    while (getchar() != 'n') {}

    write = 1;
    printf("\nWriting port B, pin %X\n", GPIO_Pin_5);
    BSP_GPIO_Write_Pin(PORTB, GPIO_Pin_5, write);
    printf("Pin: %X, data write: %d\n", GPIO_Pin_5, write);

    printf("Get state port B, pin %X\n", GPIO_Pin_5);
    data = BSP_GPIO_Get_State(PORTB, GPIO_Pin_5);
    printf("Pin: %X, data read: %X\n", GPIO_Pin_5, data);
    
}