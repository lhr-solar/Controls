#include "common.h"
#include "os.h"
#include "CANbus.h"
#include <string.h>

//loop through range of currents from 0 to 1
//loop through range of velocity in rpm
//loop by varying both
int main(void){
    //initialize CANBus message
    CANDATA_t response;
    CANDATA_t message;
    message.ID = MOTOR_DRIVE;
    message.idx = 0;
    float vel = 0.0f;
    float i = 0.0f;


    //velocity -> 735
    //current is a percentage -> 0 to 1 (current reaches its max in this loop [i = 1])
    for(i = 0; i <= 1; i += .02f){ //current
        for(vel = -735; vel <= 735; vel += 50){

            memcpy(&message.data[0], &vel, sizeof(vel));
            memcpy(&message.data[4], &i, sizeof(i));         

            CANbus_Send(message, CAN_BLOCKING, MOTORCAN);   
            CANbus_Read(&response, CAN_BLOCKING, MOTORCAN);
            
            printf("Sending (Current: %f, Velocity %f), Receiving: (Current: %f, Velocity %f)",
            *((float*)&message.data[4]), *((float*)&message.data[0]), *((float*)&response.data[4]), 
            *((float*)&response.data[0]));          


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

        printf("Sending (Current: %f, Velocity %f), Receiving: (Current: %f, Velocity %f)",
        *((float*)&message.data[4]), *((float*)&message.data[0]), *((float*)&response.data[4]), 
        *((float*)&response.data[0]));  

        for(int d = 0; d < 1000000; d++){} //delay

    }

    vel = -20000;
    memcpy(&message.data[0], &vel, sizeof(vel));
    for(i = 0; i <= 1; i += .02f){
        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);  
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN); 

        printf("Sending (Current: %f, Velocity %f), Receiving: (Current: %f, Velocity %f)",
        *((float*)&message.data[4]), *((float*)&message.data[0]), *((float*)&response.data[4]), 
        *((float*)&response.data[0]));  
   
        for(int d = 0; d < 1000000; d++){} //delay

    }    

    //leaps in velocity from unrealistic min to max, i stays .5
    i = .5;
    for(vel = -20000; vel <= 20000; vel += 40000){
        memcpy(&message.data[0], &vel, sizeof(vel));
        memcpy(&message.data[4], &i, sizeof(i));
        CANbus_Send(message, CAN_BLOCKING, MOTORCAN);  
        CANbus_Read(&response, CAN_BLOCKING, MOTORCAN); 

        printf("Sending (Current: %f, Velocity %f), Receiving: (Current: %f, Velocity %f)",
        *((float*)&message.data[4]), *((float*)&message.data[0]), *((float*)&response.data[4]), 
        *((float*)&response.data[0]));  

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

        printf("Sending (Current: %f, Velocity %f), Receiving: (Current: %f, Velocity %f)",
        *((float*)&message.data[4]), *((float*)&message.data[0]), *((float*)&response.data[4]), 
        *((float*)&response.data[0]));  

        for(int d = 0; d < 1000000; d++){} //delay

    }

}