import time
import serial
import threading

# CAN configuration
PORT = "/dev/ttyUSB1"
CAN_ID = "241"  # Motor status message ID from your code
BITRATE_CMD = 'S5'  # 250 kbps (matches your motor controller setup)

# Fault bit descriptions from Motor_FaultBitmap
FAULT_MAP = {
    0: "Hardware Overcurrent",
    1: "Software Overcurrent",
    2: "DC Bus Overvoltage",
    3: "Hall Sensor Fault",
    4: "Watchdog Last Reset",
    5: "Config Read Error", 
    6: "Undervolt Lockout",
    7: "Desat Fault",
    8: "Motor Overspeed"
}

def send_fault(ser, fault_bits):
    """Send CAN frame with specified fault bits"""
    # Create 16-bit fault value
    fault_value = sum(1 << bit for bit in fault_bits)
    
    # Convert to little-endian bytes
    fault_bytes = fault_value.to_bytes(2, 'little')
    
    # Construct 8-byte payload (bytes 4-5 contain faults)
    payload = bytes([0]*4) + fault_bytes + bytes([0]*2)
    
    # Create CAN message string
    can_msg = f't{CAN_ID}{8}{payload.hex()}\r'
    
    # Send message
    ser.write(can_msg.encode('ascii'))
    print(f"Sent: {can_msg.strip()}")

def user_input_handler(ser):
    """Handle user input for fault configuration"""
    print("\nAvailable fault bits:")
    for bit, desc in FAULT_MAP.items():
        print(f"{bit}: {desc}")
    
    while True:
        try:
            user_input = input("\nEnter fault bits to trigger (comma-separated 0-8) or 'q' to quit: ")
            if user_input.lower() == 'q':
                return
            
            bits = [int(b) for b in user_input.split(',') if b.strip().isdigit()]
            invalid_bits = [b for b in bits if b not in FAULT_MAP]
            
            if invalid_bits:
                print(f"Invalid bits: {invalid_bits}. Valid range 0-8")
                continue
                
            send_fault(ser, bits)
            
        except Exception as e:
            print(f"Input error: {str(e)}")

def main():
    # Configure serial connection
    ser = serial.Serial(
        port=PORT,
        baudrate=921600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        write_timeout=1
    )
    
    # Initialize CAN interface
    ser.write(f'{BITRATE_CMD}\r'.encode('ascii'))
    ser.write('O\r'.encode('ascii'))  # Open channel
    time.sleep(0.1)
    
    # Start user input thread
    input_thread = threading.Thread(target=user_input_handler, args=(ser,))
    input_thread.start()
    
    try:
        while input_thread.is_alive():
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
