import time
import serial
import threading

# take in parameters
port_name = "/dev/ttyUSB0" #input('Enter port name (e.g., /dev/ttyUSB0): ')

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

def parse_io_state(data_hex):
    """
    Parse IO_STATE message data (0x581)
    Format:
    [23:16] : Switch Bitmap
    [15:8] : Brake Pedal 
    [7:0] : Accel Pedal
    """
    # Convert hex string to bytes
    if len(data_hex) < 6:  # Need at least 6 hex chars for 3 bytes
        return "Insufficient data length"
    
    try:
        # Extract the three bytes (assuming little-endian format)
        accel_pedal = int(data_hex[0:2], 16)    # [7:0]
        brake_pedal = int(data_hex[2:4], 16)    # [15:8]
        switch_bitmap = int(data_hex[4:6], 16)  # [23:16]
        
        # Parse switch bitmap
        switches = ["BRAKELIGHT", "CRUZ_ST", "CRUZ_EN", "REV_SW", "FOR_SW", "REGEN_SW", "IGN_2_Motor", "IGN_1_Array"]
        switch_states = []
        for i, name in enumerate(switches):
            bit_pos = i  # MSB first as per the description
            state = (switch_bitmap >> (len(switches) - i - 1)) & 1
            switch_states.append(f"{name}: {'ON' if state == 1 else 'OFF'}")
        
        # Format the output
        result = "\n===== IO_STATE (0x581) =====\n"
        result += f"Switch Bitmap: 0b{switch_bitmap:08b}\n"
        result += "Switch Status:\n"
        for state in switch_states:
            result += f"  {state}\n"
        result += f"Brake Pedal: {brake_pedal}\n"
        result += f"Accel Pedal: {accel_pedal}\n"
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
                        if msg_id == '581':
                            parsed_data = parse_io_state(data_hex)
                            print(parsed_data)
                        else:
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
