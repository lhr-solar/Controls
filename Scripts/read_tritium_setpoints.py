import time
import serial
import threading
import struct 

# hello this is devin (notably not controls) and i janked this together from read io soooo if it dont work, im sorry

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

# ok so theres two messages, drive and power


def parse_power_state(data_hex):
    """
    Parse Tritium Power message data (0x222)
    Format:
    [32:63] : Power
    """
    # Convert hex string to bytes
    if len(data_hex) < 16:  # Need at least 16 hex chars for 8 bytes
        return "Insufficient data length"
    
    try:
        # Extract the last 4 bytes (assuming little-endian format)
        raw_bytes = bytes.fromhex(data_hex[8:16])  # get 4 bytes
        power = struct.unpack('<f', raw_bytes)[0]


        # Format the output
        result = "\n===== POWER (0x222) =====\n"
        result += f"Power: {power:.4f}\n" # 4 decimals?
        result += "============================="
        
        return result
    except Exception as e:
        return f"Error parsing IO_STATE data: {str(e)}"



def parse_drive_state(data_hex):
    """
    Parse Tritium Drive message data (0x221)
    Format:
    [0:31] : Velocity Setpoint
    [32:63] : Current Setpoint
    """
    # Convert hex string to bytes
    if len(data_hex) < 16:  # Need at least 16 hex chars for 8 bytes
        return "Insufficient data length"
    
    try:
        # Extract the first 4 bytes (assuming little-endian format)
        
        raw_bytes = bytes.fromhex(data_hex[0:8])  # get 4 bytes
        velocity = struct.unpack('<f', raw_bytes)[0]

        # Extract the last 4 bytes (assuming little-endian format)
        raw_bytes = bytes.fromhex(data_hex[8:16])  # get 4 bytes
        current = struct.unpack('<f', raw_bytes)[0]


        # Format the output
        result = "\n===== IO_STATE (0x581) =====\n"
        result += f"Velocity SP: {velocity:.4f}\n" # 4 decimals?
        result += f"Current SP: {current:.4f}\n" 
        result += "============================="
        
        return result
    except Exception as e:
        return f"Error parsing IO_STATE data: {str(e)}"

def rec():
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
                        
                        # Check if it's IO_STATE message (0x581)
                        if msg_id == '222':
                            parsed_data = parse_power_state(data_hex)
                            print(parsed_data)
                        elif msg_id == '221':   
                            parsed_data = parse_drive_state(data_hex)
                            print(parsed_data) 
                        elif print_other_can_msgs:
                            # For other messages, just print ID and data
                            print(f"CAN ID: 0x{msg_id}, Data: {data_hex}")
            except Exception as e:
                print(f"Error in receive thread: {str(e)}")
        
        # Small delay to prevent CPU hogging
        time.sleep(0.01)

# Start the receive thread
threading.Thread(target=rec, daemon=True).start()

# Keep main thread running
try:
    print("Listening for CAN messages. Press Ctrl+C to exit.")
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting...")
    ser.close()
