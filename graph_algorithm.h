//
//  graph_algorithm.h
//  AnalysisGraph
//
//  Created by Tarek Auel on 12.02.15.
//
//

#ifndef __AnalysisGraph__graph_algorithm__
#define __AnalysisGraph__graph_algorithm__

#include <google/dense_hash_map>

#include <string>
#include <vector>

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

    class graph_algorithm {
    public:
        std::vector<std::pair<vertex*,double>> spreading_activation(oc::graph&,const std::string&,int);
        void worker(std::vector<Impuls*>*, std::vector<std::pair<long unsigned int,double>>*,int,int);
        void spreading_activation_step(Impuls*,std::vector<std::pair<long unsigned int,double>>&, std::vector<Impuls*>&,int);
        bool check_history(const Impuls*, const vertex*);
    };


}

#endif /* defined(__AnalysisGraph__graph_algorithm__) */
