# Controls
This repository contains all code related to Longhorn Racing Solar's Controls System

## Setup
There is currently one option to develop the Controls system. This option is to use a terminal in a linux environment to build and flash the program.

### Setup for Terminal Development
The system can be built and deployed from a terminal.
1. Ensure that you have some type of linux machine such as Ubuntu, Windows Subsystem for Linux, etc. Windows will not work.
    - A Linux Virtual Machine is highly recommended. You will not be able to flash code onto the board with WSL.
    - Use this link to download the Ubuntu Server image: [Ubuntu Server 22.04 Install](https://releases.ubuntu.com/22.04/)
    - Once the server is set up, you can ssh into it (basically connecting your development environment on the host machine to the code on the virtual machine). Ask another experienced member/lead to help you out with this step.
2. Download [VSCode](https://code.visualstudio.com/)
3. Clone the repository and its submodules with: 
```git clone --recurse-submodules https://github.com/lhr-solar/Controls.git``` 
4. Run install.sh to install the appropriate dependencies for both microcontroller and simulator development.
    - The install script will not work on systems that do not use the apt package manager. Please ask a lead to help install if you have a system without apt. 
5. In VSCode, download the following extensions (Click on the block icon on the left column):
    - C/C++
    - ARM
    - cortex-debug

## Building
When calling any of the following commands, make sure you are in the Controls folder.

Call ```make bsp_type``` to compile the release version of the code. ```bsp_type``` is the system you want to compile for.

Call ```make help``` for more information on how to build the project.

Call ```make clean``` if the build fails for any reason other than syntax related.

For testing, please read the Testing section.

## Testing
The following testing information is speficially for terminal development.

### Rules for making a new test src file
1. Test names: The formatting of the file names is crucial to have the makefile work properly. "Test_" should be the prefix and the src file name that is to be tested must follow i.e. if you want to test x.c, the test src file must be named Test_x.c or the makefile will not be able to find the file.
    E.g. A test for Dashboard.c should be Test_Dashboard.c file, a test for BSP_CAN.c should be named Test_BSP_CAN.c
2. All test files should be placed into the Tests folder.

### How to build a test
To build a new test, you need to use the following command:
```make bsp_type TEST=x```

- ```bsp_type``` specifies which system you want to compile the code for: ```stm32f413```, ```simulator```
- ```x``` specifies which test you want to compile. TEST= is optional and only required if a test is to be compiled. Set TEST equal to the suffix of the Test_ src files i.e. if the test you want to run is in Test_x.c, set TEST=x.
    E.g. Call ```make stm32f413 TEST=Dashboard``` if you want to test Dashboard.c with the Test_Dashboard.c src file

## Rules
Commit frequently into your own branches. Create a Pull Request whenever you are ready to add you working code to the master branch. You must select 1 reviewer for approval. Follow the coding guidelines in the Solar SharePoint. The reviewers will make sure everything is up to par with the coding standards.

Reviewers:
1. Sidharth Babu
2. Roie Gal
3. Ishan Deshpande
4. Nathaniel Delgado