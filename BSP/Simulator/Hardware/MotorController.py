import csv
import os
import CAN

# Path of file
file = "BSP/Simulator/Hardware/Data/CAN.csv"


# Relevant IDs
MOTOR_DRIVE_ID = 0x221
VELOCITY_ID = 0x243

# State values
CURRENT_VELOCITY = 0


def read():
    """Reads CAN2 bus

    Returns:
        list: [ID, Message, Length]
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
    message = [0, 0] if message == [] else message[1]
    return message


def write(id_, message):
    """Writes message to CAN2

    Args:
        id_ (int): ID of message to be sent
        message (int): contents of message to be sent
    """    
    CAN1 = CAN.read()
    with open(file, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(CAN1)
        csvwriter.writerow([hex(id_), hex(message), len(hex(message)[2:])//2])


def confirm_drive():
    """Acts as the motor controller confirming
    that the proper message is received periodically.
    If the message is received, the motor speed is changed
    accordingly. If it is not received, the motor speed stays constant.

    Returns:
        tuple: desired and current velocities to display
    """
    try:      
        id_, message, _ = read()
        id_ = int(id_, 16)
        message = int(message, 16)
        if id_ == MOTOR_DRIVE_ID:
            # Read the message and separate
            desired_current = (message >> 32) & 0xFFFFFFFF
            desired_velocity = message & 0xFFFFFFFF
            update_velocity(desired_velocity)
            # Write the current velocity value
            tx_message = int(CURRENT_VELOCITY) << 32 + int(CURRENT_VELOCITY)
            write(VELOCITY_ID, tx_message)
            return desired_velocity, CURRENT_VELOCITY
        else:
            return CURRENT_VELOCITY, CURRENT_VELOCITY
    except ValueError:
        return CURRENT_VELOCITY, CURRENT_VELOCITY


def update_velocity(v):
    """Acts as the motor controller increasing
    the speed of the motor

    Args:
        v (float): desired velocity received by the Controls system
    """
    global CURRENT_VELOCITY
    if CURRENT_VELOCITY < v:
        CURRENT_VELOCITY += 0.5
    elif CURRENT_VELOCITY > v:
        CURRENT_VELOCITY -= 0.5
