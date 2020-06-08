import os
import tkinter as tk

import Switches


def create_buttons():
    left_turn = tk.Button(master=window, text="LT", command=lambda:Switches.toggle("LT"))
    left_turn.grid(row=0, column=0, sticky='nsew')
    right_turn = tk.Button(master=window, text="RT", command=lambda:Switches.toggle("RT"))
    right_turn.grid(row=0, column=1, sticky='nsew')
    headlight = tk.Button(master=window, text="HDLT", command=lambda:Switches.toggle("HDLT"))
    headlight.grid(row=0, column=2, sticky='nsew')
    fwd_rev = tk.Button(master=window, text="FWD/REV", command=lambda:Switches.toggle("FWD/REV"))
    fwd_rev.grid(row=1, column=0, sticky='nsew')
    hazard = tk.Button(master=window, text="HZD", command=lambda:Switches.toggle("HZD"))
    hazard.grid(row=1, column=1, sticky='nsew')
    crs_set = tk.Button(master=window, text="CRS_SET", command=lambda:Switches.toggle("CRS_SET"))
    crs_set.grid(row=1, column=2, sticky='nsew')
    crs_en = tk.Button(master=window, text="CRS_EN", command=lambda:Switches.toggle("CRS_EN"))
    crs_en.grid(row=2, column=0, sticky='nsew')
    regen = tk.Button(master=window, text="REGEN", command=lambda:Switches.toggle("REGEN"))
    regen.grid(row=2, column=1, sticky='nsew')


# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0, 1, 2], minsize=50, weight=1)
window.columnconfigure([0, 1, 2], minsize=100, weight=1)

create_buttons()

window.mainloop()
