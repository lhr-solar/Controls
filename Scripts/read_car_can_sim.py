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
CONTACTOR_SENSE = 0x400


MAX_DATA_LENGTH = 8  # Maximum data length for CAN messages TODO: WHAT IS THIS?

# Command terminator (cairrage return)
CR = chr(13)

#com_num = input('Enter COM number: ')
#port_name = input('Enter port name: ')
port_name = "/dev/ttyUSB0"

# Establish serial connection with specified parameters
ser = serial.Serial(
    port=port_name,
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

def test_contactor_sense():
    ACTUAL_MOTOR_SENSE_BIT = 0b1
    EXPECTED_MOTOR_SENSE_BIT = 0b10
    MOTOR_SENSE_FAULT_BIT = 0b100
    ACTUAL_MOTOR_PRECHARGE_SENSE_BIT = 0b1000
    EXPECTED_MOTOR_PRECHARGE_SENSE_BIT = 0b1_0000
    MOTOR_PRECHARGE_SENSE_FAULT_BIT = 0b10_0000
    ACTUAL_ARRAY_PRECHARGE_SENSE_BIT = 0b100_0000
    EXPECTED_ARRAY_PRECHARGE_SENSE_BIT = 0b1000_0000
    ARRAY_PRECHARGE_SENSE_FAULT_BIT = 0b1_0000_0000

    states = [
        [0], # All off
        [ACTUAL_MOTOR_SENSE_BIT], # Motor sense on
        [ACTUAL_MOTOR_PRECHARGE_SENSE_BIT], # Motor precharge on
        [ACTUAL_ARRAY_PRECHARGE_SENSE_BIT], # Array precharge on
        [ACTUAL_MOTOR_SENSE_BIT | ACTUAL_MOTOR_PRECHARGE_SENSE_BIT | ACTUAL_ARRAY_PRECHARGE_SENSE_BIT], # All on
        [MOTOR_SENSE_FAULT_BIT], # Motor sense fault
        [MOTOR_PRECHARGE_SENSE_FAULT_BIT], # Motor precharge sense fault
        [ARRAY_PRECHARGE_SENSE_FAULT_BIT], # Array precharge sense fault
        [ARRAY_PRECHARGE_SENSE_FAULT_BIT | ACTUAL_ARRAY_PRECHARGE_SENSE_BIT] # Array precharge sense fault plus contactor on
    ]

    for state in states:
        print(state[0])
        data = [ # Convert to little-endian bytes
            state[0] & 0xFF,
            (state[0] >> 8) & 0xFF
        ]
        send_can_message(CONTACTOR_SENSE, data)
        time.sleep(1)
        print(f"[INFO] Sent contactor sense: {state}")
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



# Simulate BPS + Contactor board
# Send BPS Contactor and Contactor Sense (at least) every 500 ms to avoid tripping the watchdog
# Intermittently send other messages
def test_sim():
    CONTACTOR_SENSE_MSG = 0b0_0000_0000
    

    def thread_send_contactor_sense():
        while True:
            send_can_message(CONTACTOR_SENSE, [CONTACTOR_SENSE_MSG])
            time.sleep(.4)

    def thread_send_bps_contactor():
        while True:
            send_can_message(BPS_CONTACTOR, [CONTACTOR_SENSE_MSG])
            time.sleep(.4)
           
    printf("\n=== Running simulation test ===")

    

    thread1 = threading.Thread(target=thread_send_contactor_sense)
    thread2 = threading.Thread(target=thread_send_bps_contactor)

    # Arrays of messages to cycle through

    # While loop every 
   
############
def send(id, msg, hz, cnt):
    int i = 0
    while (i < cnt):
        send_can_message(id, msg)
        time.sleep(1.0/hz)
    
# threading.Thread(target=rec).start()
for ind in range(len(num_messages)):
    threading.Thread(target=send, args=num_messages[ind]).start()
######################



# Main test sequence function
def run_all_tests():
    # Configure SLCAN interface now done in main function
    configure_slcan()
    
    print("\n=== Running BPS Contactor Tests ===")
    test_bps_contactor()

    print("\n=== Running State of Charge Tests ===")
    test_state_of_charge()

    print("\n=== Running Supplemental Voltage Tests ===")
    test_supplemental_voltage()

    print("\n=== Running Contactor Sense Tests ===")
    #TODO

    print("\n=== Running BPS Trip Test ===")
    print("[WARN] This will trigger a fault condition!")
    if input("Continue? (y/n): ").lower() == 'y':
        test_bps_trip()

# Continuous message sending function
# def send_continuous():
#     while True:
#         msg_can_id = int(input("Enter CAN message ID (e.g., 0x10D, 0x10E, etc.): "))
#         msg_data = input("Enter message data (e.g., 0x00, 0x01, etc.): ")
#         if len(msg_data) > MAX_DATA_LENGTH:
#             print("[ERR] Invalid data length")
#             continue
#         send_can_message(msg_can_id, msg_data)
#         time.sleep(1.0)


if __name__ == "__main__":
    print("\n=== ReadCarCAN Test ===")
    print("Select test mode:")
    print("1. Run all tests in sequence")
    # print("2. Send particular messages")
    print("3. Send BPS trip")
    print("4. Test Supplemental voltage")
    print("5. Test Contactor Sense")
    choice = input("Enter choice (1, 2, 3, 4, 5): ")
    configure_slcan()
    
    if choice == '1':
        run_all_tests()
    # elif choice == '2':
    #     configure_slcan()
    #     send_continuous()
    elif choice == '3':
        while True:
            test_bps_trip()
    elif choice == '4':
        test_supplemental_voltage()
    elif choice == '5':
        test_contactor_sense()
    elif choice == '6':
        test_sim()
    else:
        print("[ERR] Invalid choice")
