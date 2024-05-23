// Hsin Li
// 05.23.2024
// Lab2: Dijkstra's Algorithm

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <climits>
#include <algorithm>
#include <iomanip>

using namespace std;

// first digit of student ID
const int ID_FRISTDIGIT = 1;

// Reading graph from csv file
// Sample input: 0,2,5,1,, 2,0,3,2,,;5,3,0,3,1,5;1,2,3,0,1, ,,1,1,0,2;,,5,,2,0;
// cells are separated by a comma ','
// rows are separated by a semicolon ';' or a white space ' '
vector<vector<int>> inputGraph(const string& filename) {
	vector<vector<int>> graph;
	ifstream file(filename);
	vector<int> row;
	char ch;
	string cell;
	// number of vertices
	size_t V = 0;

	if (file.is_open()) {
		// read the file character by character
		while (file.get(ch)) {
			if (ch == ',') {
				if (!cell.empty()) {
					row.push_back(stoi(cell));
					cell.clear();
				} else {
					row.push_back(INT_MAX);
				}
			} else if (ch == ';' || ch == ' ') {
				if (!cell.empty()) {
					row.push_back(stoi(cell));
					cell.clear();
				}
				V = max(V, row.size());
				graph.push_back(row);
				row.clear();
			} else {
				cell += ch;
			}
		}

		// ensure all rows have the same size
		for (auto& row : graph) {
			while (row.size() < V) {
				// add INT_MAX to the end of the row if the row is not full
				row.push_back(INT_MAX);
			}
		}
	}
	file.close();
	return graph;
}

// Print Initial Distance Table
void printTable(const vector<vector<int>>& graph) {
	cout << "Initial Distance Table:" << endl;
	for (const auto& row : graph) {
		for (const auto& cell : row) {
			if (cell == INT_MAX) {
				// if cell is null, leave 4 spaces
				cout << "    ";
			}
			else {
				cout << setw(4) << cell;
			}
		}
		cout << endl;
	}
}

// Helper function to find the vertex with minimum distance value
int minDistance (const vector<int> &dist, const vector<bool> &visited) {
	int min = INT_MAX, min_index;

	for (size_t v = 0; v < dist.size(); v++) {
		// if vertex v is not visited and the distance is equal to first digit of student ID
		if (!visited[v] && dist[v] == ID_FRISTDIGIT) {
			return v;
		} else if (!visited[v] && dist[v] <= min) {
			min = dist[v];
			min_index = v;
		}
	}
	return min_index;
}

// print out the result of Dijkstra's Algorithm
void printDijkstra (const vector<int> &dist, const vector<int>& prev, int V, int source, const vector<int>& path) {
	cout << "Dijkstra Result:" << endl;
	cout << "Spanning Tree: ";
	// print out spanning tree
	for (int v: path) {
		cout << static_cast<char>('A' + v);
	}
	cout << endl;

	cout << "(Destination, Previous node, Distance)" << endl;
	// print out all edges in shortest path
	for (int i = 1; i < V; i++) {
		if (prev[i] != -1) {
			// 
			cout << "(" << static_cast<char>('A' + i) << ",";
			cout << static_cast<char>('A' + prev[i]) << ",";
			cout << dist[i] << ")" << endl;
		}
	}
}

// Dijkstra's Algorithm
void dijkstra (const vector<vector<int>>& graph, int source) {
	int V = graph.size();
	vector<int> dist(V, INT_MAX);
	vector<int> prev(V, -1);
	vector<bool> visited(V, false);
	vector<int> path;

	// distance from source node to itself is 0
	dist[source] = 0;

	for (int u : dist) {
		// find the vertex with minimum distance value
		u = minDistance(dist, visited);
		// mark the vertex as visited
		visited[u] = true;
		// add the vertex to the path
		path.push_back(u);
		// cout << "curr: " << static_cast<char>('A' + u) << endl;
		// update the distance value of the adjacent vertices of the picked vertex
		for (int v = 0; v < V; v++) {
			// if vertex v is not visited
			// there is an edge from u to v
			// the total weight of path from source to v through u is less than the current distance value of v
			if (!visited[v] && graph[u][v] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
				// Update dist from source to v
				dist[v] = dist[u] + graph[u][v];
				// set previous node of v to u
				prev[v] = u;
			}
		}
	}

	printDijkstra(dist, prev, V, source, path);
}

// Main function
int main() {
	string filename = "graph.csv";
	vector<vector<int>> graph = inputGraph(filename);
	printTable(graph);

	// Dijkstra's Algorithm
	dijkstra(graph, 0);

}