import os
import tkinter as tk
from functools import partial

import Switches
import Pedals
import Display
import CAN
import MotorController


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
    """Periodically update the display state of display"""
    global display_text
    display = Display.read()
    for i, text in enumerate(display.keys()):
        display_text[i].set(f"{text}: {display[text]}")
    window.after(50, update_display)


def update_CAN():
    """Periodically update the display state of the CAN bus"""
    global id_text, message_text
    can = CAN.read()
    id_text.set(f"ID: {can[0]}")
    message_text.set(f"Message: {can[1]}")
    window.after(20, update_CAN)


def update_motor():
    """Periodically update the velocity and display of the motor"""
    global desired_velocity_text, current_velocity_text
    desired_velocity, current_velocity = MotorController.confirm_drive()
    desired_velocity_text.set(f"Desired Velocity: {round(desired_velocity, 3)} m/s")
    current_velocity_text.set(f"Current Velocity: {round(current_velocity, 3)} m/s")
    window.after(250, update_motor)


# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0, 1], minsize=200, weight=1)
window.columnconfigure([0, 1, 2], minsize=200, weight=1)

# Sets up frames
button_frame = tk.Frame(master=window)
button_frame_rows = [0, 1, 2, 3]
button_frame_columns = [0, 1, 2]
button_frame.rowconfigure(button_frame_rows, minsize=50, weight=1)
button_frame.columnconfigure(button_frame_columns, minsize=100, weight=1)
button_frame.grid(row=0, column=0, sticky='nsew')

pedal_frame = tk.Frame(master=window)
pedal_frame.rowconfigure([0], minsize=50, weight=1)
pedal_frame.columnconfigure([0, 1], minsize=100, weight=1)
pedal_frame.grid(row=0, column=1, sticky='nsew')

display_frame = tk.Frame(master=window)
display_frame_rows = [0, 1, 2, 3, 4]
display_frame_columns = [0]
display_frame.rowconfigure(display_frame_rows, minsize=50, weight=1)
display_frame.columnconfigure(display_frame_columns, minsize=100, weight=1)
display_frame.grid(row=0, column=2, sticky='nsew')

can_frame = tk.LabelFrame(master=window, text='CAN')
can_frame_rows = [0, 1]
can_frame_columns = [0]
can_frame.rowconfigure(can_frame_rows, minsize=50, weight=1)
can_frame.columnconfigure(can_frame_columns, minsize=100, weight=1)
can_frame.grid(row=1, column=0, sticky='nsew')

motor_frame = tk.LabelFrame(master=window, text='Motor')
motor_frame_rows = [0, 1]
motor_frame_columns = [0]
motor_frame.rowconfigure(motor_frame_rows, minsize=50, weight=1)
motor_frame.columnconfigure(motor_frame_columns, minsize=100, weight=1)
motor_frame.grid(row=1, column=1, sticky='nsew')

### Switches ###
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch, command=partial(Switches.toggle, switch))
    button.grid(row=i//len(button_frame_columns), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)

### Pedals ###
accelerator = tk.Scale(pedal_frame, from_=1, to=0, resolution=0.001, label='accelerator')
accelerator.grid(row=0, column=0, sticky='nsew')
brake = tk.Scale(pedal_frame, from_=1, to=0, resolution=0.001, label='brake')
brake.grid(row=0, column=1, sticky='nsew')

accelerator.configure(command=lambda pos : Pedals.set_pedals(pos, brake.get()))
brake.configure(command=lambda pos : Pedals.set_pedals(accelerator.get(), pos))

### Display ###
display_text = list()
for i, label in enumerate(Display.get_display()):
    text = tk.StringVar(value=label)
    display_text.append(text)
    cell = tk.Label(master=display_frame, textvariable=text)
    cell.grid(row=i//len(display_frame_columns), column=i%len(display_frame_columns), sticky='nsew')


### CAN ###
id_text = tk.StringVar(value='ID: ')
id_ = tk.Label(master=can_frame, textvariable=id_text)
id_.grid(row=0, column=0, sticky='nsew')
message_text = tk.StringVar(value='Message: ')
message = tk.Label(master=can_frame, textvariable=message_text)
message.grid(row=1, column=0, sticky='nsew')


### Motor ###
desired_velocity_text = tk.StringVar(value='Desired Velocity: ')
desired_velocity = tk.Label(master=motor_frame, textvariable=desired_velocity_text)
desired_velocity.grid(row=0, column=0, sticky='nsew')
current_velocity_text = tk.StringVar(value='Current Velocity: ')
current_velocity = tk.Label(master=motor_frame, textvariable=current_velocity_text)
current_velocity.grid(row=1, column=0, sticky='nsew')

# Sets up periodic updates
window.after(10, update_buttons)
window.after(20, update_CAN)
window.after(50, update_display)
window.after(250, update_motor)
window.mainloop()
