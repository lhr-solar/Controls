*** Settings ***
Resource        Robot_Resource.resource
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      Reset Emulation
Test Teardown   Test Teardown

*** Test Cases ***
Test ${TEST_NAME}
    Start Test
    Wait For Line On Uart    End Test