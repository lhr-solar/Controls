#!/bin/bash

if [[ -z "${RENODE_UART2_TELNET}" ]] || [[ -z "${RENODE_UART3_TELNET}" ]] || [[ -z "${RENODE_MOTORCAN_SOCKET}" ]] || [[ -z "${RENODE_CARCAN_SOCKET}" ]]; then
    echo "Please add your load_env_vars.sh script to your .bashrc in order to load the environment variables." ; exit 1
fi

(renode -v | grep -q 'Renode, version 1.13.3') && (echo "Starting Renode." && \
    renode --disable-xwt --console \
    -e "\$uart2_telnet=${RENODE_UART2_TELNET}; \
        \$uart3_telnet=${RENODE_UART3_TELNET}; \
        \$motorcan_socket=${RENODE_MOTORCAN_SOCKET}; \
        \$carcan_socket=${RENODE_CARCAN_SOCKET}; \
        include @Renode/startup.resc") || echo "Please install version 1.13.3 of Renode simulator."