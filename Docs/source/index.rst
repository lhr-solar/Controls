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

To actually build the documentation, run ``make docs``. The resulting ``build/html/index.html`` can then be viewed in any browser or in the editor using this [VSCode extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.live-server) by navigating to the Controls/Docs/build/html directory. When your pull request is merged into the master branch, the documentation changes should be automatically visible on this website.  

Keep the docs clear and readable. Follow the style of the existing documentation. If you're not sure how to document something, ask someone else on the team.

.. toctree::
   :hidden:
   :caption: Apps

   Apps/Initialization
   Apps/ReadCarCan
   Apps/ReadTritium
   Apps/SendCarCan
   Apps/UpdateDisplay
   Apps/SendTritium
   Apps/Tasks
   Apps/CommandLine
   Apps/DebugDump
   Apps/PedalToPercent

.. toctree::
   :hidden:
   :caption: Drivers

   Drivers/CanBus
   Drivers/Contactors
   Drivers/Display
   Drivers/Minions
   Drivers/Pedals

.. toctree::
   :hidden:
   :caption: BSP

   BSP/BSP_ADC
   BSP/BSP_CAN
   BSP/BSP_GPIO
   BSP/BSP_UART

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: STM32F4_StdPeriph

   STM32F4_StdPeriph/StdPeriph_Docs