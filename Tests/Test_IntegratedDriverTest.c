#include "common.h"
#include "config.h"

#include "Pedals.h"
#include "Display.h"
#include "MotorController.h"
#include "CANbus.h"

int main() {
    //CAN Test
    CANbus_Init();
    uint32_t ids[10] = {0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x24B, 0x24E, 0x580, 0x10A};
    uint8_t buffer[8];
    CANData_t data;
    data.w = 0x87654321;
    CANPayload_t payload;
    payload.data = data;
    payload.bytes = 4;

    for(int i=0; i<sizeof(ids)/sizeof(ids[0]); i++){
        CANbus_Send(ids[i], payload);
        ErrorStatus read = CANbus_Read(buffer);
        printf("\nSent ID: 0x%x - Motor Disable Detected?: %d\n", ids[i], CANbus_Read(buffer));
        //printf("Payload match send?(Success: 1, Fail: 0): %d", buffer[i]==payload.data.w);
        if(read==SUCCESS){

            printf("Payload/Buffer Match?: \n\r");
            printf("BufferValues: \n");
            for(int count=0;count<4;count++){
                printf("%x",buffer[count]);
            };
            printf("\n");
            printf("PayloadVal:\n%x\n",payload.data.w);
        }

    }
    
    // Display init
    Display_Init();
    display_data_t packet;

    // MotorController init
    MotorController_Init();
    CANbuff MotorControllerTest={0,0,0};
    
    //Init Pedals module
    Pedals_Init();

    printf("\n\nAccelerator,    Brake,    MotorControllerBusRead?,    MCB msg ID,    MCB msg\n\n");
    while (1){
        MotorController_Drive((rand()/500),(rand()/500));
        bool check=MotorController_Read(&MotorControllerTest);


        printf("%d\t%d\t%d\t%x\t%x,%x\r",Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE),check,MotorControllerTest.id,MotorControllerTest.firstNum,MotorControllerTest.secondNum);
        packet.speed = (rand() % 500) / 10.0;
        packet.cruiseEnabled = rand() % 2;
        packet.cruiseSet = rand() % 2;
        packet.regenEnabled = rand() % 2;
        packet.canError = rand() % 10;
        Display_SetData(&packet);
    }
}
