.. _velocity:

********************
SendTritium Task
********************

The SendTritium task currently houses our velocity control code. 
It utilizes a FSM to coordinate what messages should be sent to the motor controller depending on a certain set of input variables.
This FSM depends on data from across the system, such as:
- The Pedals
- The switches
- The CAN messages from BPS (indicates whether we can regen brake or not)

.. doxygengroup:: SendTritium
   :project: doxygen
   :path: "/doxygen/xml/group__SendTritium.xml"
