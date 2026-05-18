#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <climits>

using namespace std;

void parallelReduction(const vector<int>& arr) {
    int n = arr.size();
    
    int min_val = INT_MAX;
    int max_val = INT_MIN;
    long long sum = 0;

    // OpenMP handles the splitting, local computation, and merging automatically
    #pragma omp parallel for reduction(min:min_val) reduction(max:max_val) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
        sum += arr[i];
    }

    double average = static_cast<double>(sum) / n;

    cout << "--- Parallel Reduction Results ---" << endl;
    cout << "Minimum: " << min_val << endl;
    cout << "Maximum: " << max_val << endl;
    cout << "Sum:     " << sum << endl;
    cout << "Average: " << average << endl;
}

int main() {
    int N = 1000000; // 1 Million elements
    vector<int> data(N);
    
    for (int i = 0; i < N; i++) {
        data[i] = i + 1; // Array of 1 to 1,000,000
    }

    double start = omp_get_wtime();
    parallelReduction(data);
    double end = omp_get_wtime();

    cout << "Reduction Time: " << (end - start) << " seconds" << endl;

    return 0;
}

// --- PARALLEL REDUCTION COMPLEXITY ---
// Theoretical Span: O(log n), representing the height of the reduction tree.
// Practically, following the Work-Span model, the complexity is O( n / P + log n ).
// Note: According to Amdahl's Law, the sequential fraction (s) consists of the 
// final steps of combining local thread results into the global total. 
// Because log(n) is very small, reduction typically shows the highest parallel 
// efficiency and speedup of all parallel patterns.
// (P = processors, n = number of elements)