import csv
import os

# Path of file
file = "BSP/Simulator/Hardware/Data/Switches.csv"

# Names of switches
switches = ["LT", "RT", "HDLT", "FWD/REV", "HZD", "CRS_SET", "CRS_EN", "REGEN"]

def toggle(switch):
    """Toggles a specified switch state

    Args:
        switch (string): name of the switch to toggle
    """
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        open(file, 'w')
    states = read()
    for i, sw in enumerate(switches):
        if sw == switch:
            states ^= 1<<i
    states = [states]
    with open(file, 'w') as csvfile:
        csvreader = csv.writer(csvfile)
        csvreader.writerow(states)


def read():
    """Reads switch states

    Returns:
        int: bit string of switch states
    """
    states = []
    with open(file, 'r') as csvfile:
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            states.append(row)
    states = 0 if states == [] else int(states[0][0])
    return states

if __name__ == '__main__':
    toggle("RT")
    print(read())
