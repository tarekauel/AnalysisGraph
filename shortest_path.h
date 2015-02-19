#ifndef __AnalysisGraph__shortest_path__
#define __AnalysisGraph__shortest_path__

#include <string>

namespace oc {

    class graph;

    class shortest_path {
    public:
        int algorithm(graph&,const std::string,const std::string,const std::vector<unsigned long>&);
    };
}

#endif