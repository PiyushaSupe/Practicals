/*
1. Design and implement Parallel Breadth First Search and Depth First Search based on existing
algorithms using OpenMP. Use a Tree or an undirected graph for BFS and DFS .
*/

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <chrono>
#include <omp.h>
using namespace std;

class Graph {
    // Bidrectional graph btw
    int V;
    vector<vector<int>> adj;

    public:
    Graph(int v){
        V = v;
        adj.resize(v);
    }

    void addEdge(int a, int b) {
        adj[a].push_back(b);
        adj[b].push_back(a);
    }


    
    void serialDFSTask(int node, vector<int> &ans, vector<bool> &visited) {
        ans.push_back(node);

        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                serialDFSTask(neighbor, ans, visited);
            }
        }

    }


    vector<int> serialDFS(int start) {
        vector<int> ans;
        vector<bool> visited(V, false);
        visited[start] = true;

        serialDFSTask(start, ans, visited);
        return ans;
    }


    void parallelDFSTask(int node, vector<int> &visited, vector<int> &ans) {
        #pragma omp critical
        {
            ans.push_back(node);
        }

        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                bool run_flag = false;

                #pragma omp critical
                {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        run_flag = true;
                    }
                }

                if (run_flag) {
                    #pragma omp task shared(visited, ans) firstprivate(neighbor)
                    {
                        parallelDFSTask(neighbor, visited, ans);
                    }
                }

            }

        }

    }


    vector<int> parallelDFS(int start) {
        vector<int> ans;
        vector<int> visited(V, false);
        visited[start] = true;

        #pragma omp parallel
        {
            #pragma omp single
            {
                parallelDFSTask(start, visited, ans);
            }
            
        }
        
        return ans;
    }


    // ------------------------------------------------------------------------------------------------------
    // Second approach to parallel DFS using stack and critical sections (might not be theoretically correct)
    // Note: This method is not recommended for large graphs.
    // ------------------------------------------------------------------------------------------------------
    void parallelDFS1(int start, vector<int>& ans)
    {
        vector<bool> visited(V, false);
        stack<int> st;

        visited[start] = true;
        st.push(start);

        ans.push_back(start);

        while (!st.empty())
        {
            int node = st.top();
            st.pop();
            ans.push_back(node);

#pragma omp parallel for
            for (int i = 0; i < adj[node].size(); i++)
            {
                int neighbour = adj[node][i];

                if (!visited[neighbour])
                {
#pragma omp critical
                    if (!visited[neighbour])
                    {
                        visited[neighbour] = true;
                        st.push(neighbour);
                    }
                }
            }
        }
    }


    vector<int> serialBFS(int start) {
        vector<int> visited(V, false);
        queue<int> q;
        vector<int> ans;
        q.push(start);

        while (!q.empty()) {
            int node = q.front();
            visited[node] = true;
            ans.push_back(node);
            q.pop();

            for (int i : adj[node]) {
                if (!visited[i]) {
                    q.push(i);
                    visited[i] = true;
                }
            }
        }

        return ans;
    }


    vector<int> parallelBFS(int start) {
        vector<int> visited(V, false);
        vector<int> current_level;
        vector<int> next_level;
        vector<int> ans;
        current_level.push_back(start);

        while (!current_level.empty()) {
            ans.insert(ans.end(), current_level.begin(), current_level.end());
            int size = current_level.size();
            
            #pragma omp parallel for
            for (int n=0 ; n<size ; n++) {
                int node = current_level[n];
                visited[node] = true;
                
                for (int neighbor : adj[node]) {
                    if (!visited[neighbor]) {
                    #pragma omp critical 
                        {
                            next_level.push_back(neighbor);
                            visited[neighbor] = true;
                        }
                    }
                }
            }
            current_level = next_level;
            next_level.clear();
        }
        return ans; 
    }

};

void printAns(vector<int> &ans) {
    for (int i : ans) {
        cout << " -> " << i;
    }
    cout << endl;
}

int main() {
    // Sample Large graph
    int num_nodes = 200000;
    Graph g(num_nodes);

    cout << "Building a massive Binary Tree with " << num_nodes << " nodes..." << endl;
    // Build a binary tree (Each node has up to 2 children)
    for (int i = 0; i < num_nodes / 2; i++) {
        if (2 * i + 1 < num_nodes) g.addEdge(i, 2 * i + 1);
        if (2 * i + 2 < num_nodes) g.addEdge(i, 2 * i + 2);
    }
    cout << "Graph built successfully!\n" << endl;

    // Graph g(6);

    // g.addEdge(0, 1);
    // g.addEdge(0, 2);
    // g.addEdge(1, 3);
    // g.addEdge(2, 3);
    // g.addEdge(2, 4);
    // g.addEdge(3, 5);
    // g.addEdge(4, 5);

    // Graph
    /*
       0
      / \
     1   2
      \ / \
       3   4
        \ /
         5`
    */

    // Considering start = 0...
    // Expected DFS: 0 -> 1 -> 3 -> 2 -> 4 -> 5
    // Expected BFS: 0 -> 1 -> 2 -> 3 -> 4 -> 5
    // Note, parallel versions might not give the exact same order due to the nature of parallelism, but they should contain the same nodes.

    double start_time, end_time;
    vector<int> ans;

    start_time = omp_get_wtime();
    ans = g.serialDFS(0);
    end_time = omp_get_wtime();
    cout<<"Serial DFS:";
    // printAns(ans);
    cout << "Time taken for Serial DFS: " << (end_time - start_time) << " seconds" << endl;
    ans.clear();
    cout<<endl;

    start_time = omp_get_wtime();
    ans = g.parallelDFS(0);
    end_time = omp_get_wtime();
    cout<<"Parallel DFS:";
    // printAns(ans);
    cout << "Time taken for Parallel DFS: " << (end_time - start_time) << " seconds" << endl;
    ans.clear();
    cout<<endl;

    // EXTRA
    // start_time = omp_get_wtime();
    // g.parallelDFS1(0, ans);
    // end_time = omp_get_wtime();
    // cout<<"Parallel DFS (Method 1):";
    // // printAns(ans);
    // cout << "Time taken for Parallel DFS (Method 1): " << (end_time - start_time) << " seconds" << endl;
    // ans.clear();
    // cout<<endl;

    start_time = omp_get_wtime();
    ans = g.serialBFS(0);
    end_time = omp_get_wtime();
    cout<<"Serial BFS:";
    // printAns(ans);
    cout << "Time taken for Serial BFS: " << (end_time - start_time) << " seconds" << endl;
    ans.clear();
    cout<<endl;

    start_time = omp_get_wtime();
    ans = g.parallelBFS(0);
    end_time = omp_get_wtime();
    cout<<"Parallel BFS:";
    // printAns(ans);
    cout << "Time taken for Parallel BFS: " << (end_time - start_time) << " seconds" << endl;
    ans.clear();
    cout<<endl;


    // --- PARALLEL BFS COMPLEXITY ---
    // Theoretical Span: O(D), where D is the depth of the graph (Diameter).
    // Practically, following the Work-Span model (Brent's Theorem), 
    // the complexity is O( (V + E) / P + D ).
    // Note: By Amdahl's Law, D is the sequential bottleneck that limits maximum speedup.
    // (P = processors, V = vertices, E = edges)


    // --- PARALLEL DFS COMPLEXITY ---
    // Theoretical Span: O(L), where L is the length of the critical path (longest branch).
    // Practically, following the Work-Span model, the complexity is O( (V + E) / P + L ).
    // Note: By Amdahl's Law, L is the sequential bottleneck. Dense graphs with 
    // long, winding paths will severely limit parallel speedup due to task dependencies.
    // (P = processors, V = vertices, E = edges)


    // Why we used vector<int> for visited instead of vector<bool> in parallel versions?
    // In C++, vector<bool> is a specialized template that packs bits together for memory efficiency.
    // This means that multiple threads trying to modify different bits in the same byte can cause race conditions
    // and undefined behavior. Using vector<int> (or vector<char>) ensures that each thread modifies a separate memory location,
    // making it thread-safe without needing additional synchronization for the visited array itself.

    return 0;
}