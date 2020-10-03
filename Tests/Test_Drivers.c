#include "common.h"
#include "config.h"
#include "CANbus.h"
#include "Display.h"
#include "MotorController.h"
#include "Pedals.h"

#define TX_SIZE 128

/*
 * Moves the cursor up n lines
*/
static void moveCursorUp(int n){
    if (n < 1) n = 1;
    if (n > 99) n = 99;
    printf("%c[%dA", 0x1B, n);
}

int main(){
    // CAN Driver - SET UP --------------------
    CANbus_Init();

    // NOTE:  MC_BUS = 0x242, VELOCITY = 0x243, MC_PHASE_CURRENT = 0x244, VOLTAGE_VEC = 0x245, CURRENT_VEC = 0x246, BACKEMF = 0x247, 
    //        TEMPERATURE = 0x24B, ODOMETER_AMPHOURS = 0x24E, CAR_STATE = 0x580, MOTOR_DISABLE = 0x10A
    uint32_t ids[10] = {0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x24B, 0x24E, 0x580, 0x10A};
    uint8_t buffer[8];
    uint8_t sizeIds = sizeof(ids)/sizeof(ids[0]);

    // Random data
    CANData_t data;
    data.d = 0x87654321;

    CANPayload_t payload;
    payload.data = data;
    payload.bytes = 4;

    int CANbusIndex = 0;
    // ---------------------------------------

    // Display Driver - SET UP ---------------
    Display_Init();
    display_data_t dataPacket;
    char in[TX_SIZE];
    // ---------------------------------------

    // MotorController - SET UP --------------
    MotorController_Init();
    // ---------------------------------------

    // Pedals - SET UP -----------------------
    Pedals_Init();
    // ---------------------------------------

    
    while(1){
        printf("-----------------------CAN DRIVER TEST------------------------\n");
        printf("--------------------------------------------------------------\n");
        
        //Testing CANbus_Send funtion
        printf("Sent ID: 0x%x and sent Message: 0x%x\n", ids[CANbusIndex], payload.data);
        CANbus_Send(ids[CANbusIndex], payload);
        

        //Testing CANbus_Read funtion - NOTE: ASK IF FUNCTION IS SUPPOSED TO ONLY CHECK FOR MOTOR DISABLE ID
        printf("CANbus_Read must return success if sent ID = 0x10A (Motor Disable): \n");
        uint8_t successCANRead = CANbus_Read(buffer);
        printf("CANbus_Read Success(1)/Failure(0) = %d \n", successCANRead);
        if(successCANRead){
            printf("Success, message read: 0x");
            for(int i = 0; i < sizeIds/2 -1; i++){
                printf("%x",*(buffer+i));
            }
            printf(" \n");
        }else{
            printf("Failure: No message read              \n");
        }

        CANbusIndex++;
        if(CANbusIndex >= sizeIds){
            CANbusIndex = 0;
        }

        printf(" \n");
        printf("--------------------DISPLAY DRIVER TEST-----------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("--------The data is also being displayed in the GUI-----------\n");
       
        // Testing Display_SetData function
        dataPacket.speed = (rand() % 500) / 10.0;
        dataPacket.cruiseEnabled = rand() % 2;
        dataPacket.cruiseSet = rand() % 2;
        dataPacket.regenEnabled = rand() % 2;
        dataPacket.canError = rand() % 10;

        Display_SetData(&dataPacket);

        BSP_UART_Read(UART_2, in);
        in[strlen(in)-1] = '\0'; // remove the newline
        printf("Speed,Cruise_En,Cruise_Set,Regen_En,CAN_err: %s\n", in);

        printf(" \n");
        printf("-----------------MOTOR CONTROLLER DRIVER TEST-----------------\n");
        printf("--------------------------------------------------------------\n");
        // Testing MotorController_Drive function
        uint32_t newVelocity = 0x5053;
        uint32_t motorCurrent = 0x4056;
        MotorController_Drive(motorCurrent, newVelocity);
        

        // Testing MotorController_Read function
        CANbuff tester = {0 ,0, 0};
        bool check = MotorController_Read(&tester);
        printf("The ID on the bus was: 0x%x, Success: %d\n",tester.id, check);     
        printf("Message 1 (Current in %%): 0x%x\n", tester.firstNum);
        printf("Message 2 (Velocity in m/s): 0x%x\n", tester.secondNum);

        printf(" \n");
        printf("-----------------------PEDAL DRIVER TEST----------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("-------It is possible to control the pedals from the GUI------\n");
        // Testing pedalsRead function
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\n", Pedals_Read(ACCELERATOR), Pedals_Read(BRAKE));



        // Delay for-loop which is only used due to the testing nature of this file
        for(int i = 0; i < 500000000; i++){
            //delay
        }

        // Moving cursor up
        moveCursorUp(22);
    }

    return 0;
}