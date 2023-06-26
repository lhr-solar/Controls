*** Settings ***
Resource        ${RENODEKEYWORDS}

*** Variables ***

*** Keywords ***
Start Test
    # Cannot access environment variables from resc script, so need to define here. 
    Execute Command            $uart2_telnet=%{RENODE_UART2_TELNET};
    Execute Command            $uart3_telnet=%{RENODE_UART3_TELNET};
    Execute Command            $motorcan_socket=%{RENODE_MOTORCAN_SOCKET};
    Execute Command            $carcan_socket=%{RENODE_CARCAN_SOCKET};
    Execute Command            include @%{CONTROLS_REPO}/Simulator/startup.resc
    Create Terminal Tester     uart=sysbus.usart2    machine=ctrl-leader
    Start Emulation