#include <math.h>
#include <stdio.h>

void radixsort(int *array, int n) {
    int i, b[n], exp = 1;

    int m = array[0];
    for (i = 1; i < n; i++) // find number with max digits
        if (array[i] > m)
            m = array[i];

    while ((m / exp) > 0) {
        int bucket[10] = {0};

        for (i = 0; i < n; i++)
            bucket[(array[i] / exp) % 10]++;

        //-------------------------------------------------------
        for (i = 1; i < 10; i++)
            bucket[i] += bucket[i - 1];

        for (i = n - 1; i >= 0; i--)
            b[--bucket[(array[i] / exp) % 10]] = array[i]; // sort with digit
        //-------------------------------------------------------

        for (i = 0; i < n; i++)
            array[i] = b[i];

        exp *= 10;
    }
}

float standard_deviation(int *data, int n) {
    float mean = 0.0, sum_deviation = 0.0;
    int i;

    for (i = 0; i < n; ++i)
        mean += data[i];

    mean = mean / n;
    for (i = 0; i < n; ++i)
        sum_deviation += (data[i] - mean) * (data[i] - mean);

    return sqrt(sum_deviation / n);
}

int FindMax(float *standard_dev, int n) {
    int maxi = standard_dev[0];
    int i, x = 0;
    for (i = 1; i != n; i++) {
        if (standard_dev[i] > maxi)
            x = i;
    }
    return x;
}
