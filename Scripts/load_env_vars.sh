#!/bin/bash

export CONTROLS_REPO=$(dirname $(dirname $(realpath "$BASH_SOURCE")))
export RENODE_TEST=/usr/share/renode_portable/renode-test
export RENODE_UART2_TELNET=49152
export RENODE_UART3_TELNET=49153
export RENODE_MOTORCAN_SOCKET=49154
export RENODE_CARCAN_SOCKET=49155