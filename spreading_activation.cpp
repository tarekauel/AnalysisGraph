//
//  spreading_activation.cpp
//  AnalysisGraph
//
//  Created by Tarek Auel on 12.02.15.
//
//

#include "spreading_activation.h"
#include "vertex.h"
#include "graph.h"
#include "relationship.h"
#include "impuls.h"

#include <tbb/parallel_sort.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_for_each.h>

#include <boost/algorithm/string/replace.hpp>

#include <iostream>
#include <thread>
#include <fstream>

#include <tuple>
#include <Python/Python.h>

namespace oc {
    
    typedef std::tuple<vertex*,double,std::vector<Impuls<double>*>> CPair;

    std::string escape(std::string s) {
        boost::replace_all(s, "\"", "\\\"");
        return s;
    }

    void spreading_activation::worker(std::vector<std::vector<Impuls<double>*>*>* task_list, std::vector<std::pair<long unsigned int,Impuls<double>*>>* aggregates,int max_hops,double threshold, int thread, std::vector<std::timed_mutex*>* locks, std::vector<int*>* counters, std::vector<Impuls<double>*>* created_impuls) {
        std::vector<std::pair<long unsigned int,Impuls<double>*>> custom_aggregates;
        std::vector<Impuls<double>*> custom_created_impuls;

        std::unique_lock<std::mutex> lck_shared {*shared_list, std::defer_lock};
        std::unique_lock<std::timed_mutex> lck_own {*(*locks)[thread], std::defer_lock};

        std::vector<Impuls<double>*>* custom_task_list = task_list->at(thread);
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
                            int* temp_counter = counters->at(i);
                            std::vector<Impuls<double>*> *temp_task_list = task_list->at(i);
                            int diff = temp_task_list->size() - (*temp_counter);
                            if (diff > limit) {
                                if (lck_own.try_lock_for(std::chrono::milliseconds(10))) {
                                    std::move(temp_task_list->end() - (diff / 2), temp_task_list->end(), std::back_inserter(*custom_task_list));
                                    temp_task_list->erase(temp_task_list->end() - (diff / 2), temp_task_list->end());
                                    grabbed = true;
                                    std::cout << "Grabbed " << (diff / 2) << " items" << std::endl;
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
                spreading_activation_step(custom_task_list->at(*counter), custom_aggregates, *custom_task_list, max_hops,threshold,&custom_created_impuls);
                lck_own.unlock();
                lck_own.lock();
            }

            custom_task_list->resize(0);
            lck_own.unlock();

            lck_shared.lock();
            std::move(custom_aggregates.begin(), custom_aggregates.end(), std::back_inserter(*aggregates));
            custom_aggregates.resize(0);
            std::move(custom_created_impuls.begin(), custom_created_impuls.end(), std::back_inserter(*created_impuls));
            custom_created_impuls.resize(0);

            finished_threads++;
            lck_shared.unlock();
        } while (started_threads != finished_threads && could_not_grab != 2);
    }

    std::vector<std::tuple<vertex*,double,std::vector<Impuls<double>*>>> spreading_activation::algorithm(oc::graph& g,const std::string& start_id, int max_hops,int num_threads, double threshold, const std::string& output_filename, std::string& response) {
        shared_list = new std::mutex();
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        std::vector<std::pair<long unsigned int,Impuls<double>*>> aggregates;
        std::vector<Impuls<double>*>* created_impuls = new std::vector<Impuls<double>*>();

        vertex* start_node = g.get_vertex(start_id);
        Impuls<double>* start_impuls = new Impuls<double>(start_node,1);
        created_impuls->push_back(start_impuls);

        std::vector<std::thread> threads;
        std::vector<std::timed_mutex*> mutex_list;
        std::vector<std::vector<Impuls<double>*>*> task_lists;
        std::vector<int*> counters;

        started_threads = num_threads;

        for (int i=0; i != num_threads;++i) {
            std::timed_mutex* mutex = new std::timed_mutex();
            mutex_list.push_back(mutex);
            int* j = new int;
            counters.push_back(j);
            task_lists.push_back(new std::vector<Impuls<double>*>());
            if (i==0) {
                task_lists.at(0)->push_back(start_impuls);
            }
        }

        for (int i=0; i != num_threads;++i) {
            threads.push_back(std::thread(&spreading_activation::worker,*this,&task_lists,&aggregates,max_hops,threshold,i,&mutex_list,&counters,created_impuls));
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

        tbb::parallel_sort( aggregates.begin(), aggregates.end(),[](std::pair<long unsigned int,Impuls<double>*>& a, std::pair<long unsigned int,Impuls<double>*>& b){return a.first > b.first;});

        std::vector<std::tuple<vertex*,double,std::vector<Impuls<double>*>>> result;

        long unsigned int last_id = 0;
        bool first = true;
        double sum = 0;
        std::vector<Impuls<double>*> tracking;
        for (auto p : aggregates) {
            if (first || last_id != p.first) {
                if (!first) {
                    result.push_back(std::make_tuple(g[last_id],sum,tracking));
                }
                tracking = std::vector<Impuls<double>*>();
                sum = 0;
                last_id = p.first;
                first = false;
            }
            sum += p.second->power;
            tracking.push_back(p.second);
        }

        result.push_back(std::make_tuple(g[last_id],sum,tracking));
        result.push_back(std::make_tuple(start_impuls->node,1,std::vector<Impuls<double>*>()));
        
        tbb::parallel_sort(result.begin(),result.end(),[](CPair& a, CPair& b){return std::get<1>(a) > std::get<1>(b);});

        end = std::chrono::system_clock::now();
        
        std::cout << "Runtime for " << max_hops << " hops: " <<
                    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() <<
                    " ms" << std::endl;

        /*if (output_filename != "") {
            std::ofstream out {output_filename};
            out << "[" << std::endl;
            first = true;
            int i = 0;
            for (auto r = result.begin(); r != result.end() && i != 100; ++i, ++r) {
                vertex *v = std::get<0>(*r);
                if (first) first = false;
                else out << "," << std::endl;
                out << "\t{\"id\":\"" << v->get_identifier() << "\",\"value\":" << std::get<1>(*r) <<
                        ",\"type\":\"" << v->get_property("type") << "\"" <<
                        ",\"name\":\"" << v->get_alias() << "\",\"tracking\":[";
                bool inner_first = true;
                for (auto t : (std::get<2>(*r))) {
                    if (t != nullptr) {
                        std::vector<vertex*> history;
                        Impuls<double>::resolve_history(t, history);
                        if (inner_first) inner_first = false;
                        else out << ",";
                        out << "[";
                        bool inner_inner_first = true;
                        for (auto v : history) {
                            if (inner_inner_first) inner_inner_first = false;
                            else out << ",";
                            out << "{\"node\":\"" << v->get_alias_with_type() << "\"}";
                        }
                        out << "]";
                    }
                }
                out << "]}";
            }
            out << std::endl << "]";
        }*/

        response += "[\r\n";
        first = true;
        int i = 0;
        for (auto r = result.begin(); r != result.end() && i != 100; ++i, ++r) {
            vertex *v = std::get<0>(*r);
            if (first) first = false;
            else response += ",\r\n";
            response +=  "\t{\"id\":\"" + escape(v->get_identifier()) + "\",\"value\":" + std::to_string(std::get<1>(*r)) +
                         ",\"type\":\"" + escape(v->get_property("type")) + "\"" +
                         ",\"name\":\"" + escape(v->get_alias()) + "\",\"tracking\":[";
            bool inner_first = true;
            auto result_sort = (std::get<2>(*r));
            std::sort(result_sort.begin(),result_sort.end(),[](Impuls<double>* a,Impuls<double>* b){return a->power > b->power;});
            for (auto t : result_sort) {
                if (t != nullptr) {
                    std::vector<vertex*> history;
                    Impuls<double>::resolve_history(t, history);
                    if (inner_first) inner_first = false;
                    else response += ",";
                    response += "";
                    response += "{\"power\":" + std::to_string(t->power) + ", \"path\":[";
                    bool inner_inner_first = true;
                    for (auto v : history) {
                        if (inner_inner_first) inner_inner_first = false;
                        else response += ",";
                        response += "{\"node\":\"" + escape(v->get_alias_with_type()) + "\"}";
                    }
                    response += "]}";
                }
            }
            response += "]}";
        }
        response += "\r\n]";

        Impuls<double>::clean_list(created_impuls);

        delete shared_list;

        return result;
    };
    
    void spreading_activation::spreading_activation_step(Impuls<double>* i,std::vector<std::pair<long unsigned int,Impuls<double>*>>& aggregates, std::vector<Impuls<double>*>& task_list, int max_hops, double threshold, std::vector<Impuls<double>*>* created_impuls) {
        std::vector<vertex*> neighbors = i->node->get_neighbors();
        if (i->hops != 0) {
            i->power /= neighbors.size() - 1;
        } else {
            i->power /= neighbors.size();
        }

        for (auto n : neighbors) {
            if (!Impuls<double>::check_history(i, n)) {
                aggregates.push_back(std::pair<long unsigned int,Impuls<double>*>{(n)->get_id(),i});
                if (i->hops + 1 < max_hops && ( i->power > threshold || threshold == 0)) {
                    Impuls<double>* new_impuls = new Impuls<double>();
                    new_impuls->prev_impuls = i;
                    new_impuls->hops = i->hops + 1;
                    new_impuls->power = i->power;
                    new_impuls->node = n;
                    task_list.push_back(new_impuls);
                    created_impuls->push_back(new_impuls);
                }
            }
        }
    }
}