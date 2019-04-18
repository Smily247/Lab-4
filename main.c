#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>

static const long Num_To_Sort = 1000000;

//swap 2 values in the array
void swap(int *arr_1, int *arr_2) {
    int temp = *arr_1;// store the value that is in the first array
    *arr_1 = *arr_2;//store 2 value into the first location
    *arr_2 = temp;// store the 1 value in the second location
}

int Partition(int *arr, int start, int last) {
    int pivot = arr[last];// spot to pivot
    int i = start - 1; // keeps track of the portion that is sorted

    for (int j = start; j < last; j++) {
        // if the value at j is less than the value at the pivot
        // swap the value after i with the the value at j
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    // swap the last value in the array with the value just after the sorted portion in the array
    swap(&arr[i + 1], &arr[last]);
    return (i + 1);
}

// Sequential version of your sort
// If you're implementing the PSRS algorithm, you may ignore this section
void sort_s(int *arr, int start, int last) {

// if there is more than 1 item in the array keep sorting
// you are finished when there is only 1 left
    if (start < last) {
        // partition the array into 2 parts
        int part = Partition(arr, start, last);
        sort_s(arr, start, (part - 1));
        sort_s(arr, (part + 1), last);
    }

}

// Parallel version of your sort
void sort_p(int *arr, int start, int last) {

    if (start < last) {
        // partitioned the array into 2 parts
        int part = Partition(arr, start, last);
 // make the 2 partitioned portions into there own task to be run in parallel
#pragma omp task
        sort_p(arr, start, (part - 1));

#pragma omp task
        sort_p(arr, (part + 1), last);
    }

}

int main() {
    int *arr_s = malloc(sizeof(int) * Num_To_Sort);
    long chunk_size = Num_To_Sort / omp_get_max_threads();
#pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;
        for (long i = chunk_start; i < chunk_end; i++) {
            arr_s[i] = rand_r(&seed);
        }
    }

    // Copy the array so that the sorting function can operate on it directly.
    // Note that this doubles the memory usage.
    // You may wish to test with slightly smaller arrays if you're running out of memory.

    int *arr_p = malloc(sizeof(int) * Num_To_Sort);
    memcpy(arr_p, arr_s, sizeof(int) * Num_To_Sort);

    struct timeval start, end;
    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);

    sort_s(arr_s, 0, (Num_To_Sort - 1));

    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("sequential quick sorted first 10 values: ");
    for(int i = 0; i < 10; i++)
        printf("%i, ", arr_s[i]);

    free(arr_s);
    printf("\nTiming parallel...\n");
    gettimeofday(&start, NULL);

#pragma omp parallel
#pragma omp single
    sort_p(arr_p, 0, (Num_To_Sort - 1));

    gettimeofday(&end, NULL);

    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);
    printf("Parallel quick sorted first 10 values: ");
    for(int i = 0; i < 10; i++)
        printf("%i, ", arr_p[i]);
    free(arr_p);

    return 0;
}

// Got psuedocode from https://www.geeksforgeeks.org/quick-sort/