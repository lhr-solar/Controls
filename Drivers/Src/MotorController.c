#include "MotorController.h"
#include "os.h"
#include "Tasks.h"

#define MOTOR_DRIVE 0x221
#define MOTOR_STATUS 0x241
#define MOTOR_VELOCITY 0x243
#define MAX_CAN_LEN 8

#define MASK_MOTOR_TEMP_ERR (1<<6) //check if motor temperature is an issue on bit 6
#define MASK_SS_ERR (1<<19) //check for slip or hall sequence position error on 19 bit
#define MASK_CC_ERR (1<<2) //checks velocity on 2 bit
#define MASK_OVER_SPEED_ERR (1<<24) //check if motor overshot max RPM on 24 bit

static OS_SEM	MotorController_MailSem4;
static OS_SEM	MotorController_ReceiveSem4;
static OS_Q     MotorController_RxQueue;
static float CurrentVelocity = 0;

uint16_t Motor_FaultBitmap = T_NONE;

/**
 * @brief   Assert Error if Tritium sends error. When Fault Bitmap is set,
 *          and semaphore is posted, Fault state will run.
 * @param   motor_err Bitmap which has motor error codes
 */
static void assertTritiumError(uint16_t motor_err){
    OS_ERR err;
    if(motor_err != T_NONE){
        FaultBitmap.Fault_TRITIUM = 1;
        OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
        assertOSError(0, err);
    }
}

/**
 * @brief   Releases hold of the mailbox semaphore.
 * @note	Do not call directly.
 */
static void MotorController_Release(void) {
	OS_ERR err;

	OSSemPost(&MotorController_MailSem4,
			  OS_OPT_POST_1,
			  &err);
	assertOSError(0, err);
}

/**
 * @brief	Increments the receive semaphore.
 * @note	Do not call directly.
 */
static void MotorController_ReceiveIncoming(void) {
	OS_ERR err;

	OSSemPost(&MotorController_ReceiveSem4,
			  OS_OPT_POST_1,
			  &err);
	assertOSError(0, err);
}

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void MotorController_Init(){
    OS_ERR err;
    OSSemCreate(&MotorController_MailSem4,
                "Motor Controller Mailbox Semaphore",
                3,	// Number of mailboxes
                &err);
	assertOSError(0, err);

	OSQCreate(&MotorController_RxQueue,
              "Motor Controller Receive Queue",
              (OS_MSG_QTY)64,
              &err);
	assertOSError(0, err);
    BSP_CAN_Init(CAN_3, MotorController_ReceiveIncoming, MotorController_Release);
}

/**
 * @brief   Sends MOTOR DRIVE command on CAN2
 * @param   newVelocity desired motor velocity setpoint in m/s
 * @param   motorCurrent desired motor current setpoint as a percentage of max current setting
 * @return  None
 */ 
void MotorController_Drive(float newVelocity, float motorCurrent){
    CPU_TS ts;
	OS_ERR err;

    uint8_t data[8];
    memcpy(data, &motorCurrent, sizeof(motorCurrent));
    memcpy(data + sizeof(motorCurrent), &newVelocity, sizeof(newVelocity));
    
    OSSemPend(&MotorController_MailSem4,
			  0,
			  OS_OPT_PEND_BLOCKING,
			  &ts,
			  &err);
	assertOSError(0, err);
    ErrorStatus result = BSP_CAN_Write(CAN_3, MOTOR_DRIVE, data, MAX_CAN_LEN);
    if (result == ERROR) {
		MotorController_Release();
	}

}

/**
 * @brief   Reads most recent command from CAN3 bus
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  SUCCESS if a message is read
 */ 
ErrorStatus MotorController_Read(CANbuff *message){

    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t firstSum = 0;
    uint32_t secondSum = 0;
    CPU_TS ts;
	OS_ERR err;
	
	// Check to see if a mailbox is available: BLOCKING
	OSSemPend(&MotorController_ReceiveSem4,
			  0,    
			  OS_OPT_PEND_BLOCKING,
			  &ts,
			  &err);
	assertOSError(0, err);
    uint32_t length = BSP_CAN_Read(CAN_3, &id, data);

    if(length>0){
        message->id = id;
        //get first number (bits 0-31)
        for(int j = 0; j < MAX_CAN_LEN/2; j++){
            firstSum <<= 8;
            firstSum += data[j];
        }
        //get second number (bits 32-63)
        for(int k = MAX_CAN_LEN/2; k < MAX_CAN_LEN; k++){
            secondSum <<= 8;
            secondSum += data[k];
        }
        message->firstNum = firstSum;
        message->secondNum = secondSum;
        
        switch (id) {
            // If we're reading the output from the Motor Status command (0x241) then 
            // Check the status bits we care about and set flags accordingly
            case MOTOR_STATUS: {
                if(MASK_MOTOR_TEMP_ERR & firstSum){
                    Motor_FaultBitmap |= T_TEMP_ERR;
                }

                if(MASK_SS_ERR & firstSum){
                    Motor_FaultBitmap |= T_CC_VEL_ERR;
                }

                if(MASK_CC_ERR & firstSum){
                    Motor_FaultBitmap |= T_SLIP_SPEED_ERR;
                }

                if(MASK_OVER_SPEED_ERR & firstSum){
                    Motor_FaultBitmap |= T_OVER_SPEED_ERR;
                }

                assertTritiumError(Motor_FaultBitmap);
                
                break;
            }
            case MOTOR_DRIVE: {
                convert.n = secondSum;
                CurrentVelocity = convert.f;
                break;
            }
            default: break;
        }


        return SUCCESS;
    }
    return ERROR;
}

/**
 * @brief   Mutex protected read from Velocity parameter
 * @return  velocity value obtained from MotorController_Read
 */ 
float MotorController_ReadVelocity(void){
    return CurrentVelocity;
}
