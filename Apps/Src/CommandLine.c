// Inspired by Roie and Nathaniel's RTOS Lab 1
// commands will be added latter
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

static bool cmd_Contactors_Enable(void);

static bool cmd_Contactors_Disable(void);

static bool cmd_Minion_Read_Input(void);

static bool cmd_Minion_Write_Output(void);

static bool cmd_Pedals_Read(void);


const struct Command cmdline_commands[] = {
	{.name = "help", .action = cmd_help},
	{.name = "CANbus_Send", .action = cmd_CANbus_Send},
	{.name = "CANbus_Read", .action = cmd_CANbus_Read},
	{.name = "Contactors_Get", .action = cmd_Contactors_Get},
	{.name = "Contactors_Set", .action = cmd_Contactors_Set},
	{.name = "Contactors_Enable", .action = cmd_Contactors_Enable},
	{.name = "Contactors_Disable", .action = cmd_Contactors_Disable},
	{.name = "Minion_Read_Input", .action = cmd_Minion_Read_Input},
	{.name = "Minion_Write_Output", .action = cmd_Minion_Write_Output},
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
};

bool isWhiteSpace(char character){
	switch (character) {
		case 0x09:
		case 0x0A:
		case 0x0C:
		case 0x0D:
		case 0x20: return true;
		default: return false;
	}
}

bool isNull(char character){
	return character == '\0';
}

bool executeCommand(char *input) {
	// The first word in the input should be a valid command
	char *command = strtok_r(input, " ", &save);
	// Iterate through all valid commands and check if the input matches
	for (int i=0; cmdline_commands[i].name; i++) {
		if (!strcmp(command, cmdline_commands[i].name)) {
			return cmdline_commands[i].action(); // Execute the command
		}
	}
	return false; // Didn't find a valid command!
}

// *********** Command line (shell) ************
void Task_CommandLine(void) { 
	OS_ERR err;

	// output welcome/help screen
	printf(help);
	
	while(1){
		BSP_UART_Read(UART_2, input);

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

// Below are the function implementations for all of the available commands
// ------------------------------------------------------------------------

static bool cmd_help(void) {
	printf(help);
	return true;
}

static bool cmd_CANbus_Send(void){
	char *data = strtok_r(NULL, " ", &save);
	CANDATA_t msg = {.ID=0x0582, .idx=0};		// this would change in the future (don't assume char as data)
	for(int i = 0; i < 8 || strlen(data); i++){
		msg.data[i] = data[0];
	}

	char *blockInput = strtok_r(NULL, " ", &save);
	CAN_blocking_t blocking;
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

	CANbus_Send(msg, blocking, bus);
	return true;
}

static bool cmd_CANbus_Read(void){
	CANDATA_t msg;

	char *blockInput = strtok_r(NULL, " ", &save);
	CAN_blocking_t blocking;
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

	CANbus_Read(&msg, blocking, bus);
	printf("ID: %d, Data: ", msg.ID);
	for(int i = 0; i < 8; i++){
		printf("[%d] %x \n", i, msg.data[i]);
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

	printf("State: %s\n", Contactors_Get(contactor) == ON ? "On" : "Off");
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
	State state;
	if(strcmp(stateInput, "on") == 0){
		state = ON;
	}
	else if(strcmp(stateInput, "off") == 0){
		state = OFF;
	}
	else{
		return false;
	}

	char *blockingInput = strtok_r(NULL, " ", &save);
	bool blocking;
	if(strcmp(blockingInput, "array_c") == 0){
		blocking = true;
	}
	else if(strcmp(blockingInput, "array_p") == 0){
		blocking = false;
	}
	else{
		return false;
	}

	Contactors_Set(contactor, state, blocking);
	return true;
}

static bool cmd_Contactors_Enable(void){
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

	Contactors_Enable(contactor);
	return true;
}

static bool cmd_Contactors_Disable(void){
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

	Contactors_Disable(contactor);
	return true;
}

static bool cmd_Minion_Read_Input(void){
	Minion_Error_t err;
	char *pinInput = strtok_r(NULL, " ", &save);
	MinionPin_t pin;
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
		pin = REGEN_SW;
	}
	else if(strcmp(pinInput, "rev_sw") == 0){
		pin = REGEN_SW;
	}
	else if(strcmp(pinInput, "cruz_en") == 0){
		pin = REGEN_SW;
	}
	else if(strcmp(pinInput, "cruz_st") == 0){
		pin = REGEN_SW;
	}
	// else if(strcmp(pinInput, "brakelight") == 0){	// uncomment when fix implemented
	// 	pin = REGEN_SW;
	// }
	else{
		return false;
	}

	Minion_Read_Input(pin, &err);
	return true;
}

static bool cmd_Minion_Write_Output(void){
	Minion_Error_t err;
	char *pinInput = strtok_r(NULL, " ", &save);
	MinionPin_t pin;
	if(strcmp(pinInput, "brakelight") == 0){	// uncomment when fix implemented
		pin = REGEN_SW;
	}
	else{
		return false;
	}

	char *stateInput = strtok_r(NULL, " ", &save);
	bool state;
	if(strcmp(stateInput, "on") == 0){	// uncomment when fix implemented
		state = true;
	}
	else if(strcmp(stateInput, "off") == 0){
		state = false;
	}
	else{
		return false;
	}

	Minion_Write_Output(pin, state, &err);
	return true;
}

static bool cmd_Pedals_Read(void){
	char *pedalInput = strtok_r(NULL, " ", &save);
	contactor_t pedal;
	if(strcmp(pedalInput, "accel") == 0){
		pedal = ACCELERATOR;
	}
	else if(strcmp(pedalInput, "brake") == 0){
		pedal = BRAKE;
	}
	else{
		return false;
	}

	printf("%s: %d\n", pedalInput, Pedals_Read(pedal));
	return true;
}
