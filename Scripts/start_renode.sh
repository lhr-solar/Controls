#!/bin/bash

if [[ -z "${RENODE_UART2_TELNET}" ]] || [[ -z "${RENODE_UART3_TELNET}" ]]; then
    echo "Please add your load_env_vars.sh script to your .bashrc in order to load the environment variables." ; exit 1
fi

(renode -v | grep -q "Renode, version $RENODE_VERSION") && (echo "Starting Renode." && \
    renode --disable-xwt --console \
    -e "\$uart2_telnet=${RENODE_UART2_TELNET}; \
        \$uart3_telnet=${RENODE_UART3_TELNET}; \
        include @Renode/startup.resc") || echo "Please install version $RENODE_VERSION of Renode simulator."