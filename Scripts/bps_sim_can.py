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
    write_timeout=0
)

out = 'S4' + chr(13)
ser.write(out.encode('ascii'))

out = 'O' + chr(13)
ser.write(out.encode('ascii'))

# def rec():
#     global ign_val
#     while 1:
#         inp = ''
#         while ser.in_waiting > 0:
#             inp += ser.read(1).decode("ascii")
        
#         # print feedback if available
#         if inp[1:4] == '581':
#             ign_val = (int(inp[11:13]) >= 4)

#         # print('EN HVARR CONTACTOR: ' + str(ign_val))

# messages = ['10D460EA0000', '10E4A0860100', '103450C30000'] #60V, 100degC, 50A
import random as rand
num_messages = [('10D4', 100000, rand.randint(30, 60)), ('10E4', 100000, rand.randint(30, 60)), ('1034', 100000, rand.randint(30, 60)), ('1064',100000000, rand.randint(30, 60)), ('10B4', 100000, rand.randint(30, 60))]

CR = chr(13)

import struct

def send(msg, max, hz):
    while 1:
        out = 'T' + msg + struct.pack('<I', rand.randint(0, max)).hex().upper() + CR
        ser.write(out.encode('ascii'))
        print(out)
        # time.sleep(1)
        time.sleep(1.0/hz)
        
# threading.Thread(target=rec).start()
for ind in range(len(num_messages)):
    threading.Thread(target=send, args=num_messages[ind]).start()
