# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl
import CAN
import math

# Path of file
file = "BSP/Simulator/Hardware/Data/CAN.csv"


def sendTouC():
    """Simulates motors sending CAN messages back to the uC 
        
        Returns:
        CAN ID of the last message sent
    """

    currentID = CarState.motor_info[CarState.infoIdx]

    if currentID == CarState.VELOCITY_ID:
        tx_message = int(CarState.CURRENT_VELOCITY)
        tx_message = (tx_message << 32) + int((60 * CarState.CURRENT_VELOCITY)/(math.pi * 2 * CarState.radius))
    
    elif currentID == CarState.MOTOR_BUS_ID:
        tx_message = int(CarState.ABSOLUTE_CURRENT)
        tx_message = (tx_message << 32) + int(CarState.BUS_VOLTAGE)
    
    elif currentID == CarState.MOTOR_PHASE_CURRENT_ID:
        tx_message = int(CarState.PHASE_C_CUR)
        tx_message = (tx_message << 32) + int(CarState.PHASE_B_CUR)
    
    elif currentID == CarState.MOTOR_VOLTAGE_VECTOR_ID:
        tx_message = int(CarState.V_REAL)
        tx_message = (tx_message << 32) + int(CarState.V_IMAGINE)

    elif currentID == CarState.MOTOR_CURRENT_VECTOR_ID:
        tx_message = int(CarState.I_REAL)
        tx_message = (tx_message << 32) + int(CarState.I_IMAGINE)

    elif currentID == CarState.MOTOR_BACKEMF_ID:
        tx_message = int(CarState.REAL_COMP)
        tx_message = (tx_message << 32) + int(CarState.NEUTRAL_MOTOR)

    elif currentID == CarState.MOTOR_TEMP_ID:
        tx_message = int(CarState.PHASEC_TEMP)
        tx_message = (tx_message << 32) + int(CarState.INTERNAL_TEMP)

    
    write(currentID, tx_message)
    CarState.infoIdx = (CarState.infoIdx + 1) %  7   #increment index
    return currentID


def read():
    """Reads CAN2 bus

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
    message = [0, 0] if message == [] else message[1]
    canId = sendTouC()
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
    try:
        id_, message = read()
        id_ = int(id_, 16)
        message = int(message, 16)
        if id_ == CarState.MOTOR_POWER_ID:
            # Read the message and separate
            desired_current = (message >> 32) & 0xFFFFFFFF
            CarState.MAX_CURRENT = CarState.ABSOLUTE_CURRENT * (desired_current/100.0)
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
    try:      
        id_, message, _ = read()
        id_ = int(id_, 16)
        message = int(message, 16)
        if id_ == CarState.MOTOR_DRIVE_ID:
            # Read the message and separate
            desired_current = (message >> 32) & 0xFFFFFFFF
            desired_velocity = message & 0xFFFFFFFF
            toggle_torque(desired_velocity)     #enable torque control mode if desired_velocity is an extremely large number
            #update max available current value
            if CarState.mode != 1:
                CarState.CURRENT_SETPOINT = CarState.MAX_CURRENT * (desired_current/100.0)
            update_velocity(desired_velocity)
            # Write the current velocity value
            tx_message = int(CarState.CURRENT_VELOCITY) << 32 + int(CarState.CURRENT_VELOCITY)
            write(CarState.VELOCITY_ID, tx_message)
            return desired_velocity, CarState.CURRENT_VELOCITY
        else:
            return CarState.CURRENT_VELOCITY, CarState.CURRENT_VELOCITY
    except ValueError:
        return CarState.CURRENT_VELOCITY, CarState.CURRENT_VELOCITY



def toggle_torque(velocity):
    if velocity > 1000:
        CarState.mode = 1
    else:
        CarState.mode = 0



def torque_control(pedalPercent):
    #following code will only execute if motor is in torque control mode
    if CarState.mode == 1:
        CarState.CURRENT_SETPOINT = pedalPercent * CarState.MAX_CURRENT    #param will be a value from 0.0 to 1.0
        CarState.velocity_increase = CarState.CURRENT_SETPOINT     #update rate


def update_velocity(v):
    """Acts as the motor controller increasing
    the speed of the motor

    Args:
        v (float): desired velocity received by the Controls system
    """

    if CarState.CURRENT_VELOCITY < v:
        CarState.CURRENT_VELOCITY += CarState.velocity_increase
    elif CarState.CURRENT_VELOCITY > v:
        CarState.CURRENT_VELOCITY -= 0.5


# Class that holds the global constants and variables of the car state
class CarState:
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

    motor_info = [MOTOR_BUS_ID, VELOCITY_ID, MOTOR_PHASE_CURRENT_ID, MOTOR_VOLTAGE_VECTOR_ID, MOTOR_CURRENT_VECTOR_ID, MOTOR_BACKEMF_ID, MOTOR_TEMP_ID]

    # Index
    infoIdx = 0 

    radius = 21.67  # in inches

    # State values
    CURRENT_VELOCITY = 0

    BUS_VOLTAGE = 5

    # Phase Currents
    PHASE_C_CUR = 1 
    PHASE_B_CUR = 1

    # Voltage Vectors
    V_REAL = 0
    V_IMAGINE = 0

    #Current Vectors
    I_REAL = 0
    I_IMAGINE = 0

    #Motor BackEMF components
    REAL_COMP = 0
    NEUTRAL_MOTOR = 0

    #Temperature
    PHASEC_TEMP = 0
    INTERNAL_TEMP = 0

    mode = 0    # 0 = Velocity control mode, 1 = Torque control mode

    velocity_increase = 0.5 #how much CURRENT_VELOCITY is increased by in update_velocity()

    CURRENT_SETPOINT = 0.0 #set by user via MOTOR_DRIVE commands

    MAX_CURRENT = 1.0 #max available current, this is a percent of the Absolute bus current

    ABSOLUTE_CURRENT = 5.0  #physical limitation
    


