****************
Pedal To Percent
****************

This file contains an array for converting a pedal percentage (the index into the array) into a float between 0 and 1. The mapping is currently linear, but this can be changed. The file was created in order to avoid costly floating point operations (since we've turned the floating point unit off), and should really be somewhere else (perhaps the driver layer).