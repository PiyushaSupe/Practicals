#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

// Undirected graph with parallel BFS and DFS traversal via OpenMP.
class Graph {
    int V;
    vector<vector<int>> adj;

public:
    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Level-synchronous BFS: all nodes at the current depth (the "frontier")
    // are expanded in parallel before moving to the next level. This is the
    // natural unit of parallelism for BFS, processing individual nodes is too
    // fine-grained for threads to be useful.
    void parallelBFS(int start) {
        vector<bool> visited(V, false);
        vector<int> frontier;

        visited[start] = true;
        frontier.push_back(start);

        cout << "Parallel BFS from node " << start << ": ";

        while (!frontier.empty()) {
            for (int u : frontier)
                cout << u << " ";

            vector<int> next_frontier;

            // Each thread accumulates its own local candidates to avoid
            // contention on a shared next_frontier vector.
            #pragma omp parallel
            {
                vector<int> local_next;

                // nowait: threads that finish early skip the implicit barrier
                // and proceed directly to the merge below.
                // schedule(dynamic): faster threads pick up remaining chunks
                // when adjacency list sizes vary across nodes.
                #pragma omp for nowait schedule(dynamic)
                for (int i = 0; i < (int)frontier.size(); i++) {
                    for (int v : adj[frontier[i]]) {
                        // The check-and-set on visited[] must be a single
                        // critical section — without it, two threads could
                        // both see visited[v]==false and both enqueue v,
                        // producing duplicates in the next frontier.
                        bool should_visit = false;
                        #pragma omp critical
                        {
                            if (!visited[v]) {
                                visited[v] = true;
                                should_visit = true;
                            }
                        }
                        // local_next is thread-private so no lock needed here.
                        if (should_visit)
                            local_next.push_back(v);
                    }
                }

                // Merge: one thread at a time appends its local results.
                // This is a separate critical section from the one above
                // so the two do not serialize against each other.
                #pragma omp critical
                {
                    next_frontier.insert(next_frontier.end(),
                                         local_next.begin(),
                                         local_next.end());
                }
            } // implicit barrier: all threads finish before frontier is swapped

            frontier = next_frontier;
        }

        cout << endl;
    }

    // Iterative DFS using a vector as a stack (push_back/pop_back).
    // vector is used instead of std::stack because std::stack cannot be
    // safely shared across threads.
    void parallelDFS(int start) {
        vector<bool> visited(V, false);
        vector<int> stack;

        stack.push_back(start);

        cout << "Parallel DFS from node " << start << ": ";

        while (!stack.empty()) {
            int u = stack.back();
            stack.pop_back();

            // A node may be pushed multiple times before it is marked visited
            // (two threads can both see visited[v]==false). This guard ensures
            // it is processed only once.
            if (visited[u]) continue;
            visited[u] = true;
            cout << u << " ";

            vector<int> to_push;

            #pragma omp parallel
            {
                vector<int> local_push;

                #pragma omp for nowait schedule(dynamic)
                for (int i = 0; i < (int)adj[u].size(); i++) {
                    // visited[] is only read here, not written, so no critical
                    // section is needed. Stale reads may cause duplicates but
                    // the guard above handles that safely.
                    if (!visited[adj[u][i]])
                        local_push.push_back(adj[u][i]);
                }

                #pragma omp critical
                {
                    to_push.insert(to_push.end(),
                                   local_push.begin(),
                                   local_push.end());
                }
            }

            for (int v : to_push)
                stack.push_back(v);
        }

        cout << endl;
    }
};

int main() {
    Graph g(6);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);

    g.parallelBFS(0);
    g.parallelDFS(0);

    return 0;
}
/*# ## Parallel BFS and DFS in Graphs

Graphs are widely used in computer science to represent networks such as social media connections, maps, web pages, and communication systems. Traversing a graph means visiting all its vertices systematically. Two common traversal techniques are:

* **Breadth First Search (BFS)**
* **Depth First Search (DFS)**

In parallel computing, these algorithms are modified so that multiple vertices can be processed simultaneously using multiple threads (for example, with OpenMP).

---

# 1. Parallel Breadth First Search (BFS)

## What is BFS?

BFS visits graph vertices **level by level**.

It starts from a source node:

1. Visit the source node
2. Visit all its neighbors
3. Visit neighbors of those neighbors
4. Continue until all nodes are visited

BFS generally uses a **queue** data structure.

---

## Example

Consider the graph:

```text
    0
   / \
  1   2
 / \   \
3   4   5
```

Starting BFS from node `0`:

```text
Level 0 → 0
Level 1 → 1, 2
Level 2 → 3, 4, 5
```

Traversal order:

```text
0 → 1 → 2 → 3 → 4 → 5
```

---

# Why Parallelize BFS?

In BFS, all nodes at the same level are independent.

For example:

* Nodes `1` and `2` can be processed simultaneously.
* Their neighbors can also be explored in parallel.

This makes BFS naturally suitable for parallel execution.

---

## Working of Parallel BFS

### Sequential BFS

```text
Take one node from queue
Visit neighbors one by one
Add unvisited neighbors to queue
Repeat
```

### Parallel BFS

```text
Process multiple nodes of current level together
Different threads explore different neighbors
Threads cooperate to build next level
```

---

## Parallel BFS using OpenMP

Typically:

* Multiple threads process vertices simultaneously
* Shared structures:

  * visited array
  * queue/frontier

OpenMP constructs used:

* `#pragma omp parallel for`
* Critical sections or atomic operations

---

## Advantages of Parallel BFS

* Faster traversal on large graphs
* Better CPU utilization
* Useful in:

  * shortest path problems
  * social network analysis
  * web crawling
  * AI search problems

---

## Challenges in Parallel BFS

### 1. Synchronization

Multiple threads may try to:

* visit same node
* update queue simultaneously

Requires:

* locks
* critical sections
* atomic operations

---

### 2. Load Balancing

Some nodes may have many neighbors while others have few.

Result:

* uneven workload among threads

---

### 3. Memory Overhead

Maintaining parallel queues/frontiers increases memory usage.

---

# 2. Parallel Depth First Search (DFS)

## What is DFS?

DFS explores a graph **deeply first** before backtracking.

It uses:

* recursion
* or stack

---

## Example

Using same graph:

```text
    0
   / \
  1   2
 / \   \
3   4   5
```

DFS starting from `0`:

```text
0 → 1 → 3 → 4 → 2 → 5
```

DFS goes deep into one branch before returning.

---

# Why Parallelize DFS?

DFS is harder to parallelize because traversal is inherently sequential.

However:

* Different branches/subtrees can sometimes be explored simultaneously.

Example:

After visiting node `0`:

* one thread explores subtree of `1`
* another explores subtree of `2`

---

## Working of Parallel DFS

### Sequential DFS

```text
Visit node
Recursively visit neighbors
Backtrack
```

### Parallel DFS

```text
Visit node
Spawn parallel tasks for different neighbors
Threads explore subtrees simultaneously
```

---

## Parallel DFS using OpenMP

Usually implemented using:

* OpenMP tasks

Example concept:

```cpp
#pragma omp task
DFS(neighbor);
```

Each recursive branch may execute in parallel.

---

## Advantages of Parallel DFS

* Faster traversal in large trees/graphs
* Efficient for divide-and-conquer problems
* Useful in:

  * game trees
  * AI search
  * maze solving
  * backtracking problems

---

## Challenges in Parallel DFS

### 1. Difficult Parallelization

DFS depends heavily on recursion and traversal order.

Not all parts can run independently.

---

### 2. Race Conditions

Multiple threads may access:

* stack
* visited array

Needs synchronization.

---

### 3. Irregular Workload

Some branches may be very deep while others are shallow.

Threads may become idle.

---

# Comparison: Parallel BFS vs Parallel DFS

| Feature                 | Parallel BFS            | Parallel DFS     |
| ----------------------- | ----------------------- | ---------------- |
| Traversal Style         | Level-wise              | Depth-wise       |
| Main Data Structure     | Queue                   | Stack/Recursion  |
| Ease of Parallelization | Easier                  | Harder           |
| Parallelism Type        | Frontier-based          | Task-based       |
| Synchronization Need    | Moderate                | High             |
| Load Balancing          | Better                  | More difficult   |
| Common OpenMP Construct | parallel for            | task             |
| Applications            | Shortest path, networks | AI, backtracking |

---

# Time Complexity

For both BFS and DFS:

```text
O(V + E)
```

Where:

* `V` = number of vertices
* `E` = number of edges

Parallel versions reduce execution time using multiple processors, although theoretical complexity remains similar.

---

# Role of OpenMP

OpenMP is commonly used for implementing parallel BFS and DFS because it:

* supports multithreading
* is easy to integrate with C/C++
* provides:

  * parallel loops
  * tasks
  * synchronization constructs

Common directives:

```cpp
#pragma omp parallel for
#pragma omp critical
#pragma omp task
#pragma omp atomic
```

---

# Real-World Applications

## BFS Applications

* GPS shortest path
* Social network friend suggestions
* Network broadcasting
* Web crawlers

## DFS Applications

* Cycle detection
* Topological sorting
* Maze solving
* Backtracking algorithms
* AI game trees

---

# Conclusion

* **Parallel BFS** is easier and more efficient because nodes at the same level can be processed simultaneously.
* **Parallel DFS** is more complex due to recursive and sequential nature, but parallel tasks can still improve performance.
* OpenMP helps implement both algorithms using multithreading techniques.
* Parallel graph traversal is important in modern high-performance computing and large-scale data processing systems.
*/