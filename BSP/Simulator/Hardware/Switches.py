# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import tkinter as tk
from ShiftRegister import reg 

# Path of file
file = "BSP/Simulator/Hardware/Data/GPIO.csv"

# Names of switches
switches = ["CRUZ_ST", "CRUZ_EN", "HZD_SQ", "FR_SW", "Headlight_SW", "LEFT_SW", "RIGHT_SW", "REGEN", "IGN_1", "IGN_2","REV_SW"]


def get_switches():
    return switches


def toggle(switch, gui):
    """Toggles a specified switch state
    Args:
        switch (string): name of the switch to toggle
        gui (tk.Button list): Button object list
    """
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
    #check the length of the states variable; if its 9 bits, then ign1 is on, if its 10, ign2 is on, if its 11 Reverse is on and if its less than 10 none are on
    if len(bin(states)[2:]) > 8:
        revState = (states & 1<<10)>>10
        ignStates = states & 3<<8
        writeGPIO(ignStates)
        reg.REGISTER_DATA["GPIOB"] = reg.REGISTER_DATA["GPIOB"]|revState<<7
        reg.REGISTER_DATA["GPIOA"]=states & 0x0FF
    else: 
        writeGPIO(0)
        reg.REGISTER_DATA["GPIOB"] = reg.REGISTER_DATA["GPIOB"] & 0x7F
        reg.REGISTER_DATA["GPIOA"]=states
   
    
def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    # should be updated to also pull GPIO for Ignition
    gpioStates = readGPIO()
    states = reg.REGISTER_DATA["GPIOA"]
    revState = (reg.REGISTER_DATA["GPIOB"] & 0x80)>>7
    states = states|revState<<10
    states = states|gpioStates<<8
    return states

def writeGPIO(ignStates):
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass
    
    lines = []
    # Grab the current GPIO data
    with open(file, "r") as csv:
        fcntl.flock(csv.fileno(), fcntl.LOCK_EX)
        lines = csv.readlines()

    #If the file hasn't been initialzed yet, set the two entries to empty
    length = len(lines)
    if length < 4:
        for i in range(length,2):
            lines.append('\n')
    

    with open(file, "w") as csv:
        for (i, line) in enumerate(lines):
            if i == 0:
                csv.write(str(ignStates>>8))
                csv.write('\n')
            else:
                csv.write(line)

        fcntl.flock(csv.fileno(), fcntl.LOCK_UN)

def readGPIO():
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass
    
    lines = []
    # Grab the current GPIO data
    with open(file, "r") as csv:
        fcntl.flock(csv.fileno(), fcntl.LOCK_EX)
        lines = csv.readlines()
    return int(lines[0])
