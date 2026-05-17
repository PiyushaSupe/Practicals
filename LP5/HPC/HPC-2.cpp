/*
Write a program to implement Parallel Bubble Sort and Merge sort using OpenMP. Use
existing algorithms and measure the performance of sequential and parallel algorithms.
*/

#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <ctime>

using namespace std;

// --- Bubble Sort ---
void sequentialBubbleSort(vector<int> &arr) {
    int n = arr.size();

    for (int i=0 ; i<n ; i++) {
        for (int j=0 ; j<n-i-1 ; j++) {
            if (arr[j] > arr[j+1]) swap(arr[j], arr[j + 1]);
        }
    }

}


void parBubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n; i++) {
        // Odd phase
        #pragma omp parallel for
        for (int j = 1; j < n; j += 2) {
            if (arr[j - 1] > arr[j]) swap(arr[j - 1], arr[j]);
        }
        // Even phase
        #pragma omp parallel for
        for (int j = 2; j < n; j += 2) {
            if (arr[j - 1] > arr[j]) swap(arr[j - 1], arr[j]);
        }
    }
}


void parallelBubbleSort(vector<int> &arr) {
    int n = arr.size();

    for (int i=0 ; i<n ; i++) {

        #pragma omp parallel for
        for (int j=1 ; j<n-i-1 ; j+=2) {
            if (arr[j] < arr[j-1]) swap(arr[j], arr[j-1]);
        }

        #pragma omp parallel for
        for (int j=2 ; j<n-i-1 ; j+=2) {
            if (arr[j] < arr[j-1]) swap(arr[j], arr[j-1]);
        }
    }

}


// --- Merge Sort ---
void merge(vector<int> &arr, int l, int m, int h) {
    int n1 = m - l + 1, n2 = h - m;
    vector<int> L(n1), R(n2);

    for (int x=0 ; x<n1 ; x++) L[x] = arr[l+x];
    for (int x=0 ; x<n2 ; x++) R[x] = arr[m+1+x];

    int i=0, j=0, k=l;
    while (i<n1 && j<n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++]; 
    while (i<n1) arr[k++] = L[i++];
    while (j<n2) arr[k++] = R[j++];
}


void seqMergeSort(vector<int>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        seqMergeSort(arr, l, m);
        seqMergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}


void parallelMergeSort(vector<int> &arr, int l, int h) {
    if (l < h) {
        // int m = l + (r - l) / 2;
        // // Limit recursion depth for efficiency
        // if (r - l < 1000) {
        //     seqMergeSort(arr, l, r);
        // } else {
        int m = l + (h-l)/2;
        #pragma omp task
        parallelMergeSort(arr, l, m);
        #pragma omp task
        parallelMergeSort(arr, m+1, h);
        #pragma omp taskwait
        merge(arr, l, m, h);
    }
}


int main() {
    int N = 10000; 
    vector<int> data(N);
    generate(data.begin(), data.end(), rand);

    cout << "--- Performance for N = " << N << " ---" << endl;

    // Bubble Sort Performance
    vector<int> b1 = data, b2 = data;
    double s = omp_get_wtime();
    sequentialBubbleSort(b1);
    cout << "Sequential Bubble Sort: " << omp_get_wtime() - s << "s" << endl;

    s = omp_get_wtime();
    parallelBubbleSort(b2);
    cout << "Parallel Bubble Sort: " << omp_get_wtime() - s << "s" << endl;

    // Merge Sort Performance
    vector<int> m1 = data, m2 = data;
    s = omp_get_wtime();
    seqMergeSort(m1, 0, N - 1);
    cout << "Sequential Merge Sort: " << omp_get_wtime() - s << "s" << endl;

    s = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
            parallelMergeSort(m2, 0, N - 1);
        }
    }
    
    cout << "Parallel Merge Sort: " << omp_get_wtime() - s << "s" << endl;


    // --- PARALLEL BUBBLE SORT (ODD-EVEN) COMPLEXITY ---
    // Theoretical Span: O(n), representing the n alternating phases.
    // Practically, following the Work-Span model (Brent's Theorem), 
    // the complexity is O( (n^2) / P + n ).
    // Note: By Amdahl's Law, the synchronization barriers between phases are 
    // the bottleneck. Parallelization reduces the O(n^2) work but the O(n) 
    // dependency chain of swaps remains.
    // (P = processors, n = number of elements)


    // --- PARALLEL MERGE SORT COMPLEXITY ---
    // Theoretical Span: O(log^2 n) for standard parallel merge, or O(log n) 
    // with a parallel merge step.
    // Practically, following the Work-Span model, the complexity is O( (n log n) / P + log n ).
    // Note: By Amdahl's Law, the final merge step and task creation overhead 
    // are the sequential bottlenecks. Performance scales well on large datasets 
    // until the task granularity becomes smaller than the thread management cost.
    // (P = processors, n = number of elements)


    return 0;
}