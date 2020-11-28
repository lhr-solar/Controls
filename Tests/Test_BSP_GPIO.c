#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void) {

    BSP_GPIO_Init(PORTA);
    BSP_GPIO_Init(PORTB);

    char port;
    char rw;
    uint16_t data = 0;

    while (1) {
        printf("Choose Port (A-D): ");
        port = getchar();
        getchar();

        if (port - 'A' >= NUM_PORTS) {
            printf("Invalid input\n\r");
            continue;
        }

        printf("Choose read or write (r/w): ");
        rw = getchar();
        getchar();

        if (rw == 'r' || rw == 'R') {
            data = BSP_GPIO_Read(port - 'A');
            printf("Data from Port %c: %d\n\r\n\r", port, data);
        } else if (rw == 'w' || rw == 'W') {
            printf("Enter the data: ");
            char buffer[10] = {0};
            fgets(buffer, 10, stdin);
            data = atoi(buffer);
            printf("\n\r");
            BSP_GPIO_Write(port - 'A', data);
        } else {
            printf("Invalid input\n\r");
        }
    }
}