#include "common.h"
#include "CAN.h"

int main(void){
   uint32_t ids[10] = {0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x24B, 0x24E, 0x580, 0x10A};
   uint8_t buffer[8];

   CANData_t data;
   data.b = 0x11;
   CANPayload_t payload;
   payload.data = data;
   
   CAN_Init();

   for(int i=0; i<sizeof(ids)/sizeof(ids[0]); i++){
   	CAN_Send(ids[i], payload);
   	//printf("Success/Failure: %d\n", CAN_Read(buffer));
   }

   exit(0);
}