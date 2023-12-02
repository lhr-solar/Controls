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

In order to compile the docs locally (which is essential for making sure everything looks fine before pushing), ``sphinx``, ``doxygen``, and ``breathe`` must be installed. The procedure might differ depending on distribution, but in Ubuntu this can be done with ``pip install -r Docs/source/requirements.txt`` and look at doxygen's documentation on how to install that. If pip is not installed, run ``sudo apt install python3-pip``.

To actually build the documentation, run ``make docs``. The resulting ``build/html/index.html`` can then be viewed in any browser. When your pull request is merged into the master branch, the documentation changes should be automatically visible on this website.

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

   Apps/FaultState
   Apps/Main
   Apps/ReadCarCAN
   Apps/ReadTritium
   Apps/SendCarCAN
   Apps/UpdateDisplay
   Apps/SendTritium
   Apps/ExtraFiles


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
