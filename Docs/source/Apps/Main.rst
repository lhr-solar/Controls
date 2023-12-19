*********
Init Task
*********

The init task is the first task to be spawned in the system. It's responsible for

*   Initializing the system clock
*   Initializing all of the drivers & applications
*   Creating all other tasks in the system

After this, it goes to sleep indefinitely. The init task does not currently delete itself, but since it never does anything after initialization is complete, this might change in the future.

Main
====

The init task is contained within ``main.c`` in the apps folder, which also contains the main function. The main function handles everything that must be done before the RTOS is started and the scheduler runs tasks. It

*   Disables interrupts
*   Initializes the RTOS & Task Switch Hook
*   Creates the init task
*   Re-enables interrupts
*   Starts the RTOS

There's a body-less while loop after this last call just in case, but the RTOS starts running and won't return control back to the main function.