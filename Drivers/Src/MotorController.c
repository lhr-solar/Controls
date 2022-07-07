#include "MotorController.h"
#include "os.h"
#include "Tasks.h"
#include "Minions.h"
#include "Display.h"

#include "Contactors.h"

#define MOTOR_DRIVE 0x221
#define MOTOR_POWER 0x222
#define MOTOR_STATUS 0x241
#define MOTOR_VELOCITY 0x243
#define MAX_CAN_LEN 8

//status msg masks
#define MASK_LOW_VOLTAGE_LOCKOUT_ERR (1<<22) //Low voltage rail lockout happened
#define MASK_HALL_SENSOR_ERR (1<<19) //check for slip or hall sequence position error on 19 bit
#define MASK_DC_BUS_OVERVOLT_ERR (1<<18) //DC Bus overvoltage error
#define MASK_SOFTWARE_OVER_CURRENT_ERR (1<<17) //Software Over Current error; Tritium firmware detected an overcurrent
#define MASK_HARDWARE_OVER_CURRENT_ERR (1<<16) //Hardware Over current error; Tritium hardware detected an overcurrent
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor 6


#define BYTES_TO_UINT32(bytes) ((bytes[]))

static OS_SEM MotorController_MailSem4;
static OS_SEM MotorController_ReceiveSem4;
static bool restartFinished = false;
static float CurrentVelocity = 0;
static float CurrentRPM = 0;
static OS_MUTEX restartFinished_Mutex;

tritium_error_code_t Motor_FaultBitmap = T_NONE;

static bool is_initialized = false;

/**
 * @brief   Assert Error if Tritium sends error. When Fault Bitmap is set,
 *          and semaphore is posted, Fault state will run.
 * @param   motor_err Bitmap which has motor error codes
 */
static void _assertTritiumError(tritium_error_code_t motor_err)
{
    
    if(restartFinished && Contactors_Get(MOTOR_CONTACTOR)){
        OS_ERR err;
        if(motor_err != T_NONE){
            FaultBitmap |= FAULT_TRITIUM;
            OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
            assertOSError(0, err);
        }
    }
}

#if DEBUG == 1
#define assertTritiumError(motor_err) \
        if (motor_err != T_NONE) { \
            printf("Error asserted at %s, line %d: %d\n\r", __FILE__, __LINE__, motor_err); \
        } \
        _assertTritiumError(motor_err);
#else
#define assertTritiumError(motor_err) _assertTritiumError(motor_err);
#endif

/**
 * @brief   Releases hold of the mailbox semaphore.
 * @note	Do not call directly.
 */
static void MotorController_Release(void)
{
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
static void MotorController_CountIncoming(void)
{
    OS_ERR err;

    OSSemPost(&MotorController_ReceiveSem4,
              OS_OPT_POST_1,
              &err);
    assertOSError(0, err);
}

/**
 * @brief   Initializes the motor controller
 * @param   busCurrentFractionalSetPoint fraction of the bus current to allow the motor to draw
 * @return  None
 */
void MotorController_Init(float busCurrentFractionalSetPoint)
{
    if (is_initialized)
        return;
    is_initialized = true; // Ensure that we only execute the function once
    CPU_TS ts;
    OS_ERR err;
    OSSemCreate(&MotorController_MailSem4,
                "Motor Controller Mailbox Semaphore",
                3, // Number of mailboxes
                &err);
    assertOSError(0, err);

    OSSemCreate(&MotorController_ReceiveSem4,
                "Motor Controller Receive Semaphore",
                0, // Number of mailboxes
                &err);
    assertOSError(0, err);

    OSMutexCreate(&restartFinished_Mutex,
                "Motor Controller Restart Mutex",
                &err);
    assertOSError(0, err);

    BSP_CAN_Init(CAN_3, MotorController_CountIncoming, MotorController_Release);

    uint8_t data[8] = {0};
    memcpy(
        data + 4, // Tritium expects the setpoint in the Most significant 32 bits, so we offset
        &busCurrentFractionalSetPoint,
        sizeof(busCurrentFractionalSetPoint));
    OSSemPend(&MotorController_MailSem4,
              0,
              OS_OPT_PEND_BLOCKING,
              &ts,
              &err);
    assertOSError(0, err);
    ErrorStatus initCommand = BSP_CAN_Write(CAN_3, MOTOR_POWER, data, MAX_CAN_LEN);
    if (initCommand == ERROR)
    {
        MotorController_Release();
        Motor_FaultBitmap = T_INIT_FAIL;
        assertTritiumError(Motor_FaultBitmap);
    }
}

/**
 * @brief Restarts the motor controller. THIS FUNCTION IS FOR FAULT STATE USE ONLY.
 * 
 * @param busCurrentFractionalSetPoint 
 */
void MotorController_Restart(float busCurrentFractionalSetPoint){
    CPU_TS ts;
	OS_ERR err;
    uint8_t data[8] = {0};

    OSMutexPend(&restartFinished_Mutex, 0, OS_OPT_POST_NONE, &ts, &err);
    assertOSError(0, err);

    restartFinished = false;

    memcpy(
        data+4, //Tritium expects the setpoint in the Most significant 32 bits, so we offset
        &busCurrentFractionalSetPoint,
        sizeof(busCurrentFractionalSetPoint)
    );
    OSSemPend(&MotorController_MailSem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &ts,
            &err);
	assertOSError(0, err);
    ErrorStatus initCommand = BSP_CAN_Write(CAN_3, MOTOR_POWER, data, MAX_CAN_LEN);
    if (initCommand == ERROR) {
        restartFinished = true;
		MotorController_Release();

        OSMutexPost(&restartFinished_Mutex, OS_OPT_POST_NONE, &err);
        assertOSError(0, err);

        Motor_FaultBitmap = T_INIT_FAIL;
        assertTritiumError(Motor_FaultBitmap);
        return;
	}

    OSMutexPost(&restartFinished_Mutex, OS_OPT_POST_NONE, &err);
    assertOSError(0, err);
}

/**
 * @brief   Sends MOTOR DRIVE command on CAN3
 * @param   newVelocity desired motor velocity setpoint in m/s
 * @param   motorCurrent desired motor current setpoint as a percentage of max current setting (0.0-1.0)
 * @return  None
 */
void MotorController_Drive(float newVelocity, float motorCurrent)
{
    CPU_TS ts;
    OS_ERR err;

    uint8_t data[8];
    memcpy(data, &newVelocity, sizeof(newVelocity));
    memcpy(data + sizeof(newVelocity), &motorCurrent, sizeof(motorCurrent));

    OSSemPend(&MotorController_MailSem4,
              0,
              OS_OPT_PEND_BLOCKING,
              &ts,
              &err);
    assertOSError(0, err);
    ErrorStatus result = BSP_CAN_Write(CAN_3, MOTOR_DRIVE, data, MAX_CAN_LEN);
    if (result == ERROR)
    {
        MotorController_Release();
    }
}

/**
 * @brief
 *
 *
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  SUCCESS if a message is read
 */
ErrorStatus MotorController_Read(CANbuff *message)
{

    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t firstSum = 0;
    uint32_t secondSum = 0;
    CPU_TS ts;
    OS_ERR err;

    // Check to see if a mailbox is available: BLOCKING
    OSSemPend(
        &MotorController_ReceiveSem4,
        0,
        OS_OPT_PEND_BLOCKING,
        &ts,
        &err);
    assertOSError(0, err);
    ErrorStatus status = BSP_CAN_Read(CAN_3, &id, data);

    if (status == SUCCESS)
    {
        message->id = id;
        //get first number (bits 0-31)
        for(int j = (MAX_CAN_LEN/2)-1 ; j >= 0; j--){
            firstSum <<= 8;
            firstSum += data[j];
        }

        //get second number (bits 32-63)
        for(int k = MAX_CAN_LEN - 1; k >= (MAX_CAN_LEN/2); k--){
            secondSum <<= 8;
            secondSum += data[k];
        }

        message->firstNum = firstSum;
        message->secondNum = secondSum;
        
        switch (id) {
            // If we're reading the output from the Motor Status command (0x241) then 
            // Check the status bits we care about and set flags accordingly
            case MOTOR_STATUS: {
                
                OSMutexPend(&restartFinished_Mutex, 0, OS_OPT_POST_NONE, &ts, &err);
                assertOSError(0, err);
                if(restartFinished == false){
                    restartFinished = true;
                }
                OSMutexPost(&restartFinished_Mutex, OS_OPT_POST_NONE, &err);
                assertOSError(0, err);

                if(MASK_DC_BUS_OVERVOLT_ERR & firstSum){
                    Motor_FaultBitmap |= T_DC_BUS_OVERVOLT_ERR;
                }

                if(MASK_HALL_SENSOR_ERR & firstSum){
                    Motor_FaultBitmap |= T_HALL_SENSOR_ERR;
                }

                if(MASK_HARDWARE_OVER_CURRENT_ERR & firstSum){
                    Motor_FaultBitmap |= T_HARDWARE_OVER_CURRENT_ERR;
                }

                if(MASK_SOFTWARE_OVER_CURRENT_ERR & firstSum){
                    Motor_FaultBitmap |= T_SOFTWARE_OVER_CURRENT_ERR;
                }

                if(MASK_LOW_VOLTAGE_LOCKOUT_ERR & firstSum){
                    Motor_FaultBitmap |= T_LOW_VOLTAGE_LOCKOUT_ERR;
                }

                if(MASK_MOTOR_TEMP_LIMIT & firstSum){
                    Motor_FaultBitmap |= T_TEMP_ERR;
                }


                assertTritiumError(Motor_FaultBitmap);
                
                break;
            }
            case MOTOR_VELOCITY: {
                CurrentVelocity = *(float *) &secondSum;
                CurrentRPM = *(float*) &firstSum;
                break;
            }

            // if(MASK_OVER_SPEED_ERR & firstSum){
            //     Motor_FaultBitmap |= T_OVER_SPEED_ERR;
            // }

            assertTritiumError(Motor_FaultBitmap);

            break;
        }

        return SUCCESS;
    }
    return ERROR;
}

/**
 * @brief   Mutex protected read from Velocity parameter
 * @return  velocity value obtained from MotorController_Read
 */
float MotorController_ReadVelocity(void)
{
    return CurrentVelocity;
}

float MotorController_ReadRPM(void){
    return CurrentRPM;
}

/**
 * @brief Return the static error field from this layer
 * 
 */
tritium_error_code_t MotorController_getTritiumError(void){
    return Motor_FaultBitmap;
}