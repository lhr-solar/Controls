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
    data.d = 0x87654321;
    CANPayload_t payload;
    payload.data = data;
    payload.bytes = 4;
    for(int i=0; i<sizeof(ids)/sizeof(ids[0]); i++){
        CANbus_Send(ids[i], payload);
        printf("Sent ID: 0x%x - Success(1)/Failure(0): %d\n", ids[i], CANbus_Read(buffer));
    }
    
    // Display init
    Display_Init();
    display_data_t packet;

    // MotorController init
    MotorController_Init();
    CANbuff MotorControllerTest={0,0,0};
    
    //Init Pedals module
    Pedals_Init();


    //display
    
    int count = 0;
    while(count<10) {
        //pedals
        printf("Accelerator: %d\n\rBrake: %d\n\r",Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
        MotorController_Drive((rand()/500),(rand()/500));
        bool ReadCheck = MotorController_Read(&MotorControllerTest); 
        printf("Message Read from Motor Controller Bus?: %d\n\r", ReadCheck);
        printf("ID of CAN Msg: %x\n\rMessage: %x, %x\n\r",MotorControllerTest.id,MotorControllerTest.firstNum,MotorControllerTest.secondNum);
        packet.speed = (rand() % 500) / 10.0;
        packet.cruiseEnabled = rand() % 2;
        packet.cruiseSet = rand() % 2;
        packet.regenEnabled = rand() % 2;
        packet.canError = rand() % 10;
        Display_SetData(&packet);
        count++;
    }
    exit(0);
}
