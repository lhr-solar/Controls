import os
import csv
import fcntl

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
        with open(file, 'w'):
            pass
    accel_value = int(float(accel_pos) * 4096) # ADC conversion
    brake_value = int(float(brake_pos) * 4096) # ADC conversion
    # The slider goes from 0.0 to 1.0, but we want to cap the ADC value at 4095
    if accel_value == 4096:
        accel_value = 4095
    if brake_value == 4096:
        brake_value = 4095
    
    states = [accel_value, brake_value]
    with open(file, "w") as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.writer(csvfile)
        csvreader.writerow(states)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
