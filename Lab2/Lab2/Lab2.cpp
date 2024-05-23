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

// Reading graph from file
// Sample input: 0,2,5,1,, 2,0,3,2,,;5,3,0,3,1,5;1,2,3,0,1, ,,1,1,0,2;,,5,,2,0;
// cells are separated by a comma �, � and rows are separated by a semicolon ';' or a white space ' '
vector<vector<int>> inputGraph(const string& filename) {
	vector<vector<int>> graph;
	ifstream file(filename);
	string line;
	if (file.is_open()) {
		while (getline(file, line)) {
			vector<int> row;
			stringstream ss(line);
			string cell;

			while (getline(ss, cell, ',')) {
				if (cell == ";" || cell == " ") {	// rows are separated by semicolon or white space
					if (!row.empty()) {
						graph.push_back(row);
						row.clear();
					}
				}
				else if (cell.empty()) {	// if cell is empty, set it to INT_MAX
					row.push_back(INT_MAX);
				}
				else {
					row.push_back(stoi(cell));
				}
			}
			// add the last row
			if (!row.empty()) {
				graph.push_back(row);
			}
		}
	}
	file.close();
	return graph;
}

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

int main() {
	string filename = "graph.csv";
	vector<vector<int>> graph = inputGraph(filename);
	printTable(graph);
}