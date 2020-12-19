# Copyright (c) 2020 UT Longhorn Racing Solar

import os
import csv
import fcntl
 
file = "BSP/Simulator/Hardware/Data/GPIO.csv";

def write(data, port=2):
    os.makedirs(os.path.dirname(file), exist_ok = True);
    if not os.path.exists(file):
        with open(file, 'w'):
            pass

    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX);
        lines = csvfile.readlines()
    
    try:
        lines[port-1] = str(data) + '\n'
        
    except IndexError:
        pass

    with open(file, "w") as csvfile:
        csvfile.writelines(lines)
        fcntl.flock(csv.fileno(), fcntl.LOCK_UN)
    
def read():
    os.makedirs(os.path.dirname(file), exist_ok = True);
    if not os.path.exists(file):
        open(file, 'w')
        with open(file, 'w'):
            pass
        
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX);
        try:
            csvreader = csv.reader(csvfile);
            charges = next(csvreader);
        except StopIteration:
            charges = [0];
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN);
        states = [0, 0]
        states[0] = (int(charges[0]) % 2)
        states[1] = (int(charges[0])>>1 % 2)
        return states
