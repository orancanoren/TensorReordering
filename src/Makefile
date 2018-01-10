PURE:
	g++ -std=c++11 -c -O3 ./RCM/rcm.hpp ./RCM/rcm.cpp
	g++ -std=c++11 -c -O3 ./RabbitOrder/dendrogram.hpp ./RabbitOrder/dendrogram.cpp ./RabbitOrder/ordering.hpp ./RabbitOrder/ordering.cpp
	g++ -std=c++11 -c -O3 ./RelabelTensor/relabel.hpp ./RelabelTensor/relabel.cpp
	g++ -std=c++11 -c -O3 ./TensorToGraph/convert.hpp ./TensorToGraph/convert.cpp
	g++ -std=c++11 -O3 main.cpp ordering.o relabel.o convert.o rcm.o dendrogram.o -o PURE
clean:
	rm PURE ordering.o relabel.o convert.o rcm.o dendrogram.o
