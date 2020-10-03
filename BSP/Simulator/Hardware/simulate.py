# Copyright (c) 2020 UT Longhorn Racing Solar

import os
import tkinter as tk
from functools import partial

import Switches
import Contactor
import Pedals
import Display
import CAN
import MotorController
import UART
import Lights
import PreCharge


# Update Frequencies (ms)
MOTOR_FREQ = 250
CAN1_FREQ = 500
CONTACTOR_FREQ = 500
DISPLAY_FREQ = 500
LIGHTS_FREQ = 500
PRECHARGE_FREQ = 500


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
    window.after(CAN1_FREQ, update_CAN)

def update_CAN2():
    """Periodally update the display state of the CAN2 bus"""
    global id_text2, message_text2
    can2 = MotorController.read()
    id_text2.set(f"ID: {can2[0]}")
    message_text2.set(f"Message: {can2[1]}")
    window.after(MOTOR_FREQ, update_CAN2)


def update_motor():
    """Periodically update the velocity and display of the motor"""
    global desired_velocity_text, current_velocity_text
    desired_velocity, current_velocity = MotorController.confirm_drive()
    desired_velocity_text.set(f"Desired Velocity: {round(desired_velocity, 3)} m/s")
    current_velocity_text.set(f"Current Velocity: {round(current_velocity, 3)} m/s")
    MotorController.torque_control(accelerator.get())
    window.after(MOTOR_FREQ, update_motor)

def update_precharge():
    """Periodically update the display state of the Motor and Array precharge boards"""
    global precharge_motor_status, precharge_array_status
    precharge_status = PreCharge.read()
    precharge_motor_status.set(f"Motor Precharge: {precharge_status[1]}")
    precharge_array_status.set(f"Array Precharge: {precharge_status[0]}")
    window.after(PRECHARGE_FREQ, update_precharge)  

def update_lights():
    """Periodically update the display state of the lights"""
    global lights_text
    lights_state = Lights.read()
    lights = Lights.get_lights()
    for i, light in enumerate(lights_text):
        light.set(f"{lights[i]}: {(lights_state >> i) & 0x01}")
    window.after(LIGHTS_FREQ, update_lights)


# Sets up the display environment variable
if os.environ.get('DISPLAY','') == '':
    os.environ.__setitem__('DISPLAY', ':0')

# Sets up window
window = tk.Tk()
window.rowconfigure([0, 1], minsize=200, weight=1)
window.columnconfigure([0, 1, 2, 3, 4], minsize=200, weight=1)

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

cans_frame = tk.Frame(master=window)
cans_frame_rows = [0, 1]
cans_frame_columns = [0]
cans_frame.rowconfigure(cans_frame_rows, minsize=50, weight=1)
cans_frame.columnconfigure(cans_frame_columns, minsize=100, weight=1)
cans_frame.grid(row=1, column=0, sticky='nsew')

can_frame = tk.LabelFrame(master=cans_frame, text='CAN')
can_frame_rows = [0, 1]
can_frame_columns = [0]
can_frame.rowconfigure(can_frame_rows, minsize=50, weight=1)
can_frame.columnconfigure(can_frame_columns, minsize=100, weight=1)
can_frame.grid(row=0, column=0, sticky='nsew')

can2_frame = tk.LabelFrame(master=cans_frame, text="CAN2")
can2_frame_rows = [0, 1]
can2_frame_columns = [0]
can2_frame.rowconfigure(can2_frame_rows, minsize=50, weight=1)
can2_frame.columnconfigure(can2_frame_columns, minsize=100, weight=1)
can2_frame.grid(row=1, column=0, sticky='nsew')

CAN_messages_frame = tk.LabelFrame(master=window, text='CAN System Messages')
CAN_messages_frame_rows = [0, 1]
CAN_messages_frame_columns = [0]
CAN_messages_frame.rowconfigure(CAN_messages_frame_rows, minsize=20, weight=1)
CAN_messages_frame.columnconfigure(CAN_messages_frame_columns, minsize=20, weight=1)
CAN_messages_frame.grid(row=0, column=4, sticky='nsew')

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

messages_frame = tk.LabelFrame(master=window, text='Controls System Messages')
messages_frame_rows = [0, 1]
messages_frame_columns = [0]
messages_frame.rowconfigure(messages_frame_rows, minsize=20, weight=1)
messages_frame.columnconfigure(messages_frame_columns, minsize=20, weight=1)
messages_frame.grid(row=0, column=3, sticky='nsew')

precharge_frame = tk.LabelFrame(master=window, text="PreCharge")
precharge_frame_rows = [0,1]
precharge_frame_columns = [0]
precharge_frame.rowconfigure(precharge_frame_rows, minsize=50, weight = 1)
precharge_frame.columnconfigure(precharge_frame_columns, minsize=50, weight = 1)
precharge_frame.grid(row = 1, column = 3, sticky='nsew')

lights_frame = tk.LabelFrame(master=window, text="Lights")
lights_frame_rows = [0, 1, 2, 3, 4]
lights_frame_columns = [0, 1]
lights_frame.rowconfigure(lights_frame_rows, minsize=50, weight=1)
lights_frame.columnconfigure(lights_frame_columns, minsize=50, weight=1)
lights_frame.grid(row=1, column=4, sticky='nsew')


### Switches ###
buttons = []
for i, switch in enumerate(Switches.get_switches()):
    button = tk.Button(master=button_frame, text=switch)
    button.grid(row=i//len(button_frame_columns), column=i%len(button_frame_columns), sticky='nsew')
    buttons.append(button)
for i, switch in enumerate(Switches.get_switches()):
    buttons[i].config(command=partial(Switches.toggle, switch, buttons))



### Lights ###
lights_text = list()
for i, light in enumerate(Lights.get_lights()):
    light_text = tk.StringVar(value=f'{light}: ')
    light = tk.Label(master=lights_frame, textvar=light_text)
    light.grid(row=i//len(lights_frame_columns), column=i%len(lights_frame_columns), sticky='nsew')
    lights_text.append(light_text)

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

### CAN2 ###
id_text2 = tk.StringVar(value='ID: ')
id2_ = tk.Label(master=can2_frame, textvariable=id_text2)
id2_.grid(row=0, column=0, sticky='nsew')
message_text2 = tk.StringVar(value='Message: ')
message2 = tk.Label(master=can2_frame, textvariable=message_text2)
message2.grid(row=1, column=0, sticky='nsew')


### CAN messages input ###
can_id_input = tk.Entry(master=CAN_messages_frame)
can_id_input.grid(row=0, column=0)
can_msg_input = tk.Entry(master=CAN_messages_frame)
can_msg_input.grid(row=1, column=0)
can_button = tk.Button(master=CAN_messages_frame, text="Send", command=lambda : CAN.write(can_id_input.get(), can_msg_input.get()))
can_button.grid(row=2, column=0)


### Motor ###
desired_velocity_text = tk.StringVar(value='Desired Velocity: ')
desired_velocity = tk.Label(master=motor_frame, textvariable=desired_velocity_text)
desired_velocity.grid(row=0, column=0, sticky='nsew')
current_velocity_text = tk.StringVar(value='Current Velocity: ')
current_velocity = tk.Label(master=motor_frame, textvariable=current_velocity_text)
current_velocity.grid(row=1, column=0, sticky='nsew')

### Precharge ###
precharge_motor_status = tk.StringVar(value= 'Motor Precharge: ')
precharge_motor_ = tk.Label(master=precharge_frame, textvariable=precharge_motor_status)
precharge_motor_.grid(row = 0, column = 0, sticky='nsew')
precharge_array_status = tk.StringVar(value= 'Array Precharge: ')
precharge_array_txt = tk.Label(master=precharge_frame, textvariable=precharge_array_status)
precharge_array_txt.grid(row=1, column=0, sticky='nsew')

### UART messages input ###
uart_input = tk.Entry(master=messages_frame)
uart_input.grid(row=0, column=0)
uart_button = tk.Button(master=messages_frame, text="Send", command=lambda : UART.write(uart_input.get()))
uart_button.grid(row=1, column=0)

# Sets up periodic updates
can_frame.after(CAN1_FREQ, update_CAN)
can2_frame.after(MOTOR_FREQ,update_CAN2)
contactor_frame.after(CONTACTOR_FREQ, update_contactors)
display_frame.after(DISPLAY_FREQ, update_display)
motor_frame.after(MOTOR_FREQ, update_motor)
lights_frame.after(LIGHTS_FREQ, update_lights)
precharge_frame.after(PRECHARGE_FREQ, update_precharge)
window.mainloop()
