/*
===========================================================
PARALLEL REDUCTION USING OPENMP
(MIN, MAX, SUM AND AVERAGE)
===========================================================

AIM:
To implement Min, Max, Sum and Average operations
using Parallel Reduction in OpenMP.

-----------------------------------------------------------
THEORY:
-----------------------------------------------------------

1. Reduction:
--------------------------------
Reduction combines multiple values into one result.

Examples:
- Sum of elements
- Minimum value
- Maximum value
- Average value

-----------------------------------------------------------

2. OpenMP Reduction Syntax:
--------------------------------

#pragma omp parallel for reduction(operation:variable)

Example:
#pragma omp parallel for reduction(+:sum)

-----------------------------------------------------------

3. Formula:
--------------------------------

Average = Sum / Number of Elements

-----------------------------------------------------------

4. Time Complexity:
--------------------------------

O(n)

-----------------------------------------------------------
*/

#include <iostream>
#include <vector>
#include <omp.h>
#include <climits>

using namespace std;

// =====================================================
// PARALLEL REDUCTION FUNCTION
// =====================================================

void parallelReduction(const vector<int>& arr) {

    int min_val = INT_MAX;
    int max_val = INT_MIN;

    long long sum = 0;

    int n = arr.size();

    // =================================================
    // PARALLEL REDUCTION
    // =================================================

    #pragma omp parallel for reduction(min:min_val) reduction(max:max_val) reduction(+:sum)
    for (int i = 0; i < n; i++) {

        if (arr[i] < min_val)
            min_val = arr[i];

        if (arr[i] > max_val)
            max_val = arr[i];

        sum += arr[i];
    }

    double average = (double)sum / n;

    // =================================================
    // DISPLAY RESULTS
    // =================================================

    cout << "\n--- Parallel Reduction Results ---\n";

    cout << "Minimum: " << min_val << endl;

    cout << "Maximum: " << max_val << endl;

    cout << "Sum:     " << sum << endl;

    cout << "Average: " << average << endl;
}

// =====================================================
// MAIN FUNCTION
// =====================================================

int main() {

    int N;

    cout << "Enter number of elements: ";
    cin >> N;

    vector<int> data(N);

    cout << "\nEnter elements:\n";

    for (int i = 0; i < N; i++) {

        cin >> data[i];
    }

    cout << "\nProcessing using Parallel Reduction...\n";

    // =================================================
    // TIME MEASUREMENT
    // =================================================

    double start = omp_get_wtime();

    parallelReduction(data);

    double end = omp_get_wtime();

    cout << "\nReduction Time: "
         << (end - start)
         << " seconds\n";

    return 0;
}


    /* 
    cd "C:\Users\IFFA\OneDrive\Desktop\HPC practicals"
    g++ -fopenmp hpc3.cpp -o reduction.exe
    .\reduction.exe
    */


/*
Enter number of elements: 5

Enter elements:
10 20 30 40 50

Processing using Parallel Reduction...

--- Parallel Reduction Results ---
Minimum: 10
Maximum: 50
Sum:     150
Average: 30

Reduction Time: 0.00399995 seconds  
*/