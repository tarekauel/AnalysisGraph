//
//  spreading_activation.h
//  AnalysisGraph
//
//  Created by Tarek Auel on 12.02.15.
//
//

#ifndef __AnalysisGraph__graph_algorithm__
#define __AnalysisGraph__graph_algorithm__

#include <string>
#include <vector>
#include <mutex>

namespace oc {
    
    class graph;
    class vertex;
    
    class Impuls {
    private:
    public:
        Impuls() :node{NULL},prev_impuls{nullptr} {};
        Impuls(vertex* v,double p) :node{v},power{p},prev_impuls{nullptr} {};

        vertex* node;
        double power;
        Impuls* prev_impuls;
        int hops = 0;
    };

    class spreading_activation {
    public:

        std::vector<std::pair<vertex*,double>> spreading_activation(oc::graph&,const std::string&,int,int,double);
        void worker(std::vector<std::vector<Impuls*>*>*, std::vector<std::pair<long unsigned int,Impuls*>>*,int,double,int,std::vector<std::timed_mutex*>*,std::vector<int*>*,std::vector<Impuls*>*);
        void spreading_activation_step(Impuls*,std::vector<std::pair<long unsigned int,Impuls*>>&, std::vector<Impuls*>&,int,double,std::vector<Impuls*>*);
        bool check_history(Impuls*, const vertex*);
    };


}

#endif /* defined(__AnalysisGraph__graph_algorithm__) */
