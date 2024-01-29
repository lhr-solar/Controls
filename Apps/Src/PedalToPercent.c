/**
 * @file PedalToPercent.c
 * @brief Contains an array for converting a pedal percentage (the
 * index into the array) into a float between 0 and 1, inclusive. 
 * 
 * The mapping is currently linear, but this can be changed. The Micrium uCOS-III operating system
 * had a bug where it would not be able to use the floating point unit (FPU). The array in this
 * file is used to convert the pedal percentage to a float without using the FPU. You can index into
 * 
 *
 */

/* Converts accelerator pedal percentage to a decimal */
const float kPedalToPercent[] = {
    0.00F, 0.01F, 0.02F, 0.03F, 0.04F, 0.05F, 0.06F, 0.07F, 0.08F, 0.09F, 0.1F,
    0.11F, 0.12F, 0.13F, 0.14F, 0.15F, 0.16F, 0.17F, 0.18F, 0.19F, 0.2F,  0.21F,
    0.22F, 0.23F, 0.24F, 0.25F, 0.26F, 0.27F, 0.28F, 0.29F, 0.3F,  0.31F, 0.32F,
    0.33F, 0.34F, 0.35F, 0.36F, 0.37F, 0.38F, 0.39F, 0.4F,  0.41F, 0.42F, 0.43F,
    0.44F, 0.45F, 0.46F, 0.47F, 0.48F, 0.49F, 0.5F,  0.51F, 0.52F, 0.53F, 0.54F,
    0.55F, 0.56F, 0.57F, 0.58F, 0.59F, 0.6F,  0.61F, 0.62F, 0.63F, 0.64F, 0.65F,
    0.66F, 0.67F, 0.68F, 0.69F, 0.7F,  0.71F, 0.72F, 0.73F, 0.74F, 0.75F, 0.76F,
    0.77F, 0.78F, 0.79F, 0.8F,  0.81F, 0.82F, 0.83F, 0.84F, 0.85F, 0.86F, 0.87F,
    0.88F, 0.89F, 0.9F,  0.91F, 0.92F, 0.93F, 0.94F, 0.95F, 0.96F, 0.97F, 0.98F,
    0.99F, 1.0F,
};
