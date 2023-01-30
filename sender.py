import serial
import time
'''
Steps:
- Run com0com (zip in root) as administrator on Windows and create COM ports, say 7 & 9 (install PortName=COM7 PortName=COM9, uninstall)
- Start the VM and redirect COM 9 on Windows to COM 1 on Ubuntu
- Run the renode machine with the display sim
- Run Putty on Windows to read on COM7 at 115200 (If this is not done, then Nextion will do nothing for an unkown reason)
- Start the python script in another terminal (redirects output from /tmp/uart to ttyS0 (COM1 from Virtual Box perspective))
- Test for input on Putty and then stop the python script and disconnect Putty
- Open the Nextion debugger and connect to COM7 at 115200 and run the python script again and see the data coming in
'''

guest = serial.Serial('/tmp/uart')
host = serial.Serial('/dev/ttyS0', 115200)

host.write(b'hello\xff\xff\xff')
time.sleep(5)

while True:
    renode = guest.read()
    # print(renode)
    host.write(renode)

guest.close()
host.close()