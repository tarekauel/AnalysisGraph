#include "spreading_activation.h"
#include "vertex.h"
#include "graph.h"
#include "relationship.h"
#include "impuls.h"
#include "shortest_path.h"

#include <tbb/parallel_for_each.h>

#include <vector>
#include <iostream>
#include <thread>
#include <fstream>

namespace oc {

    Impuls<int>* one_step(std::vector<Impuls<int>*>& task_list,long unsigned target_id,std::map<unsigned long,int>& visited_nodes,std::vector<Impuls<int>*>& created_impuls,const std::vector<unsigned long>& forbidden_ids) {

        std::vector<Impuls<int>*> task_list_next;

        for (auto i : task_list) {
            for (auto n : i->node->get_neighbors()) {
                if (std::find(forbidden_ids.begin(),forbidden_ids.end(),n->get_id()) == forbidden_ids.end()) {
                    if (!Impuls<int>::check_history(i, n)) {
                        if (visited_nodes.find(n->get_id()) == visited_nodes.end()) {
                            Impuls<int> *i2 = new Impuls<int>{n, i->power + 1};
                            created_impuls.push_back(i2);
                            i2->prev_impuls = i;
                            task_list_next.push_back(i2);
                            visited_nodes[n->get_id()] = i2->power;
                            if (target_id == i2->node->get_id()) {
                                return i2;
                            }
                        } else {
                            if (visited_nodes[n->get_id()] > i->power + 1) {
                                Impuls<int> *i2 = new Impuls<int>{n, i->power + 1};
                                created_impuls.push_back(i2);
                                i2->prev_impuls = i;
                                task_list_next.push_back(i2);
                                visited_nodes[n->get_id()] = i2->power;
                                if (target_id == i2->node->get_id()) {
                                    return i2;
                                }
                            }
                        }
                    }
                }
            }
        }

        task_list = std::move(task_list_next);
        task_list_next.resize(0);

        return nullptr;
    }

    int shortest_path::algorithm(graph& g,const std::string start_identifier,const std::string target_identifier,const std::vector<unsigned long>& forbidden_ids) {

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        if (start_identifier == target_identifier) {
            std::cout << std::endl << "Distance is 0" << std::endl;
            return 0;
        }

        vertex* start_node = g[start_identifier];
        vertex* target_node = g[target_identifier];

        std::vector<Impuls<int>*> task_list;
        std::vector<Impuls<int>*>* created_impuls = new std::vector<Impuls<int>*>();

        std::map<unsigned long,int> visited_nodes;

        Impuls<int>* start_impuls = new Impuls<int>{start_node,0};
        task_list.push_back(start_impuls);
        created_impuls->push_back(start_impuls);
        visited_nodes[start_node->get_id()] = 0;


        Impuls<int>* target = nullptr;
        while (target == nullptr && task_list.size() != 0) {
            target = one_step(task_list,target_node->get_id(),visited_nodes,*created_impuls,forbidden_ids);
        }

        int distance =-1;

        if (target != nullptr) {
            std::vector<vertex *> history;
            Impuls<int>::resolve_history(target, history);

            bool first = true;
            for (auto v: history) {
                if (first) first = false;
                else std::cout << "-->";
                std::cout << v->get_identifier();
            }
            std::cout << std::endl;
            distance = history.size() - 1;
        } else {
            std::cout << "No path found!" << std::endl;
        }

        std::thread t {Impuls<int>::clean_list,created_impuls};

        end = std::chrono::system_clock::now();

        std::printf("Runtime for shortest path: %lld µs", std::chrono::duration_cast<std::chrono::microseconds>(end-start).count());

        std::cout << std::endl << "Distance is " << distance << std::endl;

        t.join();

        return distance;
    };
}