# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

# Path of file
file = "BSP/Simulator/Hardware/Data/CAN.csv"


def read():
    """Reads CAN1 bus
    Returns:
        list: [ID, Message, Length]
    """
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass
    message = []
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            message.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
    message = [0, 0] if message == [] else message[0]

    if not message: # If can is empty list
        message.extend(["Empty", "Empty"])
    return message


def write(id, msg):
    """ Writes the msg and id to CAN1
        Format: id, msg, msg_length
    Doesn't return anything
    """

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

    # If the file hasn't been initialzed yet, set the two entries to empty
    length = len(lines)
    if length < 2:
        for i in range(length,2):
            lines.append('\n')
    
    # Write back the CAN data, modifying the specified one
    CANtext = "%s,%s,%d" % (id, msg, round((len(msg)-2)/2.0+0.1))
    #CANtext = (id + ', ' + msg ', ' + (str)(len(msg)))
    with open(file, "w") as csv:
        for (i, line) in enumerate(lines):
            if i == 0:
                csv.write(CANtext)
                csv.write('\n')
            else:
                csv.write(line)

        fcntl.flock(csv.fileno(), fcntl.LOCK_UN)