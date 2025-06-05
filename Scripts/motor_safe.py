import time
import serial
import threading

# take in parameters
port_name = "/dev/ttyUSB0" #input('Enter port name (e.g., /dev/ttyUSB0): ')

print_other_can_msgs = False

# establish connection
ser = serial.Serial(
    port=port_name,
    baudrate=921600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    write_timeout=0
)

# Set up the CAN interface
out = 'S4' + chr(13)  # Set CAN speed to 500Kbps
ser.write(out.encode('ascii'))

out = 'O' + chr(13)  # Open the CAN channel
ser.write(out.encode('ascii'))

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

def send():
    while True:
        out = 'M' + chr(13)

def recv():
    """Receive and parse CAN messages"""
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

                        # Check if it's a motor safe message (0x584)
                        if msg_id == '584':
                            parsed_data = parse_motor_safe(data_hex)
                            print(parsed_data)
            except Exception as e:
                print(f"Error in receive thread: {str(e)}")
    
        # Small delay to prevent CPU hogging
        time.sleep(0.001)

threading.Thread(target=recv, daemon=True).start()
threading.Thread(target=send, daemon=True).start()


# Keep main thread running
try:
    print("Listening for CAN messages. Press Ctrl+C to exit.")
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting...")
    ser.close()