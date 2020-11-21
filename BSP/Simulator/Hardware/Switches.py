# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import tkinter as tk
import ShiftRegister as sr

# Path of file
file = "BSP/Simulator/Hardware/Data/Switches.csv"

# Names of switches
switches = ["LT", "RT", "HDLT", "FWD/REV", "HZD", "CRS_SET", "CRS_EN", "REGEN", "IGN_1", "IGN_2"]


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
    sr.register_write(states)



def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    # should be updated to use ShiftRegister.py register_read function where Bank is set to 1 (if this throws an error check the SPI_init)
    states = sr.register_read(hex(9))
    return states
