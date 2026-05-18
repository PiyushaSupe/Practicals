/*
===========================================================
PARALLEL BUBBLE SORT AND MERGE SORT USING OPENMP
===========================================================

AIM:
To implement Parallel Bubble Sort and Parallel Merge Sort
using OpenMP and compare their performance with sequential
algorithms.

-----------------------------------------------------------
THEORY:
-----------------------------------------------------------

1. Bubble Sort:
--------------------------------
- Bubble Sort repeatedly swaps adjacent elements
  if they are in wrong order.
- Largest element moves to the end after every pass.

Example:
Array: 5 3 2 4 1

Pass 1:
3 2 4 1 5

Pass 2:
2 3 1 4 5

Pass 3:
2 1 3 4 5

Pass 4:
1 2 3 4 5

Time Complexity:
O(n²)

-----------------------------------------------------------

2. Merge Sort:
--------------------------------
- Divide and Conquer algorithm.
- Divides array into halves recursively.
- Merges sorted halves.

Example:

        38 27 43 3

       /          \
    38 27        43 3

    /   \        /   \
   38  27      43    3

Merged:
3 27 38 43

Time Complexity:
O(n log n)

-----------------------------------------------------------

3. OpenMP:
--------------------------------
- OpenMP is used for parallel programming.
- Multiple threads execute simultaneously.

Important Directives:
1. #pragma omp parallel for
2. #pragma omp parallel sections

-----------------------------------------------------------
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <omp.h>

using namespace std;

// =====================================================
// DISPLAY ARRAY
// =====================================================

void display(vector<int>& arr) {

    for (int x : arr)
        cout << x << " ";

    cout << endl;
}

// =====================================================
// SEQUENTIAL BUBBLE SORT
// =====================================================

void sequentialBubbleSort(vector<int>& arr) {

    int n = arr.size();

    for (int i = 0; i < n - 1; i++) {

        for (int j = 0; j < n - i - 1; j++) {

            if (arr[j] > arr[j + 1]) {

                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// =====================================================
// PARALLEL BUBBLE SORT
// =====================================================

void parallelBubbleSort(vector<int>& arr) {

    int n = arr.size();

    for (int i = 0; i < n; i++) {

        // Odd phase
        #pragma omp parallel for
        for (int j = 1; j < n - 1; j += 2) {

            if (arr[j] > arr[j + 1]) {

                swap(arr[j], arr[j + 1]);
            }
        }

        // Even phase
        #pragma omp parallel for
        for (int j = 0; j < n - 1; j += 2) {

            if (arr[j] > arr[j + 1]) {

                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// =====================================================
// MERGE FUNCTION
// =====================================================

void merge(vector<int>& arr, int left, int mid, int right) {

    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];

    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {

        if (L[i] <= R[j]) {

            arr[k] = L[i];
            i++;
        }
        else {

            arr[k] = R[j];
            j++;
        }

        k++;
    }

    while (i < n1) {

        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {

        arr[k] = R[j];
        j++;
        k++;
    }
}

// =====================================================
// SEQUENTIAL MERGE SORT
// =====================================================

void sequentialMergeSort(vector<int>& arr, int left, int right) {

    if (left >= right)
        return;

    int mid = (left + right) / 2;

    sequentialMergeSort(arr, left, mid);
    sequentialMergeSort(arr, mid + 1, right);

    merge(arr, left, mid, right);
}

// =====================================================
// PARALLEL MERGE SORT
// =====================================================

void parallelMergeSort(vector<int>& arr, int left, int right) {

    if (left >= right)
        return;

    int mid = (left + right) / 2;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            parallelMergeSort(arr, left, mid);
        }

        #pragma omp section
        {
            parallelMergeSort(arr, mid + 1, right);
        }
    }

    merge(arr, left, mid, right);
}

// =====================================================
// MAIN FUNCTION
// =====================================================

int main() {

    int n;

    cout << "Enter number of elements: ";
    cin >> n;

    vector<int> arr(n);

    cout << "\nEnter elements:\n";

    for (int i = 0; i < n; i++) {

        cin >> arr[i];
    }

    // Arrays for sorting
    vector<int> bubbleSeq = arr;
    vector<int> bubblePar = arr;
    vector<int> mergeSeq = arr;
    vector<int> mergePar = arr;

    // =================================================
    // SEQUENTIAL BUBBLE SORT
    // =================================================

    double start = omp_get_wtime();

    sequentialBubbleSort(bubbleSeq);

    double end = omp_get_wtime();

    cout << "\nSequential Bubble Sort:\n";
    display(bubbleSeq);

    cout << "Time taken: "
         << end - start
         << " seconds\n";

    // =================================================
    // PARALLEL BUBBLE SORT
    // =================================================

    start = omp_get_wtime();

    parallelBubbleSort(bubblePar);

    end = omp_get_wtime();

    cout << "\nParallel Bubble Sort:\n";
    display(bubblePar);

    cout << "Time taken: "
         << end - start
         << " seconds\n";

    // =================================================
    // SEQUENTIAL MERGE SORT
    // =================================================

    start = omp_get_wtime();

    sequentialMergeSort(mergeSeq, 0, n - 1);

    end = omp_get_wtime();

    cout << "\nSequential Merge Sort:\n";
    display(mergeSeq);

    cout << "Time taken: "
         << end - start
         << " seconds\n";

    // =================================================
    // PARALLEL MERGE SORT
    // =================================================

    start = omp_get_wtime();

    parallelMergeSort(mergePar, 0, n - 1);

    end = omp_get_wtime();

    cout << "\nParallel Merge Sort:\n";
    display(mergePar);

    cout << "Time taken: "
         << end - start
         << " seconds\n";

    return 0;
}

 /* 
 cd "C:\Users\IFFA\OneDrive\Desktop\HPC practicals"
 g++ -fopenmp hpc2.cpp -o practical.exe
 .\practical.exe
 */       

/*
Enter number of elements: 6

Enter elements:
5 2 8 1 9 3

Sequential Bubble Sort:
1 2 3 5 8 9 
Time taken: 0 seconds

Parallel Bubble Sort:
1 2 3 5 8 9 
Time taken: 0.00400019 seconds

Sequential Merge Sort:
1 2 3 5 8 9 
Time taken: 0 seconds

Parallel Merge Sort:
1 2 3 5 8 9 
Time taken: 0.000999928 seconds
*/
