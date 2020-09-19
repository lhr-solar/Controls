import os
import csv
import fcntl
 
file = "BSP/Simulator/Hardware/Data/PreCharge.csv";
def read():
    os.makedirs(os.path.dirname(file), exist_ok = True);
    if not os.path.exists(file):
        open(file, 'w')
        file.close()
        
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX);
        try:
            csvreader = csv.reader(csvfile);
            charges = next(csvreader);
        except StopIteration:
            charges = 0;
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN);
        states = []
        states.append(int(charges[0]) % 2)
        states.append(int(charges[0])>>1 % 2)
        return states
    
    