#!/bin/bash

(renode -v | grep -q 'Renode, version 1.13.3') && (echo "Starting Renode." && renode --disable-xwt --console -- Simulator/startup.resc) || echo "Please install version 1.13.3 of Renode simulator."