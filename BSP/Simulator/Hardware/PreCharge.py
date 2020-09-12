# import time
# import tkinter as tk
import os
import csv
import fcntl
 
file = "BSP/Simulator/Hardware/Data/PreCharge.csv";
def read():
    os.makedirs(os.path.dirname("BSP/Simulator/Hardware/Data/PreCharge.csv"), exist_ok = True);
    if not os.path.exists("BSP/Simulator/Hardware/Data/PreCharge.csv"):
        open(file, 'w')
        file.close()
        
    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX);
        try:
            csvreader = csv.reader(csvfile);
            charges = int(next(csvreader));
        except StopIteration:
            charges = 0;
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN);
        chargearr = int[2];
        chargearr[0] = charges % 1;
        chargearr[1] = charges /2;
        return chargearr
            
    

# window = tk.Tk();
# frame = tk.Frame(master = window, relief = tk.RAISED, borderwidth = 2);
# frame.pack();

# data = read();
# time.sleep(0.2);
# d = int(data, base = 2);
# boardOneReady = d & 1;
# boardTwoReady = (d >> 1) & 1;
# if(boardOneReady == 1):
#     label = tk.Label(master = frame,text = "READY");
# else:
#     label = tk.Label(master = frame,text = "NOT READY");
# label.pack();
# if(boardTwoReady == 1):
#     labelTwo = tk.Label(master = frame,text = "READY");
# else:
#     labelTwo = tk.Label(master = frame,text = "NOT READY");
# labelTwo.pack();
# window.mainloop();
    
    