*** Settings ***
Resource        ${RENODEKEYWORDS}
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      Reset Emulation
Test Teardown   Test Teardown

*** Variables ***

*** Keywords ***
Start Test
    Execute Command            include @%{CONTROLS_REPO}/Simulator/startup.resc
    Create Terminal Tester     uart=sysbus.usart2    machine=ctrl-leader
    Start Emulation

*** Test Cases ***
Test ${TEST_NAME}
    Start Test
    Wait For Line On Uart    End Test    