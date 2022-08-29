*********
Init Task
*********

The init task is the first task to be spawned in the system. It's responsible for

*   Initializing systick and all of the drivers
*   Creating the CAN queue
*   Creating all other tasks in the system

After this, it goes to sleep indefinitely. The init task does not currently delete itself, but since it never does anything after initialization is complete, this might change in the future.

Main
====

The init task is contain within ``main.c`` in the apps folder, which also contains the main function. The main function handles everything that must be done before the RTOS is started and the scheduler runs tasks. It

*   Disables interrupts
*   Initializes the RTOS
*   Initializes all fault related bitmaps
*   Creates the fault state semaphore
*   Creates the init task
*   Re-enables interrupts and starts the RTOS

There's a body-less while loop after this last call just in case, but the RTOS starts running and won't return control back to the main function.