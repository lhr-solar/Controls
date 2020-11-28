# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import struct

# Path of file
file = "BSP/Simulator/Hardware/Data/UART.csv"

# Data
data = {
    "speed": 0,
    "cruise_control_en": 0,
    "cruise_control_set": 0,
    "regen_en": 0,
    "CAN_err": 0,
}


def get_display():
    """Getter function for the display keys

    Returns:
        list: list of dictionary keys
    """
    return data.keys()


def read():
    """Reads the UART line

    Returns:
        dict: dictionary with filled in values
    """
    global data
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, "w"):
            pass
    uart = list()
    with open(file, "r") as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            uart.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
    if len(uart):
        uart = uart[0][0]
        speed_str = uart[0:8]

        try:
            speed_arr = [
                int(uart[0:2], 16),
                int(uart[2:4], 16),
                int(uart[4:6], 16),
                int(uart[6:8], 16),
            ]
            (speed,) = struct.unpack(
                "<f", bytearray(speed_arr)
            )  # Convert our byte string to a float
            flags_str = uart[8:10]
            flags = int(flags_str, 16)

            data["speed"] = round(speed, 2)  # Otherwise we get too many decimal places
            data["cruise_control_en"] = (flags >> 3) & 1
            data["cruise_control_set"] = (flags >> 2) & 1
            data["regen_en"] = (flags >> 1) & 1
            data["CAN_err"] = (flags) & 1
        except Exception:
            pass
    return data
