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

    const int limit = 1000;

    int started_threads;
    int finished_threads;

    std::mutex shared_list;

    void graph_algorithm::worker(std::vector<std::vector<std::shared_ptr<Impuls>>*>* task_list, std::vector<std::pair<long unsigned int,std::shared_ptr<Impuls>>>* aggregates,int max_hops,double threshold, int thread, std::vector<std::timed_mutex*>* locks, std::vector<int*>* counters) {
        std::vector<std::pair<long unsigned int,std::shared_ptr<Impuls>>> custom_aggregates;
        
        std::unique_lock<std::mutex> lck_shared {shared_list, std::defer_lock};
        std::unique_lock<std::timed_mutex> lck_own {*(*locks)[thread], std::defer_lock};

        std::vector<std::shared_ptr<Impuls>>* custom_task_list = task_list->at(thread);
        int* counter = counters->at(thread);

        bool first = true;
        int could_not_grab = 0;
        do {
            if (first) first = false;
            else {
                lck_shared.lock();
                --finished_threads;
                bool grabbed = false;
                int random = std::rand() % started_threads;
                for (int j=0; j != locks->size(); ++j) {
                    int i = (j + random) % started_threads;
                    if (i!=thread) {
                        std::unique_lock<std::timed_mutex> temp_lock {*(locks->at(i)),std::defer_lock};
                        if (temp_lock.try_lock_for(std::chrono::milliseconds(10))) {
                            int *temp_counter = counters->at(i);
                            std::vector<std::shared_ptr<Impuls>> *temp_task_list = task_list->at(i);
                            int diff = temp_task_list->size() - *temp_counter;
                            if (diff > limit) {
                                if (lck_own.try_lock_for(std::chrono::milliseconds(10))) {
                                    std::move(temp_task_list->end() - diff / 2, temp_task_list->end(), std::back_inserter(*custom_task_list));
                                    temp_task_list->erase(temp_task_list->end() - diff / 2, temp_task_list->end());
                                    grabbed = true;
                                    std::cout << "Grabbed " << diff / 2 << " items" << std::endl;
                                    lck_own.unlock();
                                }
                            }
                        }
                    }
                }
                if (grabbed) could_not_grab = 0;
                else ++could_not_grab;
                lck_shared.unlock();
            }

            lck_own.lock();
            for (*counter = 0; *counter != custom_task_list->size(); ++(*counter)) {
                spreading_activation_step(custom_task_list->at(*counter), custom_aggregates, *custom_task_list, max_hops,threshold);
                lck_own.unlock();
                lck_own.lock();
            }

            custom_task_list->resize(0);
            lck_own.unlock();

            lck_shared.lock();
            std::move(custom_aggregates.begin(), custom_aggregates.end(), std::back_inserter(*aggregates));
            custom_aggregates.resize(0);

            finished_threads++;
            lck_shared.unlock();
        } while (started_threads != finished_threads && could_not_grab != 2);
    }

    std::vector<std::pair<vertex*,double>> graph_algorithm::spreading_activation(oc::graph& g,const std::string& start_id, int max_hops,int num_threads, double threshold) {

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        std::vector<std::pair<long unsigned int,std::shared_ptr<Impuls>>> aggregates;

        vertex* start_node = g.get_vertex(start_id);
        std::shared_ptr<Impuls> start_impuls(new Impuls(start_node,1));

        std::vector<std::thread> threads;
        std::vector<std::timed_mutex*> mutex_list;
        std::vector<std::vector<std::shared_ptr<Impuls>>*> task_lists;
        std::vector<int*> counters;

        started_threads = num_threads;

        for (int i=0; i != num_threads;++i) {
            std::timed_mutex* mutex = new std::timed_mutex();
            mutex_list.push_back(mutex);
            int* j = new int;
            counters.push_back(j);
            task_lists.push_back(new std::vector<std::shared_ptr<Impuls>>());
            if (i==0) {
                task_lists.at(0)->push_back(start_impuls);
            }
        }

        for (int i=0; i != num_threads;++i) {
            threads.push_back(std::thread(&graph_algorithm::worker,*this,&task_lists,&aggregates,max_hops,threshold,i,&mutex_list,&counters));
        }
        
        for (auto p = threads.begin(); p != threads.end(); ++p) {
            p->join();
        }

        for (int i=0; i != num_threads;++i) {
            delete mutex_list.at(i);
            delete counters.at(i);
            delete task_lists.at(i);
        }

        std::cout << "Aggregates: " << aggregates.size() << std::endl;

        tbb::parallel_sort( aggregates.begin(), aggregates.end(),[](std::pair<long unsigned int,std::shared_ptr<Impuls>>& a, std::pair<long unsigned int,std::shared_ptr<Impuls>>& b){return a.first > b.first;});
        
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
            sum += p.second->power;
        }
        
        result.push_back(std::pair<vertex*,double>{g[last_id],sum});

        result.push_back(std::pair<vertex*,double>{start_impuls->node,1});
        
        std::sort(result.begin(),result.end(),[](CPair& a, CPair& b){return a.second > b.second;});

        end = std::chrono::system_clock::now();
        
        std::cout << "Runtime for " << max_hops << " hops: " <<
                    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() <<
                    " ms" << std::endl;
        
        return result;
    };
    
    void graph_algorithm::spreading_activation_step(std::shared_ptr<Impuls> i,std::vector<std::pair<long unsigned int,std::shared_ptr<Impuls>>>& aggregates, std::vector<std::shared_ptr<Impuls>>& task_list, int max_hops, double threshold) {
        std::vector<vertex*> neighbors = i->node->get_neighbors();
        if (i->hops != 0) {
            i->power /= neighbors.size() - 1;
        } else {
            i->power /= neighbors.size();
        }
        //todo ein impuls für jeden koten

        for (auto n : neighbors) {
            if (!check_history(i, n)) {
                aggregates.push_back(std::pair<long unsigned int,std::shared_ptr<Impuls>>{(n)->get_id(),i});
                if (i->hops + 1 < max_hops && ( i->power > threshold || threshold == 0)) {
                    std::shared_ptr<Impuls> new_impuls(new Impuls());
                    new_impuls->prev_impuls = i;
                    new_impuls->hops = i->hops + 1;
                    new_impuls->power = i->power;
                    new_impuls->node = n;
                    task_list.push_back(new_impuls);
                }
            }
        }
    }
    
    bool graph_algorithm::check_history(std::shared_ptr<Impuls> i, const oc::vertex* v) {
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