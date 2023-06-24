/** 
 * @copyright Copyright (c) 2023 UT Longhorn Racing Solar
 * @file FaultHighPrio.c
 * @brief Fault Handler task spawn template for high-priority faults.
 * 
 * This contains a task spawned by error assertion functions to handle faults.
 * It pends on a mutex to ensure only one high-prio task is handled at a time,
 * Executes a callback function passed in as an argument,
 * And enters a nonrecoverable fault handler to stop the car.
 * 
 * The goal of this system is to localize error handling
 * and allow high priority errors to be addressed as soon as possible
 * while also making sure there are no blocking calls in timer callbacks.
 * 
 * @author Madeleine Lee (KnockbackNemo)
*/