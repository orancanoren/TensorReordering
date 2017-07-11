#include "ordering.hpp"
#include <iostream>
#include <string>

using namespace std;

int main() {
	Ordering graph(6);
	graph.insertEdge(2, 5);
	graph.insertEdge(1, 2);
	graph.insertEdge(2, 4);
	graph.insertEdge(4, 5);

	graph.rabbitOrder();

	return 0;
}