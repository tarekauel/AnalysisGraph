#include <iostream>
#include <fstream>
#include <unistd.h>
#include <execinfo.h>
#include "vertex.h"
#include "relationship.h"
#include "graph.h"
#include "graph_algorithm.h"

void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main() {
    signal(SIGSEGV, handler);

    std::ifstream in("/Users/tarek/xcode/AnalysisGraph/edges_dump.csv", std::ifstream::in);
    //std::ifstream in("/Users/tarek/xcode/AnalysisGraph/sa_demo.csv", std::ifstream::in);
    oc::graph g{};
    
    g.build_graph(in);
    
    std::cout << std::endl << g << std::endl;
    

    std::string id = "organization/big-data-elephants";
    //std::string id = "A";

    oc::graph_algorithm alg;
    
    std::vector<std::pair<oc::vertex*,double>> result = alg.spreading_activation(g,id,10,8,0.00001);

    int i=0;
    for (auto p = result.begin(); p != result.end(); ++p) {
        std::cout << p->first->get_identifier() << ":" << p->second << std::endl;
        ++i;
        if (i!=10) {
            continue;
        }
        break;
    }
    return 0;
}