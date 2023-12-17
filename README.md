# LHRS Controls

This repository contains all code related to Longhorn Racing Solar's Controls System. The Controls System is responsible for driver interactions, motor control, and lighting systems control.

## Table of Contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
1. [Getting Started](#getting-started)
	1. [Workspace Setup](#workspace-setup)
	1. [Build System](#build-system)
    1. [Running Tests](#running-tests)
    1. [Formatting](#formatting)
    1. [Usage](#usage)
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

This repository contains all code related to [Longhorn Racing Solar](https://www.longhornracing.org/solar-vehicle-team)'s Controls System, affiliated with **UT Austin**. The Controls System is responsible for driver interactions, motor control, and lighting systems control. The system is built on [custom hardware](https://github.com/lhr-solar/Controls-LeaderPCB) and an ARM Cortex-M4 Processor (STM32F413).

For more information, see our [documentation](https://controls-docs.readthedocs.io/en/latest/).

**[Back to top](#table-of-contents)**

## Project Status

The Controls System is currently on release **M2.0** for the 2022-24 build cycle, to compete in FSGP 2024. See our [Versioning](#versioning).

**[Back to top](#table-of-contents)**

## Getting Started

The preferred development environment for the system is VSCode on a Linux machine. Options for developers on Windows environments include WSL or a Virtual Machine. Instructions provided here will assume that you have a working Linux environment with terminal control.

### Workspace Setup

1. Clone this repository via [SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh): ```git clone --recurse-submodules git@github.com:lhr-solar/Controls.git```.

1. In VSCode, remote into your Linux machine if needed. Click **File** &#8594; **Open Workspace from File** and select the workspace located in ```.vscode/LHR.code-workspace```.

1. Run ```Embedded-Sharepoint/Scripts/install.sh``` to selectively install your packages. For Controls development, you will need the following packages:
    * ARM Toolchain
    * build-essential
    * gdb-multiarch
    * openocd
    * stlink-tools
    * Renode Simulator
    * Documentation Tools (Sphinx, Doxygen, and Breathe)

    The script will prompt for installation of each of these packages.

1. Add ```source /path/to/Controls/Scripts/load_env_vars.sh``` to the end of your ```~/.bashrc``` file in order to load the necessary environment variables on shell startup. A typical example: ```source /${HOME}/LHR/Controls/Scripts/load_env_vars.sh```

1. Recommended extensions for VSCode:
    * C/C++ Extension Pack
    * GitHub Pull Requests
    * GitLens
    * Subway Surfers

### Build System

We use the GNU Make utility as our build system.

Examples:
```
make leader
make leader TEST=HelloWorld
make leader TEST=HelloWorld DEBUG=1
make flash
make docs
```

### Running Tests

WIP: A formal test framework has yet to be defined for the Controls system. For now, ```make leader TEST=TestName``` should build the Controls system excluding **main.c** and including **Tests/Test_TestName.c**

### Formatting

WIP: clang-format and clang-tidy are in the process of being integrated into our workflow.

### Usage

Follow our documentation to set up the Controls test rig, power the board with 12V, and use ```make flash``` to flash the board with your ```Objects/controls-leader.elf``` executable, either built from source or placed in the Objects folder from your intended release.

For debugging purposes, make sure there is a Nucleo plugged into your computer and connected to the leaderboard via JTAG. Run ```openocd``` in one bash session and ```gdb-multiarch``` in another, then attach to the process openocd has started.

**[Back to top](#table-of-contents)**

## Release Process

Each release commit will be tagged with the version number (**MX.X.X**) and live on a production branch (**production/MX.X**). The controls-leader.elf binary corresponding to the release will also be uploaded to our [Releases](https://github.com/lhr-solar/Controls/releases) page on Github.

### Versioning

Our versioning system does NOT follow semantic versioning, but instead is based around iterations of the Longhorn Racing Solar Car. Each major version change corresponds to a different iteration of the car for a different year of competition.

Current versions:
- **M1** is for the FSGP 2022 Solar Car
- **M2** is for the FSGP 2024 Solar Car
- **M3** is for the FSGP 2025 Solar Car

Intermediate version names usually refers to a major change that exists for the same vehicle, such as breaking API changes or a rearchitecture.

**[Back to top](#table-of-contents)**

## Contributing

### Branch Convention

**Feature** branches (```feature/*-###```) are for features or bug fixes that are specifically associated with an existing issue ticket.
- Feature branches end with the issue ticket number that the feature is solving.

**Development** branches (```dev/*```) are for temporary development purposes or hotfixes.
- If a feature, bug, or enhancement starts being worked on without an existing issue ticket, a development branch can be created for it. 
- Hotfixes without associated issue tickets can also be created on development branches and merged from a PR without the need for a feature branch, given that the change is small.
- Any other experimentation or temporary needs can live on a development branch.

**Integration** branches (```integration/*-###```) are for integration testing.
- This includes integration between Controls and other systems on the vehicle, as well as integration between our internal Applications/Drivers.
- Integration branches end with the issue ticket number for the specific integration being done.

**Production** branches (```production/MX.X```) are for specific versions of the codebase. ```master``` will always host the latest version.

### Review Process

Commit frequently into your branch. Create a Pull Request whenever you are ready to add you working code to the master branch. You must select 2 reviewers for approval. Any trained contributors are allowed to review and approve code. See our [PR Template](#PULL_REQUEST_TEMPLATE.md) for more review guidelines.

**[Back to top](#table-of-contents)**

## Documentation

### Controls System
[LHRS Controls ReadTheDocs](https://controls-docs.readthedocs.io/en/latest/)

### RTOS
[uCOS-III Documentation](https://www.analog.com/media/en/dsp-documentation/software-manuals/Micrium-uCOS-III-UsersManual.pdf)

[FreeRTOS Developer Documentation](https://www.freertos.org/features.html)

### Hardware
[STM32F413 Documentation](https://www.st.com/resource/en/reference_manual/rm0430-stm32f413423-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

[Controls Leader Repo](https://github.com/lhr-solar/Controls-LeaderPCB)

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