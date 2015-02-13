#include <iostream>
#include <fstream>
#include "vertex.h"
#include "relationship.h"
#include "graph.h"
#include "graph_algorithm.h"

int main() {

    std::ifstream in("/Users/tarek/xcode/AnalysisGraph/edges_dump.csv", std::ifstream::in);
    //std::ifstream in("/Users/tarek/xcode/AnalysisGraph/sa_demo.csv", std::ifstream::in);
    oc::graph g{};
    
    g.build_graph(in);
    
    std::cout << std::endl << g << std::endl;
    

    std::string id = "organization/big-data-elephants";
    //std::string id = "A";
    
    oc::graph_algorithm alg;
    
    std::vector<std::pair<oc::vertex*,double>> result = alg.spreading_activation(g,id,2);

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