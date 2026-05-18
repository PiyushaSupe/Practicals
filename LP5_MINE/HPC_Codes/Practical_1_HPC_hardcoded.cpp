#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <omp.h>
using namespace std;

class Graph {
    int V;
    vector< vector<int> > adj;

public:
    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    void sequentialBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;
        visited[start] = true;
        q.push(start);
        cout << "Sequential BFS: ";
        while (!q.empty()) {
            int node = q.front();
            q.pop();
            cout << node << " ";
            for (int i = 0; i < (int)adj[node].size(); i++) {
                int neighbor = adj[node][i];
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        cout << endl;
    }

   void parallelBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;
        visited[start] = true;
        q.push(start);
        cout << "Parallel   BFS: ";
        while (!q.empty()) {
            int node;
            #pragma omp critical
            {
                node = q.front();
                q.pop();
            }
            cout << node << " ";

            #pragma omp parallel for
            for (int i = 0; i < (int)adj[node].size(); i++) {
                int neighbor = adj[node][i];
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
        cout << endl;
    }

    void sequentialDFS(int start) {
        vector<bool> visited(V, false);
        stack<int> st;
        st.push(start);
        cout << "Sequential DFS: ";
        while (!st.empty()) {
            int node = st.top();
            st.pop();
            if (visited[node]) continue;
            visited[node] = true;
            cout << node << " ";
            for (int i = (int)adj[node].size() - 1; i >= 0; i--) {
                if (!visited[adj[node][i]])
                    st.push(adj[node][i]);
            }
        }
        cout << endl;
    }

   void dfsUtil(int node, vector<bool> &visited) {
        #pragma omp critical
        {
            cout << node << " ";
        }

        #pragma omp parallel for
        for (int i = 0; i < (int)adj[node].size(); i++) {
            int neighbor = adj[node][i];
            bool shouldVisit = false;
            #pragma omp critical
            {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    shouldVisit = true;
                }
            }
            if (shouldVisit) {
                dfsUtil(neighbor, visited);
            }
        }
    }

    void parallelDFS(int start) {
        vector<bool> visited(V, false);
        visited[start] = true;
        cout << "Parallel   DFS: ";
        dfsUtil(start, visited);
        cout << endl;
    }
};

int main() {
    cout << "========================================" << endl;
    cout << "   HPC: Parallel BFS & DFS (OpenMP)    " << endl;
    cout << "========================================" << endl;

   
    //        0
    //       / \
    //      1   2
    //     / \ / \
    //    3  4 5  6
    //
    int vertices = 7;
    Graph g(vertices);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);
    g.addEdge(2, 6);

    cout << "Graph: 7 nodes, binary tree structure"  << endl;
    cout << "Edges: 0-1, 0-2, 1-3, 1-4, 2-5, 2-6"  << endl;
    cout << "Threads available: " << omp_get_max_threads() << endl;
    cout << "----------------------------------------" << endl;

    double t1 = omp_get_wtime();
    g.sequentialBFS(0);
    double seq_bfs = omp_get_wtime() - t1;

    double t2 = omp_get_wtime();
    g.parallelBFS(0);
    double par_bfs = omp_get_wtime() - t2;

    cout << "----------------------------------------" << endl;
    cout << "BFS Sequential time : " << seq_bfs * 1000 << " ms" << endl;
    cout << "BFS Parallel   time : " << par_bfs * 1000 << " ms" << endl;
    if (par_bfs > 0)
        cout << "BFS Speedup        : " << seq_bfs / par_bfs << "x" << endl;

    cout << "----------------------------------------" << endl;

    double t3 = omp_get_wtime();
    g.sequentialDFS(0);
    double seq_dfs = omp_get_wtime() - t3;

    double t4 = omp_get_wtime();
    g.parallelDFS(0);
    double par_dfs = omp_get_wtime() - t4;

    cout << "----------------------------------------" << endl;
    cout << "DFS Sequential time : " << seq_dfs * 1000 << " ms" << endl;
    cout << "DFS Parallel   time : " << par_dfs * 1000 << " ms" << endl;
    if (par_dfs > 0)
        cout << "DFS Speedup        : " << seq_dfs / par_dfs << "x" << endl;

    cout << "========================================" << endl;
    return 0;
}
