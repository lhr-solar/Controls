#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void) {

    BspGpioInit(kPortA, 0xFFFF, 0);
    BspGpioInit(kPortB, 0xFFFF, 1);

    char port;
    char rw;
    uint16_t data = 0;

    while (1) {
        /*printf("Choose Port (A, B): ");
        port = getchar();
        getchar();

        if (port - 'A' >= NUM_PORTS) {
            printf("Invalid input\n\r");
            continue;
        }*/

        printf("All pins in Port A are only read, all pins in Port B are only write\n");
        printf("Choose read (from Port A) or write (to Port B)(r/w): ");
        rw = getchar();
        getchar();

        if (rw == 'r' || rw == 'R') {
            port = 'A';
            data = BspGpioReadPin(kPortA);
            printf("Data from Port A: %d\n\r\n\r", data);
        } else if (rw == 'w' || rw == 'W') {
            printf("Enter the data: ");
            char buffer[10] = {0};
            fgets(buffer, 10, stdin);
            data = atoi(buffer);
            printf("\n\r");
            BspGpioWritePin(kPortB, data);
        } else {
            printf("Invalid input\n\r");
        }
    }
}