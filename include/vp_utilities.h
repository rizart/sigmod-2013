#ifndef VP_UTILITIES_H
#define VP_UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

// sorting function - given an array with size n it sorts it using radixsort
void radixsort(int *a, int n);

// return the standard deviation of an array with size n
float standard_deviation(int *data, int n);

// given an array of floating numbers return the position of the maximum value
int FindMax(float *standard_dev, int n);

#ifdef __cplusplus
}
#endif

#endif // VP_UTILITIES_H