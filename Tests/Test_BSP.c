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

/**
 * Moves the cursor n lines
 */
static void moveCursorUp(int n) {
    if (n < 1) n = 1;
    if (n > 99) n = 99;
    printf("%c[%dA", 0x1B, n);
}

int main() {
    while(1) {
        
    }
}
