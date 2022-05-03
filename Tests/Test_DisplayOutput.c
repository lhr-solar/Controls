#include "common.h"
#include "config.h"
#include "Display.h"
#include "Tasks.h"
#include <bsp.h>

int main(void) {
    Display_Init();
    Display_SetMainView();
    Display_NoErrors();
}