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
#include <thread>

namespace oc {
    
    typedef std::pair<vertex*,double> CPair;

    std::mutex m_task_list;
    std::mutex m_wait_init_work;
    std::condition_variable cond_wait_init_work;

    const int limit = 10;

    void graph_algorithm::worker(std::vector<Impuls*>* task_list, std::vector<std::pair<long unsigned int,double>>* aggregates,int max_hops, int thread) {
        std::vector<Impuls*> custom_task_list;
        std::vector<std::pair<long unsigned int,double>> custom_aggregates;

        std::unique_lock<std::mutex> lock {m_task_list};
        std::unique_lock<std::mutex> lock_init{m_wait_init_work};
        if (task_list->size() == 0) {
            std::cout << "Start waiting " << thread << std::endl;
            lock.unlock();
            cond_wait_init_work.wait(lock_init);
            lock.lock();
            std::cout << "Start now " << thread << std::endl;
        } else {
            std::cout << "Could start immediately " << thread << std::endl;
        }


        while (task_list->size() != 0) {
            if (task_list->size() > limit) {
                std::move(task_list->begin(), task_list->begin() + limit, std::back_inserter(custom_task_list));
                task_list->erase(task_list->begin(),task_list->begin() + limit);
            } else {
                std::move(task_list->begin(), task_list->end(), std::back_inserter(custom_task_list));
                task_list->erase(task_list->begin(),task_list->end());
            }
            lock.unlock();

            for (int j = 0; j != custom_task_list.size(); ++j) {
                spreading_activation_step(custom_task_list[j], custom_aggregates, custom_task_list, max_hops);
                if (custom_task_list.size() > limit) {
                    lock.lock();
                    std::move(custom_task_list.begin(),custom_task_list.begin() + limit,std::back_inserter(*task_list));
                    custom_task_list.erase(custom_task_list.begin(),custom_task_list.begin() + limit);
                    lock.unlock();
                    cond_wait_init_work.notify_one();
                }
            }

            lock.lock();
            std::move(custom_aggregates.begin(), custom_aggregates.end(), std::back_inserter(*aggregates));
        }
        std::cout << "Finished " << thread << std::endl;
        lock.unlock();
        cond_wait_init_work.notify_all();
    }

    std::vector<std::pair<vertex*,double>> graph_algorithm::spreading_activation(oc::graph& g,const std::string& start_id, int max_hops) {

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        std::vector<std::pair<long unsigned int,double>> aggregates;
        
        vertex* start_node = g.get_vertex(start_id);
        aggregates.push_back(std::pair<long unsigned int,double>{start_node->get_id(),1});
        
        Impuls* i = new Impuls(start_node,1);
        
        std::vector<Impuls*> task_list;
        task_list.push_back(i);

        std::thread w1 (&graph_algorithm::worker,*this,&task_list,&aggregates,max_hops,1);
        std::thread w2 (&graph_algorithm::worker,*this,&task_list,&aggregates,max_hops,2);

        w1.join();
        w2.join();
        
        tbb::parallel_sort( aggregates.begin(), aggregates.end(),[](std::pair<long unsigned int,double>& a, std::pair<long unsigned int,double>& b){return a.first > b.first;});
        
        std::vector<std::pair<vertex*,double>> result;
        
        long unsigned int last_id = 0;
        bool first = true;
        double sum = 0;
        for (auto p : aggregates) {
            if (first || last_id != p.first) {
                if (!first) {
                    result.push_back(std::pair<vertex*,double>{g[last_id],sum});
                }
                sum = 0;
                last_id = p.first;
                first = false;
            }
            sum += p.second;
        }
        
        result.push_back(std::pair<vertex*,double>{g[last_id],sum});
        
        
        for (auto p : task_list) {
            delete p;
        }
        
        std::sort(result.begin(),result.end(),[](CPair& a, CPair& b){return a.second > b.second;});

        end = std::chrono::system_clock::now();
        
        std::cout << "Runtime for " << max_hops << " hops: " <<
                    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() <<
                    " ms" << std::endl;
        
        return result;
    };
    
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