/**
 * Test file that unifies all individual test files of the BSP module
 * 
 * Run this test in conjunction with the simulator 
 * GUI. 
 * 
 */ 

#include "common.h"
#include "config.h"
#include <unistd.h>
#include "BSP_Timer.h"
#include "BSP_ADC.h"
#include <bsp.h>

int selectionMessage(){
    int selection = -1;
    printf("-----------------------------------------------------\n");
    printf("---------------------BSP TESTS-----------------------\n");
    printf("Test ADC - 0\n");
    printf("Test CAN - 1\n");
    printf("Test Contactors - 2\n");
    printf("Test Switches - 3\n");
    printf("Test Timer - 4\n");
    printf("Test UART - 5\n");
    printf("-----------------------------------------------------\n");
    printf("Provide number of test\n");
    scanf("%d", &selection);

    return selection;
}

int main() {
    while(1) {
        int selection = selectionMessage();
        
        while(selection < 0 || selection > 5){
            printf("\nERROR: NUMBER OF TEST PROVIDED IS NOT BETWEEN 0 AND 5\n");
            selection = selectionMessage();
        }
        break;
    }
}
