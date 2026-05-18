#include <iostream>
#include <omp.h>
#include <vector>
using namespace std;

int main() {
    // Uncomment to manually control thread count
    // omp_set_num_threads(4);

    // --- Input ---
    int n = 1000000;
    vector<int> nums(n);

    for (int i = 0; i < n; i++)
        nums[i] = rand() % 10000;

    cout << "Input: " << n << " random integers in the range [0, 9999]." << endl << endl;

    // long long prevents overflow: up to 1,000,000 * 9,999 ≈ 10 billion,
    // which exceeds the int limit of ~2.1 billion.
    long long sum_seq, sum_par;
    int min_seq, max_seq;
    int min_par, max_par;
    double avg_seq, avg_par;
    double start, end;

    // --- Sequential ---
    min_seq = max_seq = nums[0];
    sum_seq = 0;

    start = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        if (nums[i] < min_seq) min_seq = nums[i];
        if (nums[i] > max_seq) max_seq = nums[i];
        sum_seq += nums[i];
    }
    end = omp_get_wtime();

    // Computed after timing so both versions are measured fairly.
    avg_seq = (double)sum_seq / n;
    double time_seq = end - start;

    // --- Parallel ---
    min_par = max_par = nums[0];
    sum_par = 0;

    start = omp_get_wtime();
    // reduction(min/max/+) gives each thread its own private copy of the
    // variable, then combines them at the end, no critical sections needed.
    // Without reduction, threads would race to update the same variable.
    #pragma omp parallel for reduction(min: min_par) reduction(max: max_par) reduction(+: sum_par)
    for (int i = 0; i < n; i++) {
        if (nums[i] < min_par) min_par = nums[i];
        if (nums[i] > max_par) max_par = nums[i];
        sum_par += nums[i];
    }
    end = omp_get_wtime();

    avg_par = (double)sum_par / n;
    double time_par = end - start;

    // --- Output ---
    cout << "--- Sequential Computation ---" << endl;
    cout << "Minimum  : " << min_seq << endl;
    cout << "Maximum  : " << max_seq << endl;
    cout << "Sum      : " << sum_seq << endl;
    cout << "Average  : " << avg_seq << endl;
    cout << "Time     : " << time_seq << " seconds" << endl;

    cout << "\n--- Parallel Computation ---" << endl;
    cout << "Minimum  : " << min_par << endl;
    cout << "Maximum  : " << max_par << endl;
    cout << "Sum      : " << sum_par << endl;
    cout << "Average  : " << avg_par << endl;
    cout << "Time     : " << time_par << " seconds" << endl;

    cout << "\nSpeedup (Sequential / Parallel) = " << (time_seq / time_par) << "x" << endl;

    return 0;
}

/*
## Theory: Parallel Reduction for Min, Max, Sum, and Average

Parallel reduction is a technique used in parallel programming to combine multiple values into a single result efficiently. Instead of processing elements one by one sequentially, the work is divided among multiple threads, and partial results are combined at the end.

In OpenMP, this is commonly done using the `reduction` clause.

---

# 1. What is Reduction?

A **reduction operation** takes a collection of values and reduces them to one final value using an operation such as:

* Addition (`+`) → Sum
* Minimum (`min`) → Minimum value
* Maximum (`max`) → Maximum value
* Average → Sum ÷ Number of elements

Example:

Array:

```text
[10, 20, 5, 40, 15]
```

Results:

```text
Sum = 90
Min = 5
Max = 40
Average = 18
```

---

# 2. Sequential vs Parallel Approach

## Sequential Method

In a normal sequential program:

* One CPU core processes all elements
* Each iteration runs one after another

Example for sum:

```cpp
sum = sum + arr[i];
```

Time complexity:

```text
O(n)
```

---

## Parallel Method

In parallel reduction:

* Array is divided among multiple threads
* Each thread computes a partial result
* Partial results are merged automatically

Example:

Thread 1:

```text
10 + 20 = 30
```

Thread 2:

```text
5 + 40 = 45
```

Thread 3:

```text
15
```

Final reduction:

```text
30 + 45 + 15 = 90
```

This improves performance for large datasets.

---

# 3. OpenMP Reduction Clause

OpenMP provides a built-in `reduction` clause.

Syntax:

```cpp
#pragma omp parallel for reduction(operator:variable)
```

Example:

```cpp
#pragma omp parallel for reduction(+:sum)
```

Meaning:

* Every thread gets its own private copy of `sum`
* Threads compute partial sums independently
* OpenMP combines all partial sums automatically at the end

---

# 4. Parallel Sum Operation

## Working

Each thread calculates part of the array sum.

Formula:

\text{Sum} = \sum_{i=0}^{n-1} a_i

Example:

```cpp
#pragma omp parallel for reduction(+:sum)
for(int i=0; i<n; i++) {
    sum += arr[i];
}
```

Advantages:

* Faster for large arrays
* Avoids race conditions
* Simple implementation

---

# 5. Parallel Minimum Operation

## Working

Each thread finds a local minimum.
OpenMP combines all local minima into the global minimum.

Formula:

\text{Min} = \min(a_0,a_1,a_2,\dots,a_{n-1})

Example:

```cpp
#pragma omp parallel for reduction(min:minValue)
for(int i=0; i<n; i++) {
    if(arr[i] < minValue)
        minValue = arr[i];
}
```

---

# 6. Parallel Maximum Operation

## Working

Each thread computes a local maximum.

Formula:

\text{Max} = \max(a_0,a_1,a_2,\dots,a_{n-1})

Example:

```cpp
#pragma omp parallel for reduction(max:maxValue)
for(int i=0; i<n; i++) {
    if(arr[i] > maxValue)
        maxValue = arr[i];
}
```

---

# 7. Parallel Average Operation

Average is calculated using:

\text{Average} = \frac{\sum_{i=0}^{n-1} a_i}{n}

Steps:

1. Compute parallel sum
2. Divide by total number of elements

Example:

```cpp
average = (double)sum / n;
```

---

# 8. Why Reduction is Needed

Without reduction, multiple threads may try to modify the same variable simultaneously.

This causes:

* Incorrect outputs
* Data inconsistency
* Race conditions

Example problem:

```cpp
sum += arr[i];
```

If two threads update `sum` at the same time, one update may be lost.

Reduction solves this by:

* Giving each thread a private copy
* Combining results safely afterward

---

# 9. Advantages of Parallel Reduction

## Faster Execution

Work is distributed among CPU cores.

## Better CPU Utilization

Multiple cores execute simultaneously.

## Scalability

Performance improves with larger datasets and more cores.

## Simpler Parallel Programming

OpenMP handles synchronization internally.

---

# 10. Limitations

## Small Arrays

Parallel overhead may make execution slower than sequential execution.

## Thread Creation Cost

Creating threads requires additional time.

## Hardware Dependency

Performance depends on number of CPU cores.

---

# 11. Applications of Parallel Reduction

Parallel reduction is widely used in:

* Scientific computing
* Machine learning
* Data analytics
* Image processing
* Matrix operations
* GPU programming
* Big data processing

---

# 12. Conclusion

Parallel reduction is an efficient parallel programming technique used to compute aggregate operations such as:

* Minimum
* Maximum
* Sum
* Average

Using OpenMP reduction clauses:

* Improves execution speed
* Prevents race conditions
* Simplifies multithreaded programming

It is one of the most fundamental concepts in parallel computing and high-performance applications.

Your program correctly implements **Parallel Reduction Operations using OpenMP** for:

* Sum
* Minimum
* Maximum
* Average

This matches the problem statement:

> “Implement Min, Max, Sum and Average operations using Parallel Reduction.”

---

# Theory / Viva Explanation

## What is OpenMP?

OpenMP is an API used for **parallel programming** on shared-memory systems.

It allows multiple threads to execute parts of a program simultaneously using compiler directives like:

```cpp
#pragma omp
```

---

# What is Parallel Computing?

Parallel computing means:

> Breaking a task into smaller parts and executing them simultaneously using multiple processors/threads.

Example:

Instead of one CPU adding all array elements one by one:

```text
10 + 20 + 5 + 40 + 15
```

multiple threads divide the work:

```text
Thread 0 → 10 + 20
Thread 1 → 5 + 40
Thread 2 → 15
```

Then results are combined.

This improves performance for large data.

---

# What is Reduction in OpenMP?

Reduction is an OpenMP technique used when:

> Multiple threads work on a common variable safely.

Without reduction:

```cpp
sum += arr[i];
```

causes **race condition** because many threads modify `sum` simultaneously.

Reduction creates:

* private copy for each thread
* combines results at end

Syntax:

```cpp
reduction(operator:variable)
```

Example:

```cpp
reduction(+:sum)
```

means:

* each thread keeps local `sum`
* OpenMP adds them together at end

---

# Types of Reduction Used

| Reduction | Meaning       |
| --------- | ------------- |
| `+`       | Addition      |
| `min`     | Minimum value |
| `max`     | Maximum value |

---

# Header Files Used

## iostream

```cpp
#include <iostream>
```

Used for input/output operations.

Functions:

* `cout`
* `cin`

---

## omp.h

```cpp
#include <omp.h>
```

Required for OpenMP functions.

Used for:

* thread management
* timing
* thread ID

Example:

```cpp
omp_get_thread_num()
```

returns current thread number.

---

# Important OpenMP Directive

## `#pragma omp parallel for`

This directive tells compiler:

> Execute loop iterations in parallel using multiple threads.

Example:

```cpp
#pragma omp parallel for
for(int i=0;i<n;i++)
```

Different threads execute different iterations.

---

# Race Condition

A race condition occurs when:

> Multiple threads access and modify same variable simultaneously causing unpredictable results.

Example:

```cpp
sum += arr[i];
```

Without reduction:

* Thread 0 updates sum
* Thread 1 updates sum same time
* one update may be lost

Reduction solves this problem.

---

# Sequential vs Parallel Execution

| Sequential            | Parallel                 |
| --------------------- | ------------------------ |
| One thread            | Multiple threads         |
| Slower for large data | Faster                   |
| Simple                | Requires synchronization |
| No race conditions    | Race conditions possible |

---

# Code Explanation

---

## Array Initialization

```cpp
int arr[5] = {10, 20, 5, 40, 15};
```

Stores array elements.

---

## Sum Variables

```cpp
int sum = 0;
int minValue = arr[0];
int maxValue = arr[0];
```

Initial values for reduction operations.

---

# Parallel Sum

```cpp
#pragma omp parallel for reduction(+:sum)
```

Creates parallel threads.

Each thread computes partial sum.

At end:

```text
Final Sum = Sum of all thread sums
```

---

## Thread Display

```cpp
omp_get_thread_num()
```

Returns current thread ID.

Useful for visualization/debugging.

Example output:

```text
SUM -> Thread 1 processing arr[2] = 5
```

---

# Parallel Minimum

```cpp
reduction(min:minValue)
```

Each thread finds local minimum.

OpenMP combines all local minimums.

---

# Parallel Maximum

```cpp
reduction(max:maxValue)
```

Each thread finds local maximum.

OpenMP combines all local maximums.

---

# Average Calculation

```cpp
double average = (double)sum / n;
```

Formula:

\text{Average} = \frac{\text{Sum}}{n}

Type casting:

```cpp
(double)
```

avoids integer division.

---

# Time Complexity

| Operation | Complexity     |
| --------- | -------------- |
| Sum       | O(n)           |
| Min       | O(n)           |
| Max       | O(n)           |
| Average   | O(1) after sum |

Parallel execution reduces effective execution time.

---

# Advantages of OpenMP

* Easy parallel programming
* Simple directives
* Faster execution
* Shared memory support
* Minimal code changes

---

# Limitations

* Works mainly on shared-memory systems
* Thread creation overhead
* Race conditions possible
* Not efficient for very small tasks

---

# Expected Output

```text
Array Elements: 10 20 5 40 15

Sum = 90
Minimum = 5
Maximum = 40
Average = 18
```

Thread order may change every run because scheduling is parallel.

---

# Viva Questions & Answers

## What is OpenMP?

OpenMP is an API for shared-memory parallel programming in C, C++, and Fortran.

---

## What is reduction in OpenMP?

Reduction combines results from multiple threads into a single variable safely.

---

## Why is reduction needed?

To avoid race conditions while updating shared variables.

---

## What is race condition?

When multiple threads modify same data simultaneously causing incorrect results.

---

## Difference between parallel and sequential execution?

Sequential uses one thread; parallel uses multiple threads simultaneously.

---

## What does `omp_get_thread_num()` do?

Returns current executing thread ID.

---

## Why use `(double)sum`?

To perform floating-point division instead of integer division.

*/