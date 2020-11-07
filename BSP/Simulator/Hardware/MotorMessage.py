# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os

from CAN import write

SAFE = 1
UNSAFE = 0

def sendMotorDisable(message):
	""" Simulates BPS by sending a message over CAN1 
		saying whether or not it is safe to turn on the motor.
		Sends the MOTOR_DISABLE command with disable/enable argument.
		message==1 means that it is safe to turn on the motor,
		message==0 means that the motor should be turned off,
		and all other messages are ignored.
	"""

	MDCommand = hex(266)

	write(MDCommand, hex(message))





