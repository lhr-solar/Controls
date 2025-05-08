import time
import serial
import threading
import struct
import random as rand

# CAN message IDs
BPS_TRIP = 0x002
# BPS_ALL_CLEAR = 0x101
BPS_CONTACTOR = 0x102
CURRENT_DATA = 0x103
VOLTAGE_SUMMARY = 0x10D
TEMPERATURE_SUMMARY = 0x10E
STATE_OF_CHARGE = 0x106
SUPPLEMENTAL_VOLTAGE = 0x10B


MAX_DATA_LENGTH = 8  # Maximum data length for CAN messages TODO: WHAT IS THIS?

# Command terminator (cairrage return)
CR = chr(13)

com_num = input('Enter COM number: ')

# Establish serial connection with specified parameters
ser = serial.Serial(
    port='COM' + com_num,
    baudrate=921600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    write_timeout=0
)

# Configure SLCAN interface for loopback testing
def configure_slcan():
    # Set bitrate to 125Kbit/s
    out = 'S4' + CR
    ser.write(out.encode('ascii'))
    time.sleep(0.1)
    
    # # Set loopback mode
    # out = 'L1' + CR
    # ser.write(out.encode('ascii'))
    # time.sleep(0.1)
    
    # Open CAN channel
    out = 'O' + CR
    ser.write(out.encode('ascii'))
    time.sleep(0.1)
    print("[INFO] SLCAN configured")

# Function to send a CAN message using SLCAN protocol
def send_can_message(id, data):
    data_len = len(data)
    
    # Convert ID to hex string (8 chars for extended frame)
    id_str = format(id, '03X')
    prefix = 'T'
    
    # Format SLCAN message: T[ID][data length][data in hex]
    message = prefix + id_str + format(data_len, '01X')
    
    # Add each data byte as a 2-character hex string
    for byte in data:
        message += format(byte, '02X')
        
    message += CR
    
    ser.write(message.encode('ascii'))
    print(f"[INFO] CAN msg sent: {message.strip()}")

# Test function for BPS contactors states
def test_bps_contactor():
    HV_ARRAY_CONTACTOR_BIT = 0b001
    HV_MINUS_CONTACTOR_BIT = 0b010
    HV_PLUS_CONTACTOR_BIT  = 0b100
    
    # Test all possible combinations of contactor states
    states = [
        [0],  # All contactors off
        [HV_ARRAY_CONTACTOR_BIT],  # Array contactor on
        [HV_MINUS_CONTACTOR_BIT],  # HV- contactor on
        [HV_PLUS_CONTACTOR_BIT],  # HV+ contactor on
        [HV_ARRAY_CONTACTOR_BIT | HV_MINUS_CONTACTOR_BIT],  # Array and HV- on
        [HV_ARRAY_CONTACTOR_BIT | HV_PLUS_CONTACTOR_BIT],  # Array and HV+ on
        [HV_MINUS_CONTACTOR_BIT | HV_PLUS_CONTACTOR_BIT],  # HV- and HV+ on
        [HV_ARRAY_CONTACTOR_BIT | HV_MINUS_CONTACTOR_BIT | HV_PLUS_CONTACTOR_BIT]  # All on
    ]
    
    for state in states:
        send_can_message(BPS_CONTACTOR, state)
        time.sleep(1)
        print(f"[INFO] Sent BPS contactor state: {state[0]:b} (binary)")
        input("Press Enter to continue...")

def test_supplemental_voltage():
    voltages = [0, 10000, 20000, 30000]
    
    for voltage in voltages:
        data = [voltage & 0xFF, (voltage >> 8) & 0xFF]
        send_can_message(SUPPLEMENTAL_VOLTAGE, data)
        time.sleep(1)
        print(f"[INFO] Sent supplemental voltage: {voltage} mV")
        input("Press Enter to continue...")

def test_state_of_charge():
    SOC_SCALER = 1000000
    
    # Test a range of state of charge values (0-100%, scaled by SOC_SCALER)
    soc_values = [0, 25000000, 50000000, 75000000, 100000000]
    
    for soc in soc_values:
        # Convert 32-bit value to little-endian bytes
        data = [
            soc & 0xFF,
            (soc >> 8) & 0xFF,
            (soc >> 16) & 0xFF,
            (soc >> 24) & 0xFF
        ]
        send_can_message(STATE_OF_CHARGE, data)
        time.sleep(1)
        print(f"[INFO] Sent state of charge: {soc/SOC_SCALER}%")
        input("Press Enter to continue...")

def test_bps_trip():
    BPS_TRIP_MESSAGE = 0x01 # BPS trip message
    
    send_can_message(BPS_TRIP, [BPS_TRIP_MESSAGE])
    print("[INFO] Sent BPS trip message")
    input("Press Enter to continue...")

# Main test sequence function
def run_all_tests():
    # Configure SLCAN interface
    configure_slcan()
    
    print("\n=== Running BPS Contactor Tests ===")
    test_bps_contactor()
    
    print("\n=== Running BPS Trip Test ===")
    print("[WARN] This will trigger a fault condition!")
    if input("Continue? (y/n): ").lower() == 'y':
        test_bps_trip()

# Continuous message sending function
def send_continuous():
    while True:
        msg_can_id = int(input("Enter CAN message ID (e.g., 0x10D, 0x10E, etc.): "))
        msg_data = input("Enter message data (e.g., 0x00, 0x01, etc.): ")
        if len(msg_data) > MAX_DATA_LENGTH:
            print("[ERR] Invalid data length")
            continue
        send_can_message(msg_can_id, msg_data)
        time.sleep(1.0)


if __name__ == "__main__":
    print("\n=== ReadCarCAN Loopback Test ===")
    print("Select test mode:")
    print("1. Run all tests in sequence")
    print("2. Send particular messages")
    choice = input("Enter choice (1 or 2): ")
    
    if choice == '1':
        run_all_tests()
    elif choice == '2':
        configure_slcan()
        send_continuous()
    else:
        print("[ERR] Invalid choice")
