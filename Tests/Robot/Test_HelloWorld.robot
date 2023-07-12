*** Settings ***
Resource        ../Inc/Robot_Resource.resource
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      Reset Emulation
Test Teardown   Test Teardown

*** Test Cases ***
Test Hello World
    Start Test
    Wait For Line On Uart    Hello World    timeout=2