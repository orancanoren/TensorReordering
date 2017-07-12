#include "ordering.hpp"
#include <iostream>
#include <string>

using namespace std;

int main() {
	Ordering graph(8);
	graph.insertEdge(0, 2);
	graph.insertEdge(0, 4);
	graph.insertEdge(0, 7);
	graph.insertEdge(5, 7);
	graph.insertEdge(4, 6);
	graph.insertEdge(4, 3);
	graph.insertEdge(3, 1);
	graph.insertEdge(6, 1);
	graph.insertEdge(2, 4);
	graph.insertEdge(2, 7);
	graph.insertEdge(4, 7);
	graph.insertEdge(6, 3);

	graph.rabbitOrder();

	return 0;
}