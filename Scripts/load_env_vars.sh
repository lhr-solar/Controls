#!/bin/bash

export CONTROLS_REPO=$(git -C "$(dirname "$BASH_SOURCE")" rev-parse --show-toplevel)
export RENODE_UART2_TELNET=49152
export RENODE_UART3_TELNET=49153
export RENODE_MOTORCAN_SOCKET=49154
export RENODE_CARCAN_SOCKET=49155