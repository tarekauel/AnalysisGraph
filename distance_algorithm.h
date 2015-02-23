#ifndef __AnalysisGraph__distance_algorithm__
#define __AnalysisGraph__distance_algorithm__

#include <string>

namespace oc {
    class graph;

    class distance_algorithm {
    public:
        double algorithm(graph& g, const std::string&,const std::string&,int,std::string&);
    };
}

#endif