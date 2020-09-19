import csv
import os
import fcntl
import CAN

# Path of file
file = "BSP/Simulator/Hardware/Data/CAN.csv"


# Relevant IDs
MOTOR_DRIVE_ID = 0x221
VELOCITY_ID = 0x243
MOTOR_POWER_ID = 0x222
MOTOR_BUS_ID = 0x242
MOTOR_PHASE_CURRENT_ID = 0x244
MOTOR_VOLTAGE_VECTOR_ID = 0x245
MOTOR_CURRENT_VECTOR_ID = 0x246
MOTOR_BACKEMF_ID = 0x247
MOTOR_TEMP_ID = 0x24B

# State values
CURRENT_VELOCITY = 0

mode = 0    # 0 = Velocity control mode, 1 = Torque control mode

velocity_increase = 0.5 #how much CURRENT_VELOCITY is increased by in update_velocity()


CURRENT_SETPOINT = 0.0 #set by user via MOTOR_DRIVE commands

MAX_CURRENT = 1.0 #max available current, this is a percent of the Absolute bus current

ABSOLUTE_CURRENT = 5.0  #physical limitation

def read():
    """Reads CAN2 bus

    Returns:
        list: [ID, Message, Length]
    """
    # Creates file if it doesn't exist
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        open(file, 'w')
        file.close()
    message = []
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            message.append(row)
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)
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
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(CAN1)
        csvwriter.writerow([hex(id_), hex(message), len(hex(message)[2:])//2])
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)

def confirm_power():
    global ABSOLUTE_CURRENT, MAX_CURRENT
    try:
        id_, message = read()
        id_ = int(id_, 16)
        message = int(message, 16)
        if id_ == MOTOR_POWER:
            # Read the message and separate
            desired_current = (message >> 32) & 0xFFFFFFFF
            MAX_CURRENT = ABSOLUTE_CURRENT * (desired_current/100.0)
    except ValueError:
        pass

            

def confirm_drive():
    """Acts as the motor controller confirming
    that the proper message is received periodically.
    If the message is received, the motor speed is changed
    accordingly. If it is not received, the motor speed stays constant.

    Returns:
        tuple: desired and current velocities to display
    """
    global MAX_CURRENT, CURRENT_SETPOINT
    try:      
        id_, message, _ = read()
        id_ = int(id_, 16)
        message = int(message, 16)
        if id_ == MOTOR_DRIVE_ID:
            # Read the message and separate
            desired_current = (message >> 32) & 0xFFFFFFFF
            desired_velocity = message & 0xFFFFFFFF
            toggle_torque(desired_velocity)     #enable torque control mode if desired_velocity is an extremely large number
            #update max available current value
            if mode != 1:
                CURRENT_SETPOINT = MAX_CURRENT * (desired_current/100.0)
            update_velocity(desired_velocity)
            # Write the current velocity value
            tx_message = int(CURRENT_VELOCITY) << 32 + int(CURRENT_VELOCITY)
            write(VELOCITY_ID, tx_message)
            return desired_velocity, CURRENT_VELOCITY
        else:
            return CURRENT_VELOCITY, CURRENT_VELOCITY
    except ValueError:
        return CURRENT_VELOCITY, CURRENT_VELOCITY

def toggle_torque(velocity):
    global mode
    if velocity > 1000:
        mode = 1
    else:
        mode = 0

def torque_control(pedalPercent):
    global MAX_CURRENT, velocity_increase
    #following code will only execute if motor is in torque control mode
    if mode == 1:
        CURRENT_SETPOINT = pedalPercent * MAX_CURRENT    #param will be a value from 0.0 to 1.0
        velocity_increase = CURRENT_SETPOINT     #update rate
        #print("torque mode on, current set point is " + str(MAX_CURRENT) + "A")


def update_velocity(v):
    """Acts as the motor controller increasing
    the speed of the motor

    Args:
        v (float): desired velocity received by the Controls system
    """
    global CURRENT_VELOCITY, velocity_increase
    if CURRENT_VELOCITY < v:
        CURRENT_VELOCITY += velocity_increase
    elif CURRENT_VELOCITY > v:
        CURRENT_VELOCITY -= 0.5
