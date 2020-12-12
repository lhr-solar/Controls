# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import tkinter as tk
from ShiftRegister import reg 

# Path of file
file = "BSP/Simulator/Hardware/Data/Switches.csv"

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
    if len(bin(states)[2:])>8:
        ignstates = states & 3>>8
    else: 
        pass
    # set both ignitions to 0
    
    #set Register Data
    reg.REGISTER_DATA["GPIOA"]=states



def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    # should be updated to use ShiftRegister.py register_read function where Bank is set to 1 (if this throws an error check the SPI_init)
    states = reg.REGISTER_DATA["GPIOA"]
    return states
