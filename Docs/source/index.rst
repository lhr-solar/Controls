.. Controls documentation master file, created by
   sphinx-quickstart on Fri Jul 15 18:39:49 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the Controls System documentation!
=============================================

Please take the time to familliarize yourself with the system. The various pages on the left side of the screen give both high level descriptions for all major components of the Controls system. Some pages contain a description of low-level implementation details, but for the most part that information is best found in the source code itself.

Contributing to the docs
************************

Any change in functionality or interface should be reflected in the docs: out-of-date documentation can often be worse than no documentation at all! Try to update the docs while waiting for the rest of your pull request to get approved.

In order to compile the docs locally (which is essential for making sure everything looks fine before pushing), ``sphinx`` must be installed. The procedure might differ depending on distribution, but in Ubuntu this can be done with ``sudo apt install python3-sphinx``. The theme for the docs can be installed with ``pip install sphinx_rtd_theme`` (if pip is not installed, run ``sudo apt install python3-pip``).

To view a local copy of the docs, cd into the Docs directory and run ``make html``. The resulting ``build/html/index.html`` can then be viewed in any browser. When your pull request is merged into the master branch, the documentation changes should be automatically visible on this website.

Keep the docs clear and readible. Reach out to more experienced team members with any questions!

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

   Apps/BlinkLights
   Apps/FaultState
   Apps/Main
   Apps/ReadCarCAN
   Apps/ReadSwitches
   Apps/ReadTritium
   Apps/SendCarCAN
   Apps/UpdateDisplay
   Apps/UpdateVelocity
   Apps/ExtraFiles


.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: Drivers

   Drivers/CANBus
   Drivers/Contactors
   Drivers/Display
   Drivers/Minions
   Drivers/MotorController
   Drivers/Pedals

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: BSP

   BSP/ADC
   BSP/CAN
   BSP/GPIO
   BSP/SPI
   BSP/UART