import csv
import os

# Path of file
file = "BSP/Simulator/Hardware/Data/UART.csv"

# Data
data = {
    "speed" : 0,
    "cruise_control_en" : 0,
    "cruise_control_set" : 0,
    "regen_en" : 0,
    "CAN_err" : 0
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
        open(file, 'w')
    uart = list()
    with open(file, 'r') as csvfile:
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            uart.append(row)
    if len(uart):
        uart = uart[0]
        for i, d in enumerate(data.keys()):
            data[d] = uart[i]
    return data
