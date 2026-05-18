// Code-2 (Parallel Bubble Sort and Merge Sort)

/*
 * THIS CODE HAS BEEN TESTED AND IS FULLY OPERATIONAL.
 *
 * Problem Statement:
 *  Write a program to implement Parallel Bubble Sort and Merge sort using OpenMP.
 *  Use existing algorithms and measure the performance of sequential and parallel algorithms.
 *
 * Code from HighPerformanceComputing (SPPU - Final Year - Computer Engineering - Content)
 * repository on KSKA Git: https://git.kska.io/sppu-be-comp-content/HighPerformanceComputing
 **/

/*
 * EXECUTION INSTRUCTIONS (Debian-based distributions):
 *
 * i) Install g++ with OpenMP support:
 *   sudo apt update
 *   sudo apt install g++
 *
 * ii) Compile:
 *   g++ -fopenmp Code-2.cpp -o Code-2
 *
 * iii) Execute:
 *   ./Code-2
 **/

// BEGINNING OF CODE
#include <iostream>
#include <vector>
#include <cstdlib>
#include <omp.h>

using namespace std;

void printArray(const vector<int>& arr) {
    for (int num : arr)
        cout << num << " ";
    cout << endl;
}

// Bubble Sort

// Sequential bubble sort.
// Sorts the array using bubble sort by repeatedly swapping adjacent elements.
void sequentialBubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}

// Parallel bubble sort using odd-even transposition.
// Standard bubble sort cannot be parallelized directly: thread on index j
// and thread on index j+1 would both touch arr[j+1] simultaneously (data race).
// Odd-even transposition alternates between two phases each pass:
//   Phase 0 (even): compare pairs (0,1), (2,3), (4,5), ...
//   Phase 1 (odd):  compare pairs (1,2), (3,4), (5,6), ...
// Within each phase every pair is independent, so threads never share elements.
void parallelBubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n; i++) {
        // i % 2 selects even phase (0) or odd phase (1).
        // The starting index of the first pair in each phase matches i % 2.
        #pragma omp parallel for
        for (int j = i % 2; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}

// Merge Sort

// Merges two sorted halves arr[left..mid] and arr[mid+1..right] in place.
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void sequentialMergeSort(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sequentialMergeSort(arr, left, mid);
    sequentialMergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// Parallel merge sort using OpenMP tasks.
// "#pragma omp parallel sections" inside a recursive function would spawn a
// new thread team at every level of recursion, hundreds of thousands of teams
// for a large array, causing enormous overhead and likely a crash.
// Tasks are lighter: the runtime schedules them across an existing thread pool.
// The depth cutoff switches to sequential below a threshold to avoid spawning
// tasks so small that the overhead exceeds the work itself.
void parallelMergeSortHelper(vector<int>& arr, int left, int right, int depth) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;

    if (depth <= 0) {
        // Below the cutoff the subarray is small enough that sequential is faster.
        sequentialMergeSort(arr, left, mid);
        sequentialMergeSort(arr, mid + 1, right);
    } else {
        #pragma omp task
        parallelMergeSortHelper(arr, left, mid, depth - 1);

        #pragma omp task
        parallelMergeSortHelper(arr, mid + 1, right, depth - 1);

        // Wait for both tasks to finish before merging.
        #pragma omp taskwait
    }

    merge(arr, left, mid, right);
}

void parallelMergeSort(vector<int>& arr, int left, int right) {
    // The single directive creates one thread team for the entire sort.
    // All recursive tasks share this pool instead of creating new teams.
    #pragma omp parallel
    {
        // single ensures only one thread kicks off the root task;
        // the rest wait and pick up the child tasks as they are created.
        #pragma omp single
        parallelMergeSortHelper(arr, left, right, 4); // depth 4 → up to 16 parallel tasks
    }
}

// Main function

int main() {
    int n = 10000; // Adjust this to specify the number of elements.
    vector<int> arr(n);

    for (int i = 0; i < n; i++)
        arr[i] = rand() % 10000;

    double start, end;
    double time_seq_bubble, time_par_bubble;
    double time_seq_merge, time_par_merge;

    // --- Sequential Bubble Sort ---
    vector<int> seqArr = arr;
    start = omp_get_wtime();
    sequentialBubbleSort(seqArr);
    end = omp_get_wtime();
    time_seq_bubble = end - start;
    cout << "Sequential Bubble Sort time: " << time_seq_bubble << " seconds" << endl;

    // --- Parallel Bubble Sort ---
    vector<int> parArr = arr;
    start = omp_get_wtime();
    parallelBubbleSort(parArr);
    end = omp_get_wtime();
    time_par_bubble = end - start;
    cout << "Parallel Bubble Sort time: " << time_par_bubble << " seconds" << endl;

    cout << "Bubble Sort Speedup (Sequential / Parallel) = " << (time_seq_bubble / time_par_bubble) << "x" << endl;

    // --- Sequential Merge Sort ---
    seqArr = arr;
    start = omp_get_wtime();
    sequentialMergeSort(seqArr, 0, n - 1);
    end = omp_get_wtime();
    time_seq_merge = end - start;
    cout << "\nSequential Merge Sort time: " << time_seq_merge << " seconds" << endl;

    // --- Parallel Merge Sort ---
    parArr = arr;
    start = omp_get_wtime();
    parallelMergeSort(parArr, 0, n - 1);
    end = omp_get_wtime();
    time_par_merge = end - start;
    cout << "Parallel Merge Sort time: " << time_par_merge << " seconds" << endl;

    cout << "Merge Sort Speedup (Sequential / Parallel) = " << (time_seq_merge / time_par_merge) << "x" << endl;

    return 0;
}
/*# Theory: Parallel Breadth First Search (BFS) and Depth First Search (DFS) using OpenMP

Graph traversal algorithms are used to visit all vertices (nodes) of a graph or tree.

The two most common traversal techniques are:

1. Breadth First Search (BFS)
2. Depth First Search (DFS)

Using OpenMP, these traversals can be parallelized to improve execution speed on multi-core processors.

---

# 1. What is a Graph?

A graph consists of:

* Vertices (nodes)
* Edges (connections between nodes)

Example:

```text id="sylzpm"
0 --- 1
|     |
2 --- 3
```

Graph representation:

```cpp id="jlwmhe"
0 -> 1,2
1 -> 0,3
2 -> 0,3
3 -> 1,2
```

---

# 2. Breadth First Search (BFS)

## Definition

Breadth First Search visits nodes level by level.

It first visits all neighboring nodes before moving deeper.

It uses a **queue** data structure.

---

## BFS Traversal Example

Graph:

```text id="56h0d5"
    0
   / \
  1   2
 / \
3   4
```

Starting from node 0:

Traversal order:

```text id="b0ckm6"
0 → 1 → 2 → 3 → 4
```

---

# 3. Sequential BFS Algorithm

Steps:

1. Start from source node
2. Mark node as visited
3. Push into queue
4. Remove node from queue
5. Visit all unvisited neighbors
6. Repeat until queue becomes empty

Pseudo-code:

```cpp id="2fk0s5"
queue.push(start);

while(!queue.empty()) {
    node = queue.front();
    queue.pop();

    for(all neighbors) {
        if(not visited) {
            visited = true;
            queue.push(neighbor);
        }
    }
}
```

Time complexity:

```text id="n7v3ns"
O(V + E)
```

Where:

* V = vertices
* E = edges

---

# 4. Parallel BFS using OpenMP

## Idea

Neighbor exploration can be done in parallel.

Different threads process different adjacent nodes simultaneously.

---

## Working

Suppose node 0 has neighbors:

```text id="n5pyqk"
1, 2, 3, 4
```

Instead of processing sequentially:

```text id="4mwsdo"
Thread 1 → node 1
Thread 2 → node 2
Thread 3 → node 3
Thread 4 → node 4
```

This speeds up traversal for large graphs.

---

# 5. OpenMP Parallel BFS

Example:

```cpp id="ud3z86"
#pragma omp parallel for
for(int i = 0; i < adj[node].size(); i++) {
    int neighbor = adj[node][i];

    if(!visited[neighbor]) {
        visited[neighbor] = true;
        queue.push(neighbor);
    }
}
```

---

# 6. Problems in Parallel BFS

## Race Condition

Two threads may try to:

* Visit same node
* Modify queue simultaneously

Example:

```cpp id="76j9d7"
visited[neighbor] = true;
```

Both threads may access it together.

---

## Synchronization Needed

Critical sections are used:

```cpp id="jxk9dj"
#pragma omp critical
{
    queue.push(neighbor);
}
```

This ensures safe shared memory access.

---

# 7. Advantages of Parallel BFS

## Faster Traversal

Multiple nodes processed simultaneously.

## Better CPU Utilization

All CPU cores participate.

## Efficient for Large Graphs

Useful in:

* Social networks
* Web crawling
* AI search
* Network routing

---

# 8. Depth First Search (DFS)

## Definition

Depth First Search explores as deep as possible before backtracking.

It uses a:

* Stack
  or
* Recursion

---

## DFS Traversal Example

Graph:

```text id="g10r7v"
    0
   / \
  1   2
 / \
3   4
```

Traversal:

```text id="9wfgkn"
0 → 1 → 3 → 4 → 2
```

---

# 9. Sequential DFS Algorithm

Pseudo-code:

```cpp id="b6pvpo"
DFS(node) {
    visited[node] = true;

    for(all neighbors) {
        if(not visited)
            DFS(neighbor);
    }
}
```

Time complexity:

```text id="5xvvk8"
O(V + E)
```

---

# 10. Parallel DFS using OpenMP

## Idea

Recursive calls for neighbors can execute in parallel.

Different branches of DFS tree are explored simultaneously.

---

## OpenMP Parallel DFS

Example:

```cpp id="wz30rw"
#pragma omp parallel for
for(int i = 0; i < adj[node].size(); i++) {

    int neighbor = adj[node][i];

    if(!visited[neighbor]) {
        DFS(neighbor);
    }
}
```

---

# 11. Why Parallel DFS is Difficult

DFS is naturally sequential because:

* Next node depends on previous traversal
* Stack/recursion creates dependency

Problems:

* Load imbalance
* Thread synchronization
* Recursive conflicts

Hence BFS is usually easier to parallelize than DFS.

---

# 12. BFS vs DFS

---

# 13. Applications of BFS

* Shortest path algorithms
* GPS navigation
* Web crawlers
* Social media analysis
* Network broadcasting

---

# 14. Applications of DFS

* Cycle detection
* Topological sorting
* Maze solving
* Backtracking problems
* AI game trees

---

# 15. Role of OpenMP

OpenMP helps parallelize traversal using:

```cpp id="nv0v33"
#pragma omp parallel
#pragma omp parallel for
#pragma omp critical
#pragma omp task
```

Features:

* Multi-threading
* Shared memory parallelism
* Automatic workload distribution

---

# 16. Advantages of Parallel Graph Traversal

## Reduced Execution Time

Large graphs processed faster.

## Scalability

Performance improves with more CPU cores.

## Efficient Resource Usage

Better utilization of modern processors.

---

# 17. Limitations

## Synchronization Overhead

Threads require coordination.

## Race Conditions

Shared data structures need protection.

## DFS Dependency Issues

DFS is harder to parallelize efficiently.

## Small Graphs

Parallel overhead may reduce performance gains.

---

# 18. Conclusion

Parallel BFS and DFS improve graph traversal performance using multiple threads.

* BFS is easier and more efficient to parallelize.
* DFS parallelization is more complex due to recursive dependencies.

Using OpenMP:

* Workload is divided among threads
* Traversal becomes faster for large graphs
* Synchronization ensures correctness

These algorithms are important in:

* Artificial Intelligence
* Big Data
* Networking
* Scientific Computing
* Parallel Processing Systems


BFS vs DFS Characteristics

Comparison of Breadth First Search and Depth First Search in graph traversal.

feature	dfs	bfs
Uses Queue	0	1
Uses Stack/Recursion	1	0
Level-wise Traversal	0	1
Deep Traversal	1	0
Easy Parallelization	0	1
Memory Usage High	0	1*/