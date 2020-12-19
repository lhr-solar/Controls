# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

from ShiftRegister import reg

# Path of file
file = "BSP/Simulator/Hardware/Data/GPIO.csv"

# Names of lights
lights = [
    "A_CNCTR",
    "M_CNCTR",
    "CTRL_FAULT",
    "LEFT_BLINK",
    "RIGHT_BLINK",
    "Headlight_ON",
    "BPS_FAULT",
    "BPS_PWR",
    "BrakeLight",
    "RSVD_LED",
]


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
        with open(file, "w"):
            pass
    # Minion Board First
    lights = reg.REGISTER_DATA["GPIOB"]

    states = []
    with open(file, "r") as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            states.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
    try:
        states = int(states[2][0])
    except IndexError:
        states = 0

    lights += (states & 0x80) << 1  # Adds BrakeLight to lights
    return lights
