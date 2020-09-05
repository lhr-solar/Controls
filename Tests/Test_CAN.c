#include "../Apps/Inc/common.h"
#include "../Drivers/Inc/CAN.h"

int main(void){
   uint8_t ids[10] = {0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x24B, 0x24E, 0x580, 0x10A};
   uint8_t buffer[8];

   CANData_t data;
   CANPayload_t payload;
   payload.data = data;

   CAN_t bus;
   
   CAN_Init();

   for(int i=0; i<sizeof(messages)/sizeof(messages[0]); i++){
   	CAN_Send(bus, ids[i], payload);
   	printf(CAN_Read(bus, buffer));
   }

   exit(0);
}