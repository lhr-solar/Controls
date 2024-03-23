/**
 * Test for Minons (high level abstraction layer of GPIO)
 */

#include "Minions.h"

void test(void) {
    BSP_UART_Init(UART_2);
    Minions_Init();

    bool brake = 0;
    int ign1, ign2, regen_sw, for_sw, rev_sw, cruz_en, cruz_st;

    Minions_Write(BRAKELIGHT, brake);
    ign1 = Minions_Read(IGN_1);
    ign2 = Minions_Read(IGN_2);
    regen_sw = Minions_Read(REGEN_SW);
    for_sw = Minions_Read(FOR_SW);
    rev_sw = Minions_Read(REV_SW);
    cruz_en = Minions_Read(CRUZ_EN);
    cruz_st = Minions_Read(CRUZ_ST);
    printf(
        "--------------------\r\nign1 : %d\r\nign2 : %d\r\nregen_sw : %d\r\nfor_sw : %d\r\nrev_sw : %d\r\ncruz_en : %d\r\ncruz_st : %d\r\n--------------------\r\n\r\n",
        ign1, ign2, regen_sw, for_sw,
        rev_sw, cruz_en, cruz_st);

    brake = !brake;

    for (int i = 0; i < 999999999; i++) {}
}

int main(void) {
    test();
    while(1) {}
}
