#include "common.h"
#include "config.h"
#include "Lights.h"
#include <unistd.h>

void all_on() {
    for(int i = 0; i < NUM_LIGHTS; i++) {
        Lights_Set(i, ON);
    }
}

void all_off() {
    for(int i = 0; i < NUM_LIGHTS; i++) {
        Lights_Set(i, OFF);
    }
}

int main() {
    Lights_Init();

    while(1) {
        all_on();

        for(volatile int i = 0; i < 10000000; i++) {
            // Delay
        }

        all_off();

        for(volatile int i = 0; i < 10000000; i++) {
            // Delay
        }
    }
}
