#!/bin/bash
echo ${CONTROLS_REPO}

if [[ $# -eq 0 ]]; then
    echo "Please provide a valid test file." ; exit 1
fi

if [[ -z "${CONTROLS_REPO}" ]] || [[ -z "${RENODE_TEST}" ]]; then
    echo "Please add your load_env_vars.sh script to your .bashrc in order to load the environment variables." ; exit 1
fi

mkdir -p ${CONTROLS_REPO}/Tests/Results/$1

cd ${CONTROLS_REPO}

make simulator TEST=$1 DEBUG=1 -s || { echo 'Compilation failed!' ; exit 1; }

# Check if a corresponding Robot test file exists
echo "Running $1"
if test -f ${CONTROLS_REPO}/Tests/Robot/Test_$1.robot; then
    TEST_ROBOT=${CONTROLS_REPO}/Tests/Robot/Test_$1.robot
else
    TEST_ROBOT=${CONTROLS_REPO}/Tests/Robot/Test_Template.robot
fi

RESULTS=${CONTROLS_REPO}/Tests/Results/$1

${RENODE_TEST} ${TEST_ROBOT} -r ${RESULTS} --kill-stale-renode-instances --verbose --variable TEST_NAME:$1