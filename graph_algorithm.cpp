//
//  graph_algorithm.cpp
//  AnalysisGraph
//
//  Created by Tarek Auel on 12.02.15.
//
//

#include "graph_algorithm.h"
#include "vertex.h"
#include "graph.h"
#include "relationship.h"

#include <tbb/parallel_sort.h>
#include <tbb/parallel_reduce.h>

#include <iostream>
#include <map>
#include <stdexcept>

namespace oc {
    
    typedef std::pair<vertex*,double> CPair;
    
    std::vector<std::pair<vertex*,double>> graph_algorithm::spreading_activation(oc::graph& g,const std::string& start_id, int max_hops) {
        
        clock_t start = clock();
        
        //google::dense_hash_map<long unsigned int,double> aggregates;
        std::vector<std::pair<long unsigned int,double>> aggregates;
        //aggregates.set_empty_key(-1);
        
        vertex* start_node = g.get_vertex(start_id);
        //aggregates[start_node->get_id()] = 1;
        aggregates.push_back(std::pair<long unsigned int,double>{start_node->get_id(),1});
        
        Impuls* i = new Impuls(start_node,1);
        
        std::vector<Impuls*> task_list;
        task_list.push_back(i);
        
        for(int j=0; j != task_list.size(); ++j) {
        //for (auto p = task_list.begin(); p != task_list.end(); ++p) {
        //tbb::parallel_reduce(tbb::blocked_range<size_t>(0,1),[&](const tbb::blocked_range<Impuls*> r){
            graph_algorithm::spreading_activation_step(task_list[j],aggregates,task_list,max_hops);
        }
        //);
        
        tbb::parallel_sort( aggregates.begin(), aggregates.end(),[](std::pair<long unsigned int,double>& a, std::pair<long unsigned int,double>& b){return a.first > b.first;});
        
        std::vector<std::pair<vertex*,double>> result;
        
        long unsigned int last_id = -1;
        double sum = 0;
        for (auto p : aggregates) {
            if (last_id == -1 || last_id != p.first) {
                if (last_id != -1) {
                    result.push_back(std::pair<vertex*,double>{g[last_id],sum});
                }
                sum = 0;
                last_id = p.first;
            }
            sum += p.second;
        }
        
        result.push_back(std::pair<vertex*,double>{g[last_id],sum});
        
        
        for (auto p : task_list) {
            delete p;
        }
        
        /*std::vector<std::pair<vertex*,double>> result;
        for (auto a : aggregates) {
            result.push_back(std::pair<vertex*,double> {g[a.first],a.second});
        }*/
        
        std::sort(result.begin(),result.end(),[](CPair& a, CPair& b){return a.second > b.second;});
        
        clock_t duration = clock() - start;
        
        std::cout << "Runtime for " << max_hops << " hops: " << duration << " clocks, " << duration * 1000 / CLOCKS_PER_SEC << " ms" << std::endl;
        
        return result;
    }
    
    void graph_algorithm::spreading_activation_step(Impuls* i,std::vector<std::pair<long unsigned int,double>>& aggregates, std::vector<Impuls*>& task_list, int max_hops) {
        std::vector<vertex*> neighbors = i->node->get_neighbors();
        if (i->hops != 0) {
            i->power /= neighbors.size() - 1;
        } else {
            i->power /= neighbors.size();
        }
        
        for (auto n : neighbors) {
        //tbb::parallel_for(neighbors.begin(),neighbors.end(),[&](vertex* n){
            if (!check_history(i, n)) {
                aggregates.push_back(std::pair<long unsigned int,double>{(n)->get_id(),i->power});
                //aggregates[n->get_id()] += i->power;
                if (i->hops + 1 < max_hops) {
                    Impuls* new_impuls = new Impuls();
                    new_impuls->prev_impuls = i;
                    new_impuls->hops = i->hops + 1;
                    new_impuls->power = i->power;
                    new_impuls->node = n;
                    task_list.push_back(new_impuls);
                }
            }
        }//);
    }
    
    bool graph_algorithm::check_history(const oc::Impuls* i, const oc::vertex* v) {
        if (i->node == v) {
            return true;
        } else {
            if (i->prev_impuls != nullptr) {
                return check_history(i->prev_impuls, v);
            } else {
                return false;
            }
        }
    }
}