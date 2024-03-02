#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void){
    // This is an example of how to use DEBUG_<TEST_FILE> MACRO
    
    BspUartInit(kUart2);
    #ifdef DEBUG_HELLOWORLD
    printf("This print statement means that DEBUG_HELLOWORLD is defined\n");
    #else
    printf("Hello World, your DEBUG_HELLOWORLD is not defined");
    #endif

    while(1){}

    return 0;
}