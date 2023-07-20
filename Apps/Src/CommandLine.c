#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"

#define MAX_BUFFER_SIZE	128	// defined from BSP_UART_Read function

// Represents a command that the command line understands
struct Command {
	const char *name;
	bool (*action)(void);
};

static bool cmd_help(void);

static bool cmd_CANbus_Send(void);

static bool cmd_CANbus_Read(void);

static bool cmd_Contactors_Get(void);

static bool cmd_Contactors_Set(void);

static bool cmd_Minions_Read_Input(void);

static bool cmd_Minions_Write(void);

static bool cmd_Pedals_Read(void);


const struct Command cmdline_commands[] = {
	{.name = "help", .action = cmd_help},
	{.name = "CANbus_Send", .action = cmd_CANbus_Send},
	{.name = "CANbus_Read", .action = cmd_CANbus_Read},
	{.name = "Contactors_Get", .action = cmd_Contactors_Get},
	{.name = "Contactors_Set", .action = cmd_Contactors_Set},
	{.name = "Minions_Read_Input", .action = cmd_Minions_Read_Input},
	{.name = "Minions_Write", .action = cmd_Minions_Write},
	{.name = "Pedals_Read", .action = cmd_Pedals_Read},
	{.name = NULL, .action = NULL}
};

static char input[MAX_BUFFER_SIZE];
char *save; // Save pointer for strtok_r

char *help = {
	"LHRS Controls Command Line:\n\r"
	"	For help, enter [help]\n\r"
	"	Format is: cmd [param, ...]\n\r"
	"	Commands and their params are as follows:\n\r"
	"	CANbus_Send (non)blocking motor/car 'string' - Sends a CAN\n\r"
	"message with the string data as is on the determined line\n\r"
	"	CANbus_Read (non)blocking motor/car - Reads a CAN message\n\r"
	"on the detemined line\n\r"
	"	Contactors_Get array_c/array_p/motor_c - Gets the status of\n\r"
	"determined contactor\n\r"
	"	Contactors_Set array_c/array_p/motor_c on/off (non)blocking -\n\r"
	"Sets the determined contactor\n\r"
	"	Contactors_Enable  array_c/array_p/motor_c - Enables the determined\n\r"
	"contactor\n\r"
	"	Contactors_Disable  array_c/array_p/motor_c - Disables the determined\n\r"
	"contactor\n\r"
	"	Minions_Read_Input 'input' - Reads the current status of the input\n\r"
	"	Minions_Write `output` on/off - Sets the current state of the output\n\r"
	"	Pedals_Read accel/brake - Reads the current status of the pedal\n\r"
};

static inline bool isWhiteSpace(char character){
	switch (character) {
		case 0x09:
		case 0x0A:
		case 0x0C:
		case 0x0D:
		case 0x20: return true;
		default: return false;
	}
}

static bool executeCommand(char *input) {
	// The first word in the input should be a valid command
	char *command = strtok_r(input, " ", &save);
	// Iterate through all valid commands and check if the input matches (exits if the action is NULL (acts as a sentinal))
	for (int i=0; cmdline_commands[i].action; i++) {
		if (!strcmp(command, cmdline_commands[i].name)) {
			return cmdline_commands[i].action(); // Execute the command
		}
	}
	return false; // Didn't find a valid command!
}

// *********** Command line (shell) ************
void Task_CommandLine(void* p_arg) { 
	OS_ERR err;

	// output welcome/help screen
	printf(help);
	
	while(1){
		printf("> ");
		BSP_UART_Read(UART_2, input);
		printf("\n\r");

		if (!executeCommand(input)) { // If command failed, error
			printf("Bad cmd. Please try again\n\r");
		}
	}

    // Delay of 1 seconds
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    if (err != OS_ERR_NONE){
        assertOSError(OS_NONE_LOC, err);
    }
}

// Function Implementations
// ------------------------------------------------------------------------

static inline bool cmd_help(void) {
	printf(help);
	return true;
}

/* This has not been tested and not sure if this implementation is correct (not sure
if we want to just transmit string data from serial) */
static bool cmd_CANbus_Send(void){
	char *data = strtok_r(NULL, " ", &save);
	CANDATA_t msg = {.ID=0x0582, .idx=0};		// this would change in the future (don't assume char as data)
	for(int i = 0; i < 8 && i < strlen(data); i++){
		msg.data[i] = data[i];
	}

	char *blockInput = strtok_r(NULL, " ", &save);
	bool blocking;
	if(strcmp(blockInput, "blocking") == 0){
		blocking = CAN_BLOCKING;
	}
	else if(strcmp(blockInput, "nonblocking") == 0){
		blocking = CAN_NON_BLOCKING;
	}
	else{
		return false;
	}

	char *busInput = strtok_r(NULL, " ", &save);
	CAN_t bus;
	if(strcmp(busInput, "motor") == 0){
		bus = CAN_1;
	}
	else if(strcmp(busInput, "car") == 0){
		bus = CAN_3;
	}
	else{
		return false;
	}

	if(CANbus_Send(msg, blocking, bus)){
		printf("msg sent on %s (%s)\n\r", "can", blockInput);
	}else{
		printf("msg sent failed\n\r");
	}
	return true;
}

static bool cmd_CANbus_Read(void){
	CANDATA_t msg;

	char *blockInput = strtok_r(NULL, " ", &save);
	bool blocking;
	if(strcmp(blockInput, "blocking") == 0){
		blocking = CAN_BLOCKING;
	}
	else if(strcmp(blockInput, "nonblocking") == 0){
		blocking = CAN_NON_BLOCKING;
	}
	else{
		return false;
	}

	char *busInput = strtok_r(NULL, " ", &save);
	CAN_t bus;
	if(strcmp(busInput, "motor") == 0){
		bus = CAN_1;
	}
	else if(strcmp(busInput, "car") == 0){
		bus = CAN_3;
	}
	else{
		return false;
	}

	if(CANbus_Read(&msg, blocking, bus) == SUCCESS){
		printf("msg recieved on %s (%s)\n\r", busInput, blockInput);
		printf("ID: %d, Data: ", msg.ID);
		for(int i = 0; i < 8; i++){
			printf("[%d] %x \n\r", i, msg.data[i]);
		}
	}else{
		printf("read failed on %s (%s)\n\r", busInput, blockInput);
	}
	return true;
}

static bool cmd_Contactors_Get(void){
	char *contactorInput = strtok_r(NULL, " ", &save);
	contactor_t contactor;
	if(strcmp(contactorInput, "array_c") == 0){
		contactor = ARRAY_CONTACTOR;
	}
	else if(strcmp(contactorInput, "array_p") == 0){
		contactor = ARRAY_PRECHARGE;
	}
	else if(strcmp(contactorInput, "motor_c") == 0){
		contactor = MOTOR_CONTACTOR;
	}
	else{
		return false;
	}

	printf("%s state: %s\n\r", contactorInput, Contactors_Get(contactor) == ON ? "on" : "off");
	return true;
}

static bool cmd_Contactors_Set(void){
	char *contactorInput = strtok_r(NULL, " ", &save);
	contactor_t contactor;
	if(strcmp(contactorInput, "array_c") == 0){
		contactor = ARRAY_CONTACTOR;
	}
	else if(strcmp(contactorInput, "array_p") == 0){
		contactor = ARRAY_PRECHARGE;
	}
	else if(strcmp(contactorInput, "motor_c") == 0){
		contactor = MOTOR_CONTACTOR;
	}
	else{
		return false;
	}

	char *stateInput = strtok_r(NULL, " ", &save);
	bool state;
	if(strcmp(stateInput, "on") == 0){
		state = true;
	}
	else if(strcmp(stateInput, "off") == 0){
		state = false;
	}
	else{
		return false;
	}

	char *blockingInput = strtok_r(NULL, " ", &save);
	bool blocking;
	if(strcmp(blockingInput, "blocking") == 0){
		blocking = true;
	}
	else if(strcmp(blockingInput, "nonblocking") == 0){
		blocking = false;
	}
	else{
		return false;
	}

	if(Contactors_Set(contactor, state, blocking)){
		printf("%s set to %s (%s)\n\r", contactorInput, stateInput, blockingInput);
	}else{
		printf("set failed\n\r");
	}
	return true;
}

static bool cmd_Minions_Read_Input(void){
	char *pinInput = strtok_r(NULL, " ", &save);
	pin_t pin;
	if(strcmp(pinInput, "ign_1") == 0){
		pin = IGN_1;
	}
	else if(strcmp(pinInput, "ign_2") == 0){
		pin = IGN_2;
	}
	else if(strcmp(pinInput, "regen_sw") == 0){
		pin = REGEN_SW;
	}
	else if(strcmp(pinInput, "for_sw") == 0){
		pin = FOR_SW;
	}
	else if(strcmp(pinInput, "rev_sw") == 0){
		pin = REV_SW;
	}
	else if(strcmp(pinInput, "cruz_en") == 0){
		pin = CRUZ_EN;
	}
	else if(strcmp(pinInput, "cruz_st") == 0){
		pin = CRUZ_ST;
	}
	else if(strcmp(pinInput, "brakelight") == 0){
		pin = BRAKELIGHT;
	}
	else{
		return false;
	}

	printf("%s is %s\n\r", pinInput, Minions_Read(pin) ? "on" : "off");
	return true;
}

static bool cmd_Minions_Write(void){
	char *pinInput = strtok_r(NULL, " ", &save);
	pin_t pin;
	if(strcmp(pinInput, "brakelight") == 0){
		pin = BRAKELIGHT;
	}
	else{
		return false;
	}

	char *stateInput = strtok_r(NULL, " ", &save);
	bool state;
	if(strcmp(stateInput, "on") == 0){
		state = true;
	}
	else if(strcmp(stateInput, "off") == 0){
		state = false;
	}
	else{
		return false;
	}

	Minions_Write(pin, state);
	printf("%s set to %s\n\r", pinInput, stateInput);
	return true;
}

static bool cmd_Pedals_Read(void){
	char *pedalInput = strtok_r(NULL, " ", &save);
	pedal_t pedal;
	if(strcmp(pedalInput, "accel") == 0){
		pedal = ACCELERATOR;
	}
	else if(strcmp(pedalInput, "brake") == 0){
		pedal = BRAKE;
	}
	else{
		return false;
	}

	printf("%s: %d\n\r", pedalInput, Pedals_Read(pedal));
	return true;
}
