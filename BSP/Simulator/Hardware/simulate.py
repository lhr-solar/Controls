import os
import tkinter as tk
from functools import partial

import Switches
import Timer
import Contactor
import Pedals
import Display
import CAN
import MotorController


# Update Frequencies (ms)
TIMER_FREQ = 1
MOTOR_FREQ = 250
CAN_FREQ = 500
CONTACTOR_FREQ = 500
DISPLAY_FREQ = 500


def update_timers():
	""" Periodically calls Timer.update to update the timers."""
	Timer.update()
	window.after(TIMER_FREQ, update_timers)


def update_contactors():
    """Periodically update the display state of the Motor and Array Contactors"""
    global motor_status, array_status
    contactors_status = Contactor.read()
    motor_status.set(f"Motor Contactor: {contactors_status[0]}")
    array_status.set(f"Array Contactor: {contactors_status[1]}")
    window.after(CONTACTOR_FREQ, update_contactors)


def update_display():
    """Periodically update the display state of display"""
    global display_text
    display = Display.read()
    for i, text in enumerate(display.keys()):
        display_text[i].set(f"{text}: {display[text]}")
    window.after(DISPLAY_FREQ, update_display)


def update_CAN():
    """Periodically update the display state of the CAN bus"""
    global id_text, message_text
    can = CAN.read()
    id_text.set(f"ID: {can[0]}")
    message_text.set(f"Message: {can[1]}")
    window.after(CAN_FREQ, update_CAN)


def update_motor():
    """Periodically update the velocity and display of the motor"""
    global desired_velocity_text, current_velocity_text
    desired_velocity, current_velocity = MotorController.confirm_drive()
    desired_velocity_text.set(f"Desired Velocity: {round(desired_velocity, 3)} m/s")
    current_velocity_text.set(f"Current Velocity: {round(current_velocity, 3)} m/s")
    MotorController.torque_control(accelerator.get())
    window.after(MOTOR_FREQ, update_motor)


# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0, 1], minsize=200, weight=1)
window.columnconfigure([0, 1, 2], minsize=200, weight=1)

# Sets up frames
button_frame = tk.LabelFrame(master=window, text='Switches')
button_frame_rows = [0, 1, 2, 3]
button_frame_columns = [0, 1, 2]
button_frame.rowconfigure(button_frame_rows, minsize=50, weight=1)
button_frame.columnconfigure(button_frame_columns, minsize=100, weight=1)
button_frame.grid(row=0, column=0, sticky='nsew')

pedal_frame = tk.LabelFrame(master=window, text='Pedals')
pedal_frame.rowconfigure([0], minsize=50, weight=1)
pedal_frame.columnconfigure([0, 1], minsize=100, weight=1)
pedal_frame.grid(row=0, column=1, sticky='nsew')

display_frame = tk.LabelFrame(master=window, text='Display')
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

contactor_frame = tk.LabelFrame(master=window, text="Contactors")
contactor_frame_rows = [0,1]
contactor_frame_columns = [0]
contactor_frame.rowconfigure(contactor_frame_rows, minsize=50, weight = 1)
contactor_frame.columnconfigure(contactor_frame_columns, minsize=50, weight=1)
contactor_frame.grid(row=1, column=2, sticky='nsew')


### Switches ###
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch)
    button.grid(row=i//len(button_frame_columns), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)
for i, switch in enumerate(Switches.get_switches()):
    buttons[i].config(command=partial(Switches.toggle, switch, buttons))


### Contactors ###
motor_status = tk.StringVar(value= 'Motor Contactor: ')
motor_ = tk.Label(master=contactor_frame, textvariable=motor_status)
motor_.grid(row = 0, column = 0, sticky='nsew')
array_status = tk.StringVar(value= 'Array Contactor: ')
array_txt = tk.Label(master=contactor_frame, textvariable=array_status)
array_txt.grid(row=1, column=0, sticky='nsew')


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
window.after(TIMER_FREQ, update_timers)
can_frame.after(CAN_FREQ, update_CAN)
contactor_frame.after(CONTACTOR_FREQ, update_contactors)
display_frame.after(DISPLAY_FREQ, update_display)
motor_frame.after(MOTOR_FREQ, update_motor)
window.mainloop()
