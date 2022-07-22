******************
Software Overview
******************

Files in the controls code base are split into three main groups: board support package (BSP), drivers, and apps. These layers correspond to increasingly more abstract code, and enable us to seperate low-level implementation details (such as specific peripheral initialization) from higher-level details (cruise control code, for example).

The board support package is our lowest abstraction layer. Its job is to hide away microcontroller specific initialization code, and present an interface to built-in peripherals, such as:

*   Analog-to-digital Converter (ADC) — for reading analog inputs
*   Controller Area Network (CAN) for — intra-car communication
*   Serial Peripheral Interface (SPI) — for off-chip communication
*   Universal Asynchronous Receiver-Transmitter (UART) — for communicating with our display

The drivers layer sits right above the BSP. Its job is to provide the hardware interface for all of our higher level code. This includes implementing communication protocols over the peripherals provided by the BSP, and in general aids in reducing the complexity of the application layer. Some examples of our drivers are:

*   Pedals — takes raw voltage values from the ADC and calibrates them into a percentage value
*   Display — includes routines to talk to the display over UART, allowing the apps layer to change values on the screen with a simple function call
*   Motor Controller — handles communication with the motor controller over CAN, allowing higher level code to set speed or current with a single function call

The apps layer sits atop the other two layers, making calls to driver code and operating system code in order to implement necessary logic for the function of the car. This code is divided into seperate, indepedent tasks, each of which controlls a certain aspect of the car. These include:

*   Send Display — retrieves status information (speed, gear, blinkers) from the rest of the car and sends them to the display
*   Update Velocity — manages car movement by sending speed and current values to the motor, as well as handling cruise control and regenerative braking
*   Read Car CAN — monitors incoming CAN messages on the car CAN line, disabling regenerative braking whenever it is deemed necessary to do so by the Battery Protection System
*   Fault State — takes over whenever a hardware or software fault is encountered, and decides whether it can recover or if the controls system must be shut down