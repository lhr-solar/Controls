# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

file = "BSP/Simulator/Hardware/Data/GPIO.csv"

ARRAY_PIN = 1
MOTOR_PIN = 2


def read():
    """Reads contactor file and returns 1 (on) or 0 (off)
    Returns:
        list: [motor_state, array_state]
    """
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, "w"):
            pass

    states = []
    with open(file, "r") as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)  # Lock file before reading
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            states.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
    try:
        states = int(states[2][0])
    except IndexError:
        states = 0

    return [(states >> MOTOR_PIN) & 0x01, (states >> ARRAY_PIN) & 0x01]
