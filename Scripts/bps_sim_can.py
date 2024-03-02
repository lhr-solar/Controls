import time
import serial
import threading

# take in parameters
com_num = input('Enter COM number: ')
# baudrate = input('Enter baudrate (921600 default): ')
# baudrate = 921600 if baudrate == '' else int(baudrate)

# establish connection
ser = serial.Serial(
    port='COM' + com_num,
    baudrate=921600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
)

out = 'S4' + chr(13)
ser.write(out.encode('ascii'))

out = 'O' + chr(13)
ser.write(out.encode('ascii'))

ign_val = False

# act as terminal

def rec():
    global ign_val
    while 1:
        inp = ''
        while ser.in_waiting > 0:
            inp += ser.read(1).decode("ascii")
        
        # print feedback if available
        if inp[1:4] == '581':
            ign_val = (int(inp[11:13]) >= 4)

        # print('EN HVARR CONTACTOR: ' + str(ign_val))

def send():
    global ign_val
    while 1:
        out = 'T1021' + ('07' if ign_val else '06') + chr(13)
        ser.write(out.encode('ascii'))

        print('SENDING ' + str(ign_val))
        time.sleep(0.25)
    
threading.Thread(target=rec).start()
threading.Thread(target=send).start()
