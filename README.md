# LHRS Controls

## Table of Contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
1. [Getting Started](#getting-started)
	1. [Workspace Setup](#workspace-setup)
	1. [Build System](#build-system)
    1. [Usage](#usage)
    1. [Running Tests](#running-tests)
    1. [Debugging](#debugging)
1. [Toolset](#toolset)
    1. [Renode Simulator](#renode-simulator)
    1. [Formatting](#formatting)
    1. [Other](#other)
1. [Release Process](#release-process)
	1. [Versioning](#versioning)
1. [Contributing](#contributing)
    1. [Branch Convention](#branch-convention)
    1. [Review Process](#review-process)
1. [Documentation](#documentation)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgments](#acknowledgements)

## About the Project

This repository contains all code related to [Longhorn Racing Solar](https://www.longhornracing.org/solar-vehicle-team)'s Controls System, affiliated with the **University of Texas at Austin**. The Controls System is responsible for driver interactions, motor control, and lighting systems control. The system is built on [custom hardware](https://github.com/lhr-solar/Controls-LeaderPCB) and an ARM Cortex-M4 Processor (STM32F413).

For more information, see:
- [API documentation](https://controls-docs.readthedocs.io/en/latest/)
- [LHR Solar Wiki](https://wikis.utexas.edu/display/LHRSOLAR/Controls+Documentation)

**[Back to top](#table-of-contents)**

## Project Status

The Controls System is currently on release **M2.0** for the 2022-24 build cycle to compete in [FSGP 2024](https://www.americansolarchallenge.org/the-competition/2024-american-solar-challenge/). See our [Versioning](#versioning).

LHR Solar uses ClickUp as our project management tool. To access the [our ClickUp board](https://app.clickup.com/9011033583/v/f/90110413188), make sure a system lead has added you as a member.

We also use GitHub Projects to track software releases. See our [active projects](https://github.com/lhr-solar/Controls/projects?query=is%3Aopen).

**[Back to top](#table-of-contents)**

## Getting Started

Please review [Setting Up an Embedded Development Environment](https://wikis.utexas.edu/display/LHRSOLAR/Setting+up+an+Embedded+Development+Environment) on our wiki. This should get you up and running with some sort of Linux machine.

### Workspace Setup

1. Clone this repository via [SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh): ```git clone --recurse-submodules git@github.com:lhr-solar/Controls.git```. Make sure to include the --recurse-submodules flag, or run ```git submodule update --init --recursive``` to pull all necessary submodules.

1. In VSCode, remote into your Linux machine if needed. Click **File** &#8594; **Open Workspace from File** and select the workspace located in ```.vscode/LHR.code-workspace```. The VSCode Workspace has many integrations that developers may find useful while debugging or writing code, including but not limited to integrations with [Renode Simulator](#renode-simulator) and the openocd on-chip debugger.

1. Run ```Embedded-Sharepoint/Scripts/install.sh``` to selectively install your packages. **If you have not done the first step, this will not work, since the Embedded-Sharepoint is one of our submodules.** For Controls development, you will need the following packages:
    * ARM Toolchain
    * build-essential
    * gdb-multiarch
    * openocd
    * stlink-tools
    * Renode Simulator
    * Documentation Tools (Sphinx, Doxygen, and Breathe)

The script will prompt for installation of each of these packages.

1. Add ```source /path/to/Controls/Scripts/load_env_vars.sh``` to the end of your ```~/.bashrc``` file in order to load the necessary environment variables on shell startup. A typical example: ```source /${HOME}/LHR/Controls/Scripts/load_env_vars.sh```

### Build System

We use the GNU Make utility as our build system.

Examples:
```
make leader
make leader TEST=HelloWorld
make leader TEST=HelloWorld DEBUG=1
make flash
make docs
make clean
```

### Usage

Follow our [documentation](https://utexas.sharepoint.com/:w:/s/ENGR-LonghornRacing/EUx6dS9swT1Js18ZlOrAfJIBKsM_7dLuQ818EnGZKrpbAQ?e=PyRIyh) to set up the Controls test rig, power the board with 12V, and use ```make flash``` to flash the board with your ```Objects/controls-leader.elf``` executable, either built from source using ```make leader``` or placed in the Objects folder from your intended release.

### Running Tests

WIP: A formal test framework has yet to be defined for the Controls system. 

For now, ```make leader TEST=TestName``` should build the Controls system excluding **Apps/Src/main.c** and including **Tests/Test_TestName.c**.

### Debugging
OpenOCD is a debugger program that is open source and compatible with the STM32F413. GDB is a debugger program that can be used to step through a program as it is being run on the board. To use, you need two terminals open, as well as a USB connection to the ST-Link programmer (as if you were going to flash the program to the board). 
1. Run ```openocd``` in one terminal. Make sure it does not crash and there are no errors. You should see that it tells you what port to connect to (usually :3333).
1. In the other terminal, start gdb with the command ```gdb-multiarch ./Objects/controls-leader.elf``` (assuming that you are doing this in the root of the project directory).
1. This will launch GDB and read in all of the symbols from the program that you are running on the board. In order to attach gdb to the board, execute the command ```target extended-remote :3333```, which will connect to the openocd session started earlier.

**Note:** If you get an error message for permission denied, try giving openocd read/write permissions using chmod: ```chmod 764 openocd```

**[Back to top](#table-of-contents)**

## Toolset

The Controls toolset currently includes the [Renode Simulator](#renode-simulator), [formatting tools](#formatting)(WIP), and [other](#other).

### Renode Simulator

The [Renode](https://github.com/renode/renode) simulator is a software tool developed by Antmicro that the Controls team uses to run unmodified binaries identical to the ones one would normally flash onto target hardware. Put simply, it simulates the hardware and allows us to run and debug tests in preparation for hardware testing. It supports GDB usage and is incredibly useful for ironing out our higher level software bugs.

Check out our [Renode folder](./Renode/). See the [Renode Documentation](https://renode.readthedocs.io/en/latest/index.html) for more information on setup. To start Renode, run [```Scripts/start_renode.sh```](./Scripts/start_renode.sh).

### Formatting

WIP: clang-format and clang-tidy are in the process of being integrated into our workflow.

### Other

* [```load_env_vars.sh```](./Scripts/load_env_vars.sh) is our script for loading environment variables.
* [```bps_sim_can.py```](./Scripts/bps_sim_can.py) is a program to control a [CANDapter](https://www.ewertenergy.com/products.php?item=candapter) to simulate BPS CAN messages, as a black box isolated test for the Controls CarCAN system.
* See the [Embedded Sharepoint](https://github.com/lhr-solar/Embedded-Sharepoint) for more of our shared toolset with the rest of the Solar Embedded systems.

**[Back to top](#table-of-contents)**

## Release Process

Each release commit will be tagged with the version number (**MX.X.X**) and live on a production branch (**production/MX.X**). The controls-leader.elf binary corresponding to the release will also be uploaded to our [Releases](https://github.com/lhr-solar/Controls/releases) page on Github.

### Versioning

Our versioning system does NOT follow semantic versioning, but instead is based around iterations of the Longhorn Racing Solar Car. Each major version change corresponds to a different iteration of the car for a different year of competition.

Current versions:
- **M1** is for the FSGP 2022 Solar Car
- **M2** is for the FSGP 2024 Solar Car
- **M3** is for the FSGP 2025 Solar Car

Intermediate version names usually refer to a major change that exists for the same vehicle, such as breaking API changes or a rearchitecture.

**[Back to top](#table-of-contents)**

## Contributing

### Branch Convention

**Feature** branches (```feature/*-###```) are for features or bug fixes that are specifically associated with an existing issue ticket.
- Feature branches end with the issue ticket number that the feature is solving.
- Unit testing should be done on these feature branches to ensure that the feature works on an isolated level.

**Development** branches (```dev/*```) are for temporary development purposes or hotfixes.
- If a feature, bug, or enhancement starts being worked on without an existing issue ticket, a development branch can be created for it. 
- Hotfixes without associated issue tickets can also be created on development branches and merged from a PR without the need for a feature branch, given that the change is small.
- Any other experimentation or temporary needs can live on a development branch.

**Integration** branches (```integration/*-###```) are for integration testing.
- This includes integration between Controls and other systems on the vehicle, as well as integration between our internal Applications/Drivers.
- Integration branches end with the issue ticket number for the specific integration being done.

**Production** branches (```production/MX.X```) are for specific versions of the codebase. ```master``` will always host the latest version.

### Review Process

Commit frequently to your branch. A good rule of thumb is to push at natural stopping points + the end of each work session or workday, or more frequently that that if necessary. Create a pull request whenever you are ready to add you working code to the master branch. You must select 2 reviewers for approval. Any trained contributors are allowed to review and approve code. See our [PR Template](#PULL_REQUEST_TEMPLATE.md) for more review guidelines.

**[Back to top](#table-of-contents)**

## Documentation

### Controls System
[LHRS Controls ReadTheDocs](https://controls-docs.readthedocs.io/en/latest/)
[LHRS Controls Wiki](https://wikis.utexas.edu/display/LHRSOLAR/Controls+Documentation)

### RTOS
[uCOS-III Documentation](https://www.analog.com/media/en/dsp-documentation/software-manuals/Micrium-uCOS-III-UsersManual.pdf)

[FreeRTOS Developer Documentation](https://www.freertos.org/features.html)

### Hardware
[STM32F413 Documentation](https://www.st.com/resource/en/reference_manual/rm0430-stm32f413423-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

[Controls Leaderboard Repo](https://github.com/lhr-solar/Controls-LeaderPCB)

[GDB Cheatsheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf)

### Simulator
[Renode Simulator Documentation](https://renode.readthedocs.io/en/latest/)

**[Back to top](#table-of-contents)**

## License

Copyright (c) 2024 Longhorn Racing Solar

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

**[Back to top](#table-of-contents)**

## Authors

See the list of [contributors](https://github.com/lhr-solar/Controls/contributors) who participated in this project.

**[Back to top](#table-of-contents)**