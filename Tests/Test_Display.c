/**
 * Test file for library to interact with gecko display
 * 
 * This test will send random values to the gecko and 
 * then inspect the UART channel through which the message
 * is suposed to be sent. Ideally, this test should be run 
 * with a gui simulator for the display (whenever that gets written)
 */


#include "common.h"
#include "config.h"
#include "Display.h"
#include <bsp.h>


int main() {
    Display_Init();
    Display_SetMainView();
    Display_SetVelocity(13.46f);
}
