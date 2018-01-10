#include "./RCM/main.cpp"
#include "./RabbitOrder/main.cpp"
#include "./RandomGraph/rand_graph.cpp"
#include "./RandomTensor/rand_tns.cpp"
#include "./TensorToGraph/main.cpp"
#include "./RelabelTensor/main.cpp"
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

void commands() {
  cout << "Available commands" << endl
       << "\trandom_tensor\tcreate a random tensor with specified dimensions and nnz count" << endl
       << "\trandom_graph\tcreate a random graph with specified edge and vertex count" << endl
       << "\tto_graph\tconvert a compatible tensor file into an k-partite graph" << endl
       << "\trelabel\trelabel a tensor file with the provided permutation file" << endl
       << "\trcm\tcompute a RCM permutation of a supplied graph" << endl
       << "\trabbit\tcompute a rabbit ordering permutation of a supplied graph" << endl;
}

void helpGeneral() {
  cout << "Usage: PURE command" << endl
       << "-------------------" << endl;
  commands();
}

void errorMessage() {
  cout << "PURE --help to see help doc\n";
}

int main(int argc, char * argv[]) {
  if (argc < 2) {
    errorMessage();
    exit(0);
  }

  const char * application = argv[1];

  if (strcmp("--help", application) == 0) {
    helpGeneral();
    exit(0);
  }
  else if (strcmp(application,"random_tensor") == 0)
    randomtensor::randTensorMain(argc - 1, &argv[1]);
  else if (strcmp(application,"random_graph") == 0)
    randgraph::randGraphMain(argc - 1, &argv[1]);
  else if (strcmp(application,"to_graph") == 0)
    convert::tensorToGraphMain(argc - 1, &argv[1]);
  else if (strcmp(application, "relabel") == 0)
    relabel::relabelMain(argc - 1, &argv[1]);
  else if (strcmp(application,"rcm") == 0)
    rcm::RCMmain(argc - 1, &argv[1]);
  else if (strcmp(application, "rabbit") == 0)
    rabbit::rabbitMain(argc - 1, &argv[1]);
  else {
    cout << "Unknown command " << application << endl;
    errorMessage();
  }
  
  return 0;
}
