***********
Extra Files
***********

Some files in the Apps folder don't contain any task code, but instead contain higher level interfaces, calibration data, and supporting structures. Some of these files will likely be relocated in the future.

================
Pedal to Percent
================

This file contains an array for converting a pedal percentage (the index into the array) into a float between 0 and 1. The mapping is currently linear, but this can be changed. The file was created in order to avoid costly floating point operations (since we've turned the floating point unit off), and should really be somewhere else (perhaps the driver layer).

=====
Tasks
=====

This file contains the storage for all general tasks related objects, including stacks, TCBs, mutexes, and semaphores. All task files include ``Tasks.h`` and thus have access to them.

.. doxygengroup:: Tasks
   :project: doxygen
   :path: "/doxygen/xml/group__Tasks.xml"

