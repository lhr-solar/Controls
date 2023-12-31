****************
Error Handling
****************
The Controls system deals with two types of errors: OS Errors and task errors. OS Errors are returned by OS function calls and immediately result in a shutdown of the Controls system. Task errors are thrown by applications as a result of undesirable conditions experienced by the car and are thrown by individual assertion functions local to apps.

OS Errors
===========
All OS functions return an error code whose value should immediately be checked by a call to ``assertOSError``. The Controls system cannot run without a properly functioning OS, so any OS error is critical and nonrecoverable. Thus, when an OS function returns an error code that is not ``OS_ERR_NONE``,  the assertion will fail, and the following steps will occur:
    #. If in DEBUG mode, the file and line of the failed assertion will be printed
    #. The Array Bypass-Precharge Contactor and Motor Bypass-Precharge Contactor will open
    #. The brakelight will be turned on
    #. The display will show the error code
    #. The Controls system will shut down by entering an infinite while loop

Task Error Assertion
======================
Some applications have local assertion functions that are called manually when the car experiences error conditions. These functions will store the error code in a global variable for inspection during debugging ("Error_AppName"), check the error type, and call ``throwTaskError`` with the appropriate arguments. Default error behavior such as opening contactors and displaying the error code is handled within ``throwTaskError``, and additional handling is done through optional callback functions.
The errors asserted by applications include

* :doc:`ReadTritium <ReadTritium>`: Motor controller errors
    * Errors sent from the motor controller are asserted by ReadTritium. These are typically nonrecoverable, with the exception of the motor watchdog and hall sensor errors.
* :doc:`ReadCarCAN <ReadCarCAN>`: Contactor disable conditions
    * These errors depend on BPS and include charge disable, contactor open, and BPS trip, as well as missed BPS messages.
* :doc:`UpdateDisplay <UpdateDisplay>`: Display errors
    * While many display errors should exist, the Nextion fails to actually send us any messages. Consequently, the only display errors that can occur are FIFO and parse errors thrown manually in Controls code.

``throwTaskError``
-------------------
Common steps for asserted task errors are handled in the general-purpose ``throwTaskError`` function located in ``Tasks.c``. Arguments are set based on the type (and possibly number of times of occurence) of the error and include:

    1. The error code to be displayed
    2. An optional callback function (NULL if none)
    3. Whether or not the scheduler should be locked (overridden to true if the error is nonrecoverable)
    4. Whether or not the error is nonrecoverable

The procedure for error-handling within the function is as follows:

    1. Return (if error code is NULL)
        * Though this step is added as an extra precaution, ``throwTaskError`` is intended to be called only if there is an error
    2. Lock the scheduler (if lockSched is selected)
        * This is used for high-priority errors. The intention is to prevent the OS from switching away from the current task so the error is addressed as quickly as possible.
        * Any nonrecoverable error is considered high-priority and will result in locking the scheduler regardless of the lockSched argument value.
    3. Open contactors
        * Nonrecoverable errors indicate a critical issue, so the array and motor precharge bypass contactors will both be opened to limit current flow.
    4. Turn on brakelight and display the error (if nonrecoverable)
    5. Callback function (if not NULL)
        * This allows for individual handling of errors. Any necessary steps not already taken above are included in these error-specific functions local to applications.
        * The callback function runs after step 3 due to its lesser importance, as well as the desire to override the display (Ex: to show an evacuation screen in lieu of the normal error screen in ReadCarCAN).
    6. Enter an infinite while-loop (if nonrecoverable) or unlock the scheduler (if recoverable)
        * If in DEBUG mode, all error variables will be printed within the while-loop.