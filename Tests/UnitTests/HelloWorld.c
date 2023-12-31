#include <stdio.h>
#include "fff.h"
#define MOCK_CONTACTORS
#include "Contactors.h"
#include "Unity/unity.h"

int main(void) {
    printf("\rHello world\n\r");
    Contactors_Init();
    printf("\n\rHello world?\n\r");
    bool x = Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR);
    printf("\n\rHello world!\n\r");
    Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, false, true);
    printf("\n\rHello world...\n\r");

    printf("\n\rCall argument for set: %d\n\r", Contactors_Set_fake.arg1_history[0]);
    printf("\n\rHello world~\n\r");
}