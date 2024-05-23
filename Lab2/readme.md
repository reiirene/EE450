Name: Hsin Li
ID: 1623988288
Email: hsinli@usc.edu

Program Summary:
    A variation on the Dijkstra's algorithm. The algorithm checks if there is a node with the cost
    that equals to the first digit of your student ID. If there is no match, the algorithm follows
    the rule of the original Dijkstra algorithm to pick the node with the minimum cost.

    - Source node is set to A
    - First digit of student ID is set to 1


References:
    lines 23 ~ 65
    I references the reading input from a file character by character from the following site:
    https://www.geeksforgeeks.org/how-to-read-file-character-by-character-in-cpp/

    lines 100 ~ 155
    I referenced the Dijkstra's shortest path algorithm code from the following site:
    https://www.geeksforgeeks.org/c-program-for-dijkstras-shortest-path-algorithm-greedy-algo-7/


Instructions:
command line input
    $ g++ -g -Wall 1623988288lab2.cpp -o 1623988288lab2
    $ ./1623988288lab2
About the code:
    You can change the constant variable ID_FIRSTDIGIT for the first digit of the student ID