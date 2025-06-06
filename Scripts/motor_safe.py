import time
import serial
import threading

# take in parameters
port_name = "/dev/ttyUSB0" #input('Enter port name (e.g., /dev/ttyUSB0): ')

print_other_can_msgs = False

# CAN message IDs
BPS_CONTACTOR = 0x102
CONTACTOR_SENSE = 0x400

MAX_DATA_LENGTH = 8  # Maximum data length for CAN messages

# Command terminator (cairrage return)
CR = chr(13)


# establish connection
ser = serial.Serial(
    port=port_name,
    baudrate=921600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    write_timeout=0
)


print_other_can_msgs = True
# Set up the CAN interface
out = 'S4' + chr(13)  # Set CAN speed to 125Kbps
ser.write(out.encode('ascii'))

out = 'O' + chr(13)  # Open the CAN channel
ser.write(out.encode('ascii'))

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
    #print(f"[INFO] CAN msg sent: {message.strip()}") ### Uncomment this to see each message sent

def parse_motor_safe(data_hex):
    """
    Parse MOTOR_SAFE message data (0x584)
    Format:
    [1] : Motor Fault
    [0] : Motor Safe
    """
    try:
        byte0 = int(data_hex[0:2], 16)
        motor_safe = (byte0 >> 0) & 1
        motor_fault = (byte0 >> 1) & 1
        # Format the output
        result = "\n===== MOTOR_SAFE (0x584) =====\n"
        result += f"Motor Safe To Run: {'TRUE' if motor_safe == 1 else 'FALSE'}\n"
        result += f"Motor Fault: {'YES' if motor_fault == 1 else 'NONE'}\n"
        result += "============================="
        return result
    except Exception as e:
        return f"Error parsing MOTOR_SAFE data: {str(e)}"

def motor_safe_sends():
    CONTACTOR_SENSE_MSG = 0b0_0000_0000
    BPS_CONTACTOR_MSG = 0b0_0000_0000
    def thread_send_bps_contactor():
        while True:
            BPS_data = [ # Convert to little-endian bytes
                BPS_CONTACTOR_MSG & 0xFF,
                (BPS_CONTACTOR_MSG >> 8) & 0xFF
            ]

            send_can_message(BPS_CONTACTOR, BPS_data)
            time.sleep(.5)
    def thread_send_contactor_sense():
        while True:

            CONTACTOR_data = [ # Convert to little-endian bytes
                CONTACTOR_SENSE_MSG & 0xFF,
                (CONTACTOR_SENSE_MSG >> 8) & 0xFF
            ]

            send_can_message(CONTACTOR_SENSE, CONTACTOR_data)
            time.sleep(.5)
    def thread_rec():
        buffer = ""
        while True:
            # Read data if available
            if ser.in_waiting > 0:
                try:
                    new_data = ser.read(ser.in_waiting).decode("ascii", errors="replace")
                    buffer += new_data
                
                    # Process complete messages (assuming they end with CR)
                    while '\r' in buffer:
                        line, buffer = buffer.split('\r', 1)
                        line = line.strip()
                        
                        if not line:
                            continue
                        
                        # Check for CAN message format (t or T followed by ID)
                        if (line.startswith('t') or line.startswith('T')) and len(line) >= 4:
                            msg_id = line[1:4]
                            data_hex = line[5:]
            

                            # Check if it's MOTOR_SAFE message (0x584)
                            if msg_id == '584':
                                print("====================================")
                                print(parse_motor_safe(data_hex))
                            elif print_other_can_msgs:
                                # For other messages, just print ID and data
                                print(f"CAN ID: 0x{msg_id}, Data: {data_hex}")
                except Exception as e:
                    print(f"Error in receive thread: {str(e)}")
            
            # Small delay to prevent CPU hogging
            time.sleep(.01)
    print("\n=== Running motor safe test ===")
    thread1 = threading.Thread(target=thread_send_contactor_sense)
    thread2 = threading.Thread(target=thread_send_bps_contactor)
    thread3 = threading.Thread(target=thread_rec)

    thread1.start()
    thread2.start()
    thread3.start()

    while True:
        BPS_CONTACTOR_MSG = 7
        CONTACTOR_SENSE_MSG = 73
        print(f"[INFO] Sent BPS_CONTACTOR and CONTACTOR_SENSE: {BPS_CONTACTOR_MSG}, {CONTACTOR_SENSE_MSG}")
        time.sleep(1)



if __name__ == "__main__":
    # Keep main thread running
    try:
        print("Listening for CAN messages. Press Ctrl+C to exit.")
        while True:
            motor_safe_sends()
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")
        ser.close()