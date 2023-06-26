*** Settings ***
Resource        Robot_Resource.resource
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      Reset Emulation
Test Teardown   Test Teardown

*** Test Cases ***
Hello World
    Start Test
    Wait For Line On Uart    Hello World    timeout=2