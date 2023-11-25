# LHRS Controls

This repository contains all code related to Longhorn Racing Solar's Controls System. The Controls System is responsible for driver interactions, motor control, and lighting systems control.

## Table of Contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
1. [Getting Started](#getting-started)
	1. [Workspace Setup](#workspace-setup)
	1. [Building](#building)
	2. [Running Tests](#running-tests)
		1. [Other Tests](#other-tests)
	1. [Installation](#installation)
	1. [Usage](#usage)
1. [Release Process](#release-process)
	1. [Versioning](#versioning)
	1. [Payload](#payload)
1. [How to Get Help](#how-to-get-help)
1. [Contributing](#contributing)
1. [Further Reading](#further-reading)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgments](#acknowledgements)

## About the Project

Here you can provide more details about the project
* What features does your project provide?
* Short motivation for the project? (Don't be too long winded)
* Links to the project site

```
Show some example code to describe what your project does
Show some of your APIs
```

**[Back to top](#table-of-contents)**

## Project Status

The Controls System is currently on release **M2.0** for the 2022-24 build cycle, to compete in FSGP 2024. See our [Release Process](#release-process).

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

### Building

We use the GNU Make utility as our build system.

Examples:
```
make leader
make leader TEST=HelloWorld
make leader TEST=HelloWorld DEBUG=1
```

### Running Tests

WIP: A formal test framework has yet to be defined for the Controls system.

#### Other Tests

If you have formatting checks, coding style checks, or static analysis tests that must pass before changes will be considered, add a section for those and provide instructions

WIP: clang-format and clang-tidy are used


**[Back to top](#table-of-contents)**

## Release Process

Talk about the release process. How are releases made? What cadence? How to get new releases?

### Versioning

This project uses [Semantic Versioning](http://semver.org/). For a list of available versions, see the [repository tag list](https://github.com/your/project/tags).

**[Back to top](#table-of-contents)**

## How to Get Help

Provide any instructions or contact information for users who need to get further help with your project.

## Contributing

**[Back to top](#table-of-contents)**

## Further Reading

Provide links to other relevant documentation here

**[Back to top](#table-of-contents)**

## License

Copyright (c) 2024 Longhorn Racing Solar

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

**[Back to top](#table-of-contents)**

## Authors

* **[Phillip Johnston](https://github.com/phillipjohnston)** - *Initial work* - [Embedded Artistry](https://github.com/embeddedartistry)

Also see the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

**[Back to top](#table-of-contents)**

## Acknowledgments

Provide proper credits, shout-outs, and honorable mentions here. Also provide links to relevant repositories, blog posts, or contributors worth mentioning.

**[Back to top](#table-of-contents)**