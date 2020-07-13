import os
import tkinter as tk
from functools import partial

import Switches
import Display


def update_buttons():
    """Periodically update the display state of buttons"""
    switches = Switches.read()
    for i, button in enumerate(buttons):
        if switches & 1<<i:
            button.config(relief=tk.SUNKEN)
        else:
            button.config(relief=tk.RAISED)
    window.after(10, update_buttons)


def update_display():
    global display_text
    display = Display.read()
    for i, text in enumerate(display.keys()):
        display_text[i].set(f"{text}: {display[text]}")
    window.after(50, update_display)

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
display_frame_rows = [0, 1, 2, 3, 4]
display_frame_columns = [0]
display_frame.rowconfigure(display_frame_rows, minsize=50, weight=1)
display_frame.columnconfigure(display_frame_columns, minsize=100, weight=1)
display_frame.grid(row=0, column=1, sticky='nsew')

### Switches ###
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch, command=partial(Switches.toggle, switch))
    button.grid(row=i//len(button_frame_columns), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)

### Display ###
display_text = list()
for i, label in enumerate(Display.get_display()):
    text = tk.StringVar(value=label)
    display_text.append(text)
    cell = tk.Label(master=display_frame, textvariable=text)
    cell.grid(row=i//len(display_frame_columns), column=i%len(display_frame_columns), sticky='nsew')


# Sets up periodic updates
window.after(10, update_buttons)
window.after(50, update_display)
window.mainloop()
