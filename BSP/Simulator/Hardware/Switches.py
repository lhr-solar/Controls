# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import tkinter as tk

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
    states = [states]
    with open(file, 'w') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.writer(csvfile)
        csvreader.writerow(states)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)


def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass
    states = []
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            states.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
    states = 0 if states == [] else int(states[0][0])
    return states
