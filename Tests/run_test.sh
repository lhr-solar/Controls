#!/bin/bash

if [[ $# -eq 0 ]]; then
    echo "Please provide a valid test file."
    exit 0
fi

if [[ -z "${CONTROLS_REPO}" ]]; then
    echo "Please set CONTROLS_REPO environment variable to point to the top level Controls folder."
    exit 0
fi

if [[ -z "${RENODE_TEST}" ]]; then
    echo "Please set RENODE_TEST environment variable to point to your renode-test script."
    exit 0
fi

mkdir -p ${CONTROLS_REPO}/Tests/Results/$1

echo "Running $1"
${RENODE_TEST} ${CONTROLS_REPO}/Tests/Robot/Test_$1.robot -r ${CONTROLS_REPO}/Tests/Results/$1 --kill-stale-renode-instances --verbose