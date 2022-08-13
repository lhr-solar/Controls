#include "MotorController.h"
#include "os.h"
#include "Tasks.h"

#include "Display.h"
#include "CANbus.h"

#include "Contactors.h"



//status msg masks
#define MASK_LOW_VOLTAGE_LOCKOUT_ERR (1<<22) //Low voltage rail lockout happened
#define MASK_HALL_SENSOR_ERR (1<<19) //check for slip or hall sequence position error on 19 bit
#define MASK_DC_BUS_OVERVOLT_ERR (1<<18) //DC Bus overvoltage error
#define MASK_SOFTWARE_OVER_CURRENT_ERR (1<<17) //Software Over Current error; Tritium firmware detected an overcurrent
#define MASK_HARDWARE_OVER_CURRENT_ERR (1<<16) //Hardware Over current error; Tritium hardware detected an overcurrent
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor 6


#define BYTES_TO_UINT32(bytes) ((bytes[]))

static bool restartFinished = true;
static float CurrentVelocity = 0;
static float CurrentRPM = 0;
static float busCurrent = 0.0;
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
    
    if(restartFinished){    
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



static void MotorController_InitCommand(){ //transmits the init command
    CANDATA_t initData;
    initData.data[4] = busCurrent; //put 32bit busCurrent value in most significant 32 bits
    initData.ID = MOTOR_POWER;
    initData.idx = 0;
    ErrorStatus initCommand = CANbus_Send(initData,CAN_BLOCKING,MOTORCAN);
    if (initCommand == ERROR)
    {
        Motor_FaultBitmap = T_INIT_FAIL;
        assertTritiumError(Motor_FaultBitmap);
    }
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
    busCurrent = busCurrentFractionalSetPoint;
    MotorController_InitCommand();
}

/**
 * @brief Restarts the motor controller. 
 * 
 */
void MotorController_Restart(void){
    CPU_TS ts;
	OS_ERR err;
    OSMutexPend(&restartFinished_Mutex, 0, OS_OPT_POST_NONE, &ts, &err);
    assertOSError(0, err);
    restartFinished = false;

    CANDATA_t restartCommand;
    restartCommand.data[0] = 0;
    restartCommand.ID = MOTOR_RESET;
    restartCommand.idx = 0;
    ErrorStatus initCommand = CANbus_Send(restartCommand,CAN_BLOCKING,MOTORCAN);
    if (initCommand == ERROR) {
        restartFinished = true;
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
    CANDATA_t driveCommand;
    *((uint32_t*)&driveCommand.data) = newVelocity; //copy velocity into LS 32 bits
    *(((uint32_t*)&driveCommand.data)+1) = motorCurrent; //copy current into MS 32 bits
    driveCommand.ID = MOTOR_DRIVE;
    driveCommand.idx = 0;
    CANbus_Send(driveCommand,CAN_NON_BLOCKING,MOTORCAN);
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

    uint32_t firstSum = 0;
    uint32_t secondSum = 0;
    CPU_TS ts;
    OS_ERR err;

    CANDATA_t motormsg;
    ErrorStatus status = CANbus_Read(&motormsg,CAN_NON_BLOCKING,MOTORCAN);

    if (status == SUCCESS)
    {
        firstSum = *(uint32_t*)&(motormsg.data); //first 32 bits
        secondSum = *(((uint32_t*)&(motormsg.data))+1); //second 32 bits

        message->id = motormsg.ID;
        message->firstNum = firstSum;
        message->secondNum = secondSum;
        
        switch (motormsg.ID) {
            // If we're reading the output from the Motor Status command (0x241) then 
            // Check the status bits we care about and set flags accordingly
            case MOTOR_STATUS: {
                
                OSMutexPend(&restartFinished_Mutex, 0, OS_OPT_POST_NONE, &ts, &err);
                assertOSError(0, err);
                if(restartFinished == false){
                    MotorController_InitCommand();
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
            default: {
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