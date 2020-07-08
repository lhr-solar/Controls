# Simulator
This simulator uses a tkinter GUI to allow us to interact with the simulated car

## Setup
The Controls system can be developed and tested using either Linux or WSL, and there are some differences in the setup

### Common
Run the following command in the terminal to install tkinter

```sudo apt-get install python3-tk```

Also, make sure you are running with python3.6+ (python3.8 is recommended)

### Linux
There should not be any additional steps to take for Linux since the DISPLAY environment variable should be set properly

### WSL
There are a few extra steps to take for WSL, and a few more if you are running WSL2 (until the issues are fixed, hopefully). We will start with the common steps for both. Run the following command

```sudo apt-get install x11-apps```

Then, install [VcXsrv](https://sourceforge.net/projects/vcxsrv/), which is an Xserver for Windows. You will have to run **XLaunch** before running the simulator each time. It will run in the system tray.

#### WSL2
Since WSL2 is so new, there are a few features that have not properly been transferred over. In order for the GUI to work for WSL2, there are a few extra steps.

1. Add a launch parameter to XLaunch
    - Find the file location of XLaunch.exe, Right Click and press **Properties**, in the **Target** field add ``` -ac``` after the closing double quotes
2. Allow VcXsrv through the Windows Firewall
    - Launch **Firewall & network protection**, click **Allow an app through firewall**, check both public and private boxes for VcXsrv windows xserver
3. Set the DISPLAY environment variable
    - Add the following line to your ```~/.bashrc```: ```export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0```
