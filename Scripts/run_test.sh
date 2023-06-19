#!/bin/bash
if [[ $# -eq 0 ]]; then
    echo "Please provide a valid test file." ; exit 1
fi

if [[ -z "${CONTROLS_REPO}" ]] || [[ -z "${RENODE_TEST}" ]]; then
    echo "Please add your load_env_vars.sh script to your .bashrc in order to load the environment variables" ; exit 1
fi

mkdir -p ${CONTROLS_REPO}/Tests/Results/$1

cd ${CONTROLS_REPO}

# Check if a corresponding Leader test file exists
if test -f ${CONTROLS_REPO}/Tests/Leader/Test_$1.c; then
    TEST_LEADER=$1
else
    echo "Leader test file does not exist!" ; exit 1
fi

# Check if a corresponding Motor test file exists
if test -f ${CONTROLS_REPO}/Tests/Motor/Test_$1.c; then
    TEST_MOTOR=$1
else
    TEST_MOTOR=MotorSim
fi

# Check if a corresponding Car test file exists
if test -f ${CONTROLS_REPO}/Tests/Car/Test_$1.c; then
    TEST_CAR=$1
else
    TEST_CAR=CarSim
fi

make simulator TEST_LEADER=$TEST_LEADER TEST_MOTOR=$TEST_MOTOR TEST_CAR=$TEST_CAR || { echo 'Compilation failed!' ; exit 1; }

# Check if a corresponding Robot test file exists
echo "Running $1"
if test -f ${CONTROLS_REPO}/Tests/Robot/Test_$1.robot; then
    TEST_ROBOT=${CONTROLS_REPO}/Tests/Robot/Test_$1.robot
else
    TEST_ROBOT=${CONTROLS_REPO}/Tests/Robot/Test_Template.robot
fi

RESULTS=${CONTROLS_REPO}/Tests/Results/$1

${RENODE_TEST} ${TEST_ROBOT} -r ${RESULTS} --kill-stale-renode-instances --verbose --variable TEST_NAME:$1