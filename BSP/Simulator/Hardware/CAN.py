import csv
import os

# Path of file
file = "BSP/Simulator/Hardware/Data/CAN.csv"

def read():
    """Reads CAN1 bus

    Returns:
        list: [ID, Message]
    """
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        open(file, 'w')
    message = []
    with open(file, 'r') as csvfile:
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            message.append(row)
    message = [0, 0] if message == [] else message[0][:-1]
    return message
