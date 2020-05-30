# RTOS

This system uses the uC/OS - III RTOS from Micrium. The full reference manual can be found [here](https://www.analog.com/media/en/dsp-documentation/software-manuals/Micrium-uCOS-III-UsersManual.pdf)

## Setup

This system currently uses the **STM32F4xx** port for the real board and the **Linux** simulation port for development and testing without the hardware

### Setup for STM32F4xx
*Documentation to be updated when this has been tested*

### Setup for Linux

In order to use uC/OS - III, the real-time priority limit must be set to unlimited. The process to do so is different in Windows Subsystem for Linux than it is for a true Linux OS

#### Windows Subsystem for Linux

Add the following line to your ```~/.bashrc``` file:
```bash
ulimit -r unlimited > /dev/null 2>&1
```
This will set the rtprio limit to unlimited every time you open a new terminal session. However, it will not run successfully unless the terminal is in sudo mode.
Now, each time you want to **execute** the Controls code, you must do so with sudo permissions by first running
```bash
sudo bash
```
then typing your password. A new terminal session with sudo permissions will be created, and you will now be able to run the executable. Unfortunately, this hinders our ability to debug using the VSCode GUI, but hopefully that will be fixed in the future.

#### Linux OS

Add the following line to your ```/env/security/limits.conf``` file:
```
<username> - rtprio unlimited
```
This is a read-only file by default, so make sure to edit the file with ```sudo``` permissions. **If this method does not work for any reason, the method prescribed for WSL will work for Linux OS as well, but it is less convenient.**

## Development

Every thread in uC/OS - III is called a **task**, and every task is created using the ```OSTaskCreate()``` method. All required arguments for that function can be found in the reference manual linked at the top of this document.

### Basic Task Requirements
1. The first task created must call ```CPU_Init()``` and ```OS_CPU_SysTickInit()```
2. Every task's body must be enclosed in a ```while``` loop
3. Every task's body must include a function call to one of the following services:
    - ```OSFlagPend()```
    - ```OSMutexPend()```
    - ```OSQPend()```
    - ```OSSemPend()```
    - ```OSTimeDly()```
    - ```OSTimeDlyHMSM()```
    - ```OSTaskQPend()```
    - ```OSTaskSemPend()```
    - ```OSTaskSuspend()```
    - ```OSTaskDel()```
