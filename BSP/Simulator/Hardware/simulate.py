import os
import tkinter as tk
from functools import partial

import Switches
import Contactor


def update_buttons():
    """Periodically update the display state of buttons"""
    switches = Switches.read()
    contactorState = Contactor.Read_Contactor()
    for i, button in enumerate(buttons):
        if switches & 1<<i:
            button.config(relief=tk.SUNKEN)
        else:
            button.config(relief=tk.RAISED)
    contactor_status = tk.Label(master=window, text="Motor Contactor:" + contactorState[0]+"\nArray Contactor:" + contactorState[1], font=("Helvetica", 8))
    contactor_status.place(x=325, y= 10)
    window.after(10, update_buttons)


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

# Creates all buttons
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch, command=partial(Switches.toggle, switch))
    button.grid(row=i//max(button_frame_rows), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)

#Creates text that displays Contactor states
contactor_status = tk.Label(master=window, text="Motor Contactor: \nArray Contactor:", font=("Helvetica", 8))
contactor_status.place(x=325, y= 10)


# Sets up periodic updates
window.after(10, update_buttons)
window.mainloop()
