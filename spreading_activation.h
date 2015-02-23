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
    template<typename T>
    class Impuls;

    class spreading_activation {
    private:
        const int limit = 1000;

        int started_threads;
        int finished_threads;

        std::mutex* shared_list;
    public:
        std::vector<std::tuple<vertex*,double,std::vector<Impuls<double>*>>> algorithm(oc::graph&,const std::string&,int,int,double,const std::string&, std::string&);
        void worker(std::vector<std::vector<Impuls<double>*>*>*, std::vector<std::pair<long unsigned int,Impuls<double>*>>*,int,double,int,std::vector<std::timed_mutex*>*,std::vector<int*>*,std::vector<Impuls<double>*>*);
        void spreading_activation_step(Impuls<double>*,std::vector<std::pair<long unsigned int,Impuls<double>*>>&, std::vector<Impuls<double>*>&,int,double,std::vector<Impuls<double>*>*);
    };


}

#endif /* defined(__AnalysisGraph__graph_algorithm__) */
