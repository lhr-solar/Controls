#include "common.h"
#include "os.h"
#include "CANbus.h"
#include <string.h>
#include "BSP_UART.h"

static void print_float(float f){
    int n = (int)f;
    f -= n;
    f *= (1000);
    printf("%d.%d", n, (int)f);
}

//loop through range of currents from 0 to 1
//loop through range of velocity in rpm
//loop by varying both
int main(void){
    //initialize CANBus message
    BSP_UART_Init(UART_2);
    CANbus_Init(MOTORCAN);
    CANDATA_t response;
    CANDATA_t message;
    message.ID = MOTOR_DRIVE;
    message.idx = 0;
    float vel = 0.00f;
    float i = 0.000f;



    //velocity -> 735
    //current is a percentage -> 0 to 1 (current reaches its max in this loop [i = 1])
    for(i = 0.000f; i <= 1.000f; i += 0.020f){ //current
        for(vel = -544; vel <= 544; vel += 50){

            memcpy(&message.data[0], &vel, sizeof(vel));
            memcpy(&message.data[4], &i, sizeof(i));         

            CANbus_Send(message, CAN_BLOCKING, MOTORCAN);   
            CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);

            
            printf("Sending (Current: ");
            print_float(*((float *)&message.data[4]));
            printf(", Velocity:");
            print_float(*((float *)&message.data[0]));
            printf("), \n\rReceiving: (Current: ");
            print_float(*((float *)&response.data[4]));
            printf(", Velocity: ");
            print_float(*((float *)&response.data[0]));
            printf(") \n\r");

            for(int d = 0; d < 1000000; d++){} //delay
        }
    }

    //test at end with v = 20,000 (current limiting)
    vel = 20000;
    memcpy(&message.data[0], &vel, sizeof(vel));
    for(i = 0; i <= 1; i += .02f){

        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);

        printf("Sending (Current: ");
        print_float(*((float *)&message.data[4]));
        printf(", Velocity:");
        print_float(*((float *)&message.data[0]));
        printf("), \n\rReceiving: (Current: ");
        print_float(*((float *)&response.data[4]));
        printf(", Velocity: ");
        print_float(*((float *)&response.data[0]));
        printf(") \n\r");

        for(int d = 0; d < 1000000; d++){} //delay

    }

    vel = -20000;
    memcpy(&message.data[0], &vel, sizeof(vel));
    for(i = 0; i <= 1; i += .02f){
        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);  
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);

        printf("Sending (Current: ");
        print_float(*((float *)&message.data[4]));
        printf(", Velocity:");
        print_float(*((float *)&message.data[0]));
        printf("), \n\rReceiving: (Current: ");
        print_float(*((float *)&response.data[4]));
        printf(", Velocity: ");
        print_float(*((float *)&response.data[0]));
        printf(") \n\r");

        for(int d = 0; d < 1000000; d++){} //delay

    }    

    //leaps in velocity from unrealistic min to max, i stays .5
    i = .5;
    for(vel = -20000; vel <= 20000; vel += 40000){
        memcpy(&message.data[0], &vel, sizeof(vel));
        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);  
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);

        printf("Sending (Current: ");
        print_float(*((float *)&message.data[4]));
        printf(", Velocity:");
        print_float(*((float *)&message.data[0]));
        printf("), \n\rReceiving: (Current: ");
        print_float(*((float *)&response.data[4]));
        printf(", Velocity: ");
        print_float(*((float *)&response.data[0]));
        printf(") \n\r");

        for(int d = 0; d < 1000000; d++){} //delay

    }

    //leaps in current from "unrealistic" min to max, vel stays 300
    i = 0;
    vel = 300;
    for(i = 0; i <= 1; i += 1){
        memcpy(&message.data[0], &vel, sizeof(vel));
        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);  
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);

        printf("Sending (Current: ");
        print_float(*((float *)&message.data[4]));
        printf(", Velocity:");
        print_float(*((float *)&message.data[0]));
        printf("), \n\rReceiving: (Current: ");
        print_float(*((float *)&response.data[4]));
        printf(", Velocity: ");
        print_float(*((float *)&response.data[0]));
        printf(") \n\r");

        for(int d = 0; d < 1000000; d++){} //delay

    }

}