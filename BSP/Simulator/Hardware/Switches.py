# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import tkinter as tk
from ShiftRegister import reg 

# Path of file
file = "BSP/Simulator/Hardware/Data/GPIO.csv"

# Names of switches
switches = ["CRUZ_ST", "CRUZ_EN", "HZD_SQ", "FR_SW", "Headlight_SW", "LEFT_SW", "RIGHT_SW", "REGEN", "IGN_1", "IGN_2"]


def get_switches():
    return switches


def toggle(switch, gui):
    """Toggles a specified switch state

    Args:
        switch (string): name of the switch to toggle
        gui (tk.Button list): Button object list
    """

    #needs to be updated to use ShiftRegister register_write() function (bank=1 but if this throws error, check SPI_init)

    states = read()
    for i, sw in enumerate(switches):
        if sw == switch:
            states ^= 1<<i
            if states & 1<<i:
                gui[i].config(relief=tk.SUNKEN)
            else:
                gui[i].config(relief=tk.RAISED)
            if sw == "IGN_2" and states & 1<<i:
                states |= 1<<(i-1)
                gui[i-1].config(relief=tk.SUNKEN)
    #check the length of the states variable; if its 9 bits, then ign1 is on, if its 10, ign2 is on, and if its less than 10 neither is on
    if len(bin(states)[2:]) > 8:
        print("inside if statement")
        print("states" + str(states))
        ignStates = states & 3<<8
        print("ignStates"  + str(ignStates))
        writeGPIO(ignStates)
    else: 
        writeGPIO(0)
    # set both ignitions to 0
    
    #set Register Data
    reg.REGISTER_DATA["GPIOA"]=states

def writeGPIO(ignStates):
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass
    
    lines = []
    # Grab the current CAN data
    with open(file, "r") as csv:
        fcntl.flock(csv.fileno(), fcntl.LOCK_EX)
        lines = csv.readlines()

    #If the file hasn't been initialzed yet, set the two entries to empty
    length = len(lines)
    if length < 4:
        for i in range(length,2):
            lines.append('\n')
    
    # Write back the CAN data, modifying the specified one
    #CANtext = "%s,%s,%d" % (id, msg, round((len(msg)-2)/2.0+0.1))
    #CANtext = (id + ', ' + msg ', ' + (str)(len(msg)))
    with open(file, "w") as csv:
        for (i, line) in enumerate(lines):
            if i == 0:
                csv.write(str(ignStates))
                csv.write('\n')
            else:
                csv.write(line)

        fcntl.flock(csv.fileno(), fcntl.LOCK_UN)


def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    # should be updated to use ShiftRegister.py register_read function where Bank is set to 1 (if this throws an error check the SPI_init)
    states = reg.REGISTER_DATA["GPIOA"]
    return states
