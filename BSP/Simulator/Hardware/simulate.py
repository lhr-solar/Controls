import os
import tkinter as tk

import Switches
import Pedals


def update_buttons():
    """Periodically update the display state of buttons"""
    switches = Switches.read()
    if switches & 1:
        left_turn.config(relief=tk.SUNKEN)
    else:
        left_turn.config(relief=tk.RAISED)
    if switches & 1<<1:
        right_turn.config(relief=tk.SUNKEN)
    else:
        right_turn.config(relief=tk.RAISED)
    if switches & 1<<2:
        headlight.config(relief=tk.SUNKEN)
    else:
        headlight.config(relief=tk.RAISED)
    if switches & 1<<3:
        fwd_rev.config(relief=tk.SUNKEN)
    else:
        fwd_rev.config(relief=tk.RAISED)
    if switches & 1<<4:
        hazard.config(relief=tk.SUNKEN)
    else:
        hazard.config(relief=tk.RAISED)
    if switches & 1<<5:
        crs_set.config(relief=tk.SUNKEN)
    else:
        crs_set.config(relief=tk.RAISED)
    if switches & 1<<6:
        crs_en.config(relief=tk.SUNKEN)
    else:
        crs_en.config(relief=tk.RAISED)
    if switches & 1<<7:
        regen.config(relief=tk.SUNKEN)
    else:
        regen.config(relief=tk.RAISED)
    window.after(1, update_buttons)


# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0], minsize=200, weight=1)
window.columnconfigure([0, 1, 2], minsize=200, weight=1)

# Sets up frames
button_frame = tk.Frame(master=window)
button_frame.rowconfigure([0, 1, 2], minsize=50, weight=1)
button_frame.columnconfigure([0, 1, 2], minsize=100, weight=1)
button_frame.grid(row=0, column=0, sticky='nsew')

pedal_frame = tk.Frame(master=window)
pedal_frame.rowconfigure([0], minsize=50, weight=1)
pedal_frame.columnconfigure([0, 1], minsize=100, weight=1)
pedal_frame.grid(row=0, column=1, sticky='nsew')

display_frame = tk.Frame(master=window)
display_frame.rowconfigure([0, 1, 2], minsize=50, weight=1)
display_frame.columnconfigure([0, 1, 2], minsize=100, weight=1)
display_frame.grid(row=0, column=2, sticky='nsew')

# Creates all buttons
left_turn = tk.Button(master=button_frame, text="LT", command=lambda:Switches.toggle("LT"))
left_turn.grid(row=0, column=0, sticky='nsew')
right_turn = tk.Button(master=button_frame, text="RT", command=lambda:Switches.toggle("RT"))
right_turn.grid(row=0, column=1, sticky='nsew')
headlight = tk.Button(master=button_frame, text="HDLT", command=lambda:Switches.toggle("HDLT"))
headlight.grid(row=0, column=2, sticky='nsew')
fwd_rev = tk.Button(master=button_frame, text="FWD/REV", command=lambda:Switches.toggle("FWD/REV"))
fwd_rev.grid(row=1, column=0, sticky='nsew')
hazard = tk.Button(master=button_frame, text="HZD", command=lambda:Switches.toggle("HZD"))
hazard.grid(row=1, column=1, sticky='nsew')
crs_set = tk.Button(master=button_frame, text="CRS_SET", command=lambda:Switches.toggle("CRS_SET"))
crs_set.grid(row=1, column=2, sticky='nsew')
crs_en = tk.Button(master=button_frame, text="CRS_EN", command=lambda:Switches.toggle("CRS_EN"))
crs_en.grid(row=2, column=0, sticky='nsew')
regen = tk.Button(master=button_frame, text="REGEN", command=lambda:Switches.toggle("REGEN"))
regen.grid(row=2, column=1, sticky='nsew')

# Create both pedals
accelerator = tk.Scale(pedal_frame, from_=1, to=0, resolution=-1, label='accelerator')
accelerator.grid(row=0, column=0, sticky='nsew')
brake = tk.Scale(pedal_frame, from_=1, to=0, resolution=-1, label='brake')
brake.grid(row=0, column=1, sticky='nsew')

accelerator.configure(command=lambda pos : Pedals.set_pedals(pos, brake.get()))
brake.configure(command=lambda pos : Pedals.set_pedals(accelerator.get(), pos))

# Sets up periodic updates
window.after(1, update_buttons)
window.mainloop()
