import os
import csv

# Path of file
file = "BSP/Simulator/Hardware/Data/Pedals.csv"

def set_pedals(accel_pos, brake_pos):
    """Sets the accelerator and brake pedals to the
       specified position between zero and one

    Args:
        accel_pos (float): the position of the accelerator pedal
        brake_pos (float): the position of the brake pedal
    """
    # Create the file if it doesn't exist yet
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        open(file, 'w')
    accel_value = int(accel_pos * 1024) # ADC conversion
    brake_value = int(brake_pos * 1024) # ADC conversion
    states = [accel_value, brake_value]
    with open(file, "w") as csvfile:
        csvreader = csv.writer(csvfile)
        csvreader.writerow(states)


if __name__ == '__main__':
    set_pedals(0.7, 0.0)