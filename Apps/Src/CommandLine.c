// Inspired by Roie and Nathaniel's RTOS Lab 1
// commands will be added latter
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "BSP_UART.h"

#define MAX_BUFFER_SIZE	128	// defined from BSP_UART_Read function

// Represents a command that the interpreter understands
struct Command {
	const char *name;
	bool (*action)(void);
};

static bool cmd_help(void);

const struct Command interpreter_commands[] = {
	{.name = "help", .action = cmd_help},
	{.name = NULL, .action = NULL}
};

static char input[MAX_BUFFER_SIZE];
char *save; // Save pointer for strtok_r

char *help = {
	"LHRS Controls Interpreter:\n\r"
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
	for (int i=0; interpreter_commands[i].name; i++) {
		if (!strcmp(command, interpreter_commands[i].name)) {
			return interpreter_commands[i].action(); // Execute the command
		}
	}
	return false; // Didn't find a valid command!
}

// *********** Command line interpreter (shell) ************
void Task_CommandLine(void) { 
	// output welcome/help screen
	BSP_UART_Write(UART_2, help, strlen(help));
	
	while(1){
		BSP_UART_Read(UART_2, input);

		if (!executeCommand(input)) { // If command failed, error
			BSP_UART_Write(UART_2, "Bad cmd. Please try again\n\r", strlen("Bad cmd. Please try again\n\r"));
		}
	}
}

// Below are the function implementations for all of the available commands
// ------------------------------------------------------------------------

static bool cmd_help(void) {
	BSP_UART_Write(UART_2, help, strlen(help));
	return true;
}
