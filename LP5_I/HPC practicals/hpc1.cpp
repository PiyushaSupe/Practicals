/*
===========================================================
PARALLEL BREADTH FIRST SEARCH (BFS) AND DEPTH FIRST SEARCH (DFS)
USING OPENMP
===========================================================

AIM:
To design and implement Parallel Breadth First Search (BFS)
and Depth First Search (DFS) using OpenMP on an undirected graph.

-----------------------------------------------------------
THEORY:
-----------------------------------------------------------

1. Breadth First Search (BFS):
--------------------------------
- BFS traverses a graph level by level.
- It uses a Queue data structure.
- First all neighbors are visited, then next level nodes.

Example Graph:

            0
          /   \
         1     2
        / \     \
       3   4     5

BFS Traversal:
0 → 1 → 2 → 3 → 4 → 5

-----------------------------------------------------------

2. Depth First Search (DFS):
--------------------------------
- DFS traverses deeply before backtracking.
- It uses Recursion or Stack.
- One branch is completely explored before moving to another.

DFS Traversal:
0 → 1 → 3 → 4 → 2 → 5

-----------------------------------------------------------

3. OpenMP:
--------------------------------
- OpenMP is an API for parallel programming in C/C++.
- It creates multiple threads for parallel execution.

Important Directives:
1. #pragma omp parallel for
   -> Executes loop iterations in parallel.

2. #pragma omp critical
   -> Allows only one thread at a time to access shared resources.

-----------------------------------------------------------

4. Time Complexity:
--------------------------------

BFS Complexity:
O(V + E)

DFS Complexity:
O(V + E)

Where:
V = Number of Vertices
E = Number of Edges

-----------------------------------------------------------
*/

#include <iostream>
#include <vector>
#include <queue>
#include <omp.h>

using namespace std;

class Graph {

    int vertices;
    vector<vector<int>> adj;

public:

    // Constructor
    Graph(int v) {
        vertices = v;
        adj.resize(v);
    }

    // Function to add edge
    void addEdge(int u, int v) {

        adj[u].push_back(v);
        adj[v].push_back(u); // Undirected Graph
    }

    // =====================================================
    // SERIAL BFS
    // =====================================================

    void serialBFS(int start) {

        vector<bool> visited(vertices, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        cout << "\nSerial BFS Traversal: ";

        double start_time = omp_get_wtime();

        while (!q.empty()) {

            int current = q.front();
            q.pop();

            cout << current << " ";

            for (int neighbor : adj[current]) {

                if (!visited[neighbor]) {

                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }

        double end_time = omp_get_wtime();

        cout << "\nTime taken for Serial BFS: "
             << end_time - start_time
             << " seconds\n";
    }

    // =====================================================
    // PARALLEL BFS
    // =====================================================

    void parallelBFS(int start) {

        vector<bool> visited(vertices, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        cout << "\nParallel BFS Traversal: ";

        double start_time = omp_get_wtime();

        while (!q.empty()) {

            int current;

            #pragma omp critical
            {
                current = q.front();
                q.pop();
            }

            cout << current << " ";

            #pragma omp parallel for
            for (int i = 0; i < adj[current].size(); i++) {

                int neighbor = adj[current][i];

                if (!visited[neighbor]) {

                    #pragma omp critical
                    {
                        if (!visited[neighbor]) {

                            visited[neighbor] = true;
                            q.push(neighbor);
                        }
                    }
                }
            }
        }

        double end_time = omp_get_wtime();

        cout << "\nTime taken for Parallel BFS: "
             << end_time - start_time
             << " seconds\n";
    }

    // =====================================================
    // SERIAL DFS
    // =====================================================

    void serialDFSUtil(int node, vector<bool>& visited) {

        visited[node] = true;

        cout << node << " ";

        for (int neighbor : adj[node]) {

            if (!visited[neighbor]) {

                serialDFSUtil(neighbor, visited);
            }
        }
    }

    void serialDFS(int start) {

        vector<bool> visited(vertices, false);

        cout << "\nSerial DFS Traversal: ";

        double start_time = omp_get_wtime();

        serialDFSUtil(start, visited);

        double end_time = omp_get_wtime();

        cout << "\nTime taken for Serial DFS: "
             << end_time - start_time
             << " seconds\n";
    }

    // =====================================================
    // PARALLEL DFS
    // =====================================================

    void parallelDFSUtil(int node, vector<bool>& visited) {

        bool alreadyVisited = false;

        #pragma omp critical
        {
            if (visited[node]) {
                alreadyVisited = true;
            }
            else {

                visited[node] = true;

                cout << node << " ";
            }
        }

        if (alreadyVisited)
            return;

        #pragma omp parallel for
        for (int i = 0; i < adj[node].size(); i++) {

            int neighbor = adj[node][i];

            if (!visited[neighbor]) {

                parallelDFSUtil(neighbor, visited);
            }
        }
    }

    void parallelDFS(int start) {

        vector<bool> visited(vertices, false);

        cout << "\nParallel DFS Traversal: ";

        double start_time = omp_get_wtime();

        parallelDFSUtil(start, visited);

        double end_time = omp_get_wtime();

        cout << "\nTime taken for Parallel DFS: "
             << end_time - start_time
             << " seconds\n";
    }
};

// =====================================================
// MAIN FUNCTION
// =====================================================

int main() {

    int vertices, edges;

    cout << "Enter number of vertices: ";
    cin >> vertices;

    Graph g(vertices);

    cout << "Enter number of edges: ";
    cin >> edges;

    cout << "\nEnter edges (u v):\n";

    for (int i = 0; i < edges; i++) {

        int u, v;
        cin >> u >> v;

        g.addEdge(u, v);
    }

    int startNode;

    cout << "\nEnter starting node: ";
    cin >> startNode;

    // BFS
    g.serialBFS(startNode);
    g.parallelBFS(startNode);

    // DFS
    g.serialDFS(startNode);
    g.parallelDFS(startNode);

    return 0;
}


//output


 /* cd "C:\Users\IFFA\OneDrive\Desktop\HPC practicals"
 g++ -fopenmp hpc1.cpp -o practical.exe
 .\practical.exe*/       

 
/*Enter number of vertices: 6
Enter number of edges: 5

Enter edges (u v):
0 1
0 2     
1 3
1 4
2 5

Enter starting node: 0

Serial BFS Traversal: 0 1 2 3 4 5 
Time taken for Serial BFS: 0.00100017 seconds

Parallel BFS Traversal: 0 2 1 5 4 3 
Time taken for Parallel BFS: 0.00299978 seconds

Serial DFS Traversal: 0 1 3 4 2 5 
Time taken for Serial DFS: 0.00100017 seconds

Parallel DFS Traversal: 0 2 5 1 3 4 
Time taken for Parallel DFS: 0 seconds*/

                          

