#include "DebugIO.h"


int main(){
    DebugIO_Init();
    while(1){
        for(int i = 0; i < NUM_DEBUG_PINS; i++){
            DebugIO_Toggle(i);
            volatile uint32_t waitTime = 0;
            while(waitTime <= 999999){
                waitTime++;
            }
        }
    }
}