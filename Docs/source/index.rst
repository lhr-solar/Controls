.. Controls documentation master file, created by
   sphinx-quickstart on Fri Jul 15 18:39:49 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the Controls API documentation!
=============================================

Please take the time to familiarize yourself with the system. The various pages on the left side of the screen give both high level descriptions for all major components of the Controls system.

Contributing to the docs
************************

Any change in functionality or interface should be reflected in the docs: out-of-date documentation can often be worse than no documentation at all! Try to update the docs while waiting for the rest of your pull request to get approved.

To ensure all dependencies are installed, run `install.sh` in the `Embedded-Sharepoint <https://github.com/lhr-solar/Embedded-Sharepoint>` for the documentation_toolset package.

To actually build the documentation, run ``make docs``. The resulting ``build/html/index.html`` can then be viewed in any browser. When your pull request is merged into the master branch, the documentation changes should be automatically visible on this website.

Keep the docs clear and readable. Follow the style of the existing documentation. If you're not sure how to document something, ask someone else on the team.

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: Overview
   
   Overview/Hardware
   Overview/Software

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: Apps

   Apps/Main
   Apps/ReadCarCAN
   Apps/ReadTritium
   Apps/SendCarCAN
   Apps/UpdateDisplay
   Apps/SendTritium
   Apps/Tasks
   Apps/CommandLine
   Apps/DebugDump
   Apps/PedalToPercent
   Apps/FaultState

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: Drivers

   Drivers/CANBus
   Drivers/Contactors
   Drivers/Display
   Drivers/Minions
   Drivers/Pedals

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: BSP

   BSP/ADC
   BSP/CAN
   BSP/GPIO
   BSP/UART