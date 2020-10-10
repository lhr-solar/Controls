# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

# Path of file
file = "BSP/Simulator/Hardware/Data/Lights.csv"

# Names of lights
lights = ["M_CNCTR","RSVD_LED","CTRL_FAULT","RIGHT_BLINK","HeadlightPWR","A_CNCTR","BPS_FAULT","LEFT_BLINK","BPS_PWR","BrakeLight"]
 

def get_lights():
    return lights

def read():
    """Reads lights states

    Returns:
        int: bit string of light states
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
