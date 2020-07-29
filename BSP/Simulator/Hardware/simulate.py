import os
import tkinter as tk
from functools import partial

import Switches
import Contactor


def update_buttons():
    """Periodically update the display state of buttons"""
    switches = Switches.read()
    for i, button in enumerate(buttons):
        if switches & 1<<i:
            button.config(relief=tk.SUNKEN)
        else:
            button.config(relief=tk.RAISED)
    window.after(10, update_buttons)

def update_contactors():
    """Periodically update the display state of the Motor and Array Contactors"""
    global motor_status, array_status
    contactors_status = Contactor.read()
    motor_status.set(f"Motor Contactor: {contactors_status[0]}")
    array_status.set(f"Array Contactor: {contactors_status[1]}")
    window.after(30, update_contactors)

# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0], minsize=200, weight=1)
window.columnconfigure([0, 1], minsize=200, weight=1)

# Sets up frames
button_frame = tk.Frame(master=window)
button_frame_rows = [0, 1, 2, 3]
button_frame_columns = [0, 1, 2]
button_frame.rowconfigure(button_frame_rows, minsize=50, weight=1)
button_frame.columnconfigure(button_frame_columns, minsize=100, weight=1)
button_frame.grid(row=0, column=0, sticky='nsew')

display_frame = tk.Frame(master=window)
display_frame.rowconfigure([0, 1, 2], minsize=50, weight=1)
display_frame.columnconfigure([0, 1, 2], minsize=100, weight=1)
display_frame.grid(row=0, column=1, sticky='nsew')

contactor_frame = tk.LabelFrame(master=window, text="Contactors")
contactor_frame_rows = [0,1]
contactor_frame_columns = [0]
contactor_frame.rowconfigure(contactor_frame_rows, minsize=50, weight = 1)
contactor_frame.columnconfigure(contactor_frame_columns, minsize=50, weight=1)
contactor_frame.grid(row=1, column=0, sticky='nsew')

# Creates all buttons
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch, command=partial(Switches.toggle, switch))
    button.grid(row=i//max(button_frame_rows), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)

### Contactors ###
motor_status = tk.StringVar(value= 'Motor Contactor: ')
motor_ = tk.Label(master=contactor_frame, textvariable=motor_status)
motor_.grid(row = 0, column = 0, sticky='nsew')
array_status = tk.StringVar(value= 'Array Contactor: ')
array_txt = tk.Label(master=contactor_frame, textvariable=array_status)
array_txt.grid(row=1, column=0, sticky='nsew')

# Sets up periodic updates
window.after(10, update_buttons)
window.after(30, update_contactors)
window.mainloop()
