#include "Display.h"

int main(void) {
    Display_Init();
    Display_SetMainView();
    float vel = 13.7f;
    Display_SetVelocity(vel);
}