#include "common.h"
#include "os.h"
#include "CANbus.h"
#include <string.h>
#include "BSP_UART.h"

#define DELAY_TIME 100
#define MAXVEL_MPS 6.7056f // 15 mph in m/s

static void print_float(float f){
    int32_t n = (int32_t)f;
    f -= n;
    f *= 100;
    if (f < 0) printf("-");
    int32_t d = (f<0)?-f:f;
    printf("%d.%02d", (int)n, (int)d);
}

static void send_motor_data(CANDATA_t message, CANDATA_t* response_ptr){
    CANbus_Send(message, CAN_BLOCKING, MOTORCAN);   
    CANbus_Read(response_ptr, CAN_BLOCKING, MOTORCAN);
    printf("Sending (Current: ");
    print_float(*((float *)&message.data[4]));
    printf(", Velocity:");
    print_float(*((float *)&message.data[0]));
    printf("), \n\rReceiving: (Velocity (m/s): ");
    print_float(*((float *)&(response_ptr->data[4])));
    printf(", Velocity (RPM): ");
    print_float(*((float *)&(response_ptr->data[0])));
    printf(") \n\r");
}

int main(void){
    //initialize CANBus message
    
    BSP_UART_Init(UART_2);
    CANbus_Init(MOTORCAN);
    
    CANDATA_t response;
    CANDATA_t message;
    message.ID = MOTOR_DRIVE;
    message.idx = 0;
    float vel = 20000.0f;
    float current = 1.0f;
    
    
    OS_ERR err;
    memcpy(&message.data[0], &vel, sizeof(vel));
    memcpy(&message.data[4], &current, sizeof(current));

    uint8_t ms=0;
    
    print_float(123.1321);
    printf("\n");
    print_float(-123.1321);

    // Test ramp up to 15 mph
    while(*((float *)&(response.data[4])) < MAXVEL_MPS){
        send_motor_data(message, &response);
        printf("(%d ms)\n\r", DELAY_TIME*(ms++));
        OSTimeDlyHMSM(0, 0, 0, DELAY_TIME, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    // Coast down to zero velocity
    current = 0.0f;
    memcpy(&message.data[4], &current, sizeof(current));
    ms = 0;
    while(*((float *)&(response.data[4])) > 0.25){
        send_motor_data(message, &response);
        printf("(%d ms)\n\r", DELAY_TIME*(ms++));
        OSTimeDlyHMSM(0, 0, 0, DELAY_TIME, OS_OPT_TIME_HMSM_STRICT, &err);
    }

    return 0;
}