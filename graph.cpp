#include <iostream>
#include <fstream>
#include <string>
#include <boost/regex.hpp>

#include "graph.h"
#include "vertex.h"
#include "relationship.h"


namespace oc {
    
    void graph::build_graph(std::ifstream& in) {
        clock_t t_start = clock();
        if (!in.is_open()) {
            std::cerr << "File could not be opended" << std::endl;
            return;
        }
        std::string s;
        bool first = true;
        boost::regex pattern("\"(.*)\",\"(.*)\",(\"(.*)\"|$)");
        boost::smatch matches;
        while (std::getline(in,s)) {
            if (first) first = false;
            else {
                if (boost::regex_search(s,matches,pattern)) {
                    if (matches.size() == 5) {
                        get_vertex(matches[1])->add_out(get_vertex(matches[2]),matches[3]);
                    } else {
                        get_vertex(matches[1])->add_out(get_vertex(matches[2]));
                    }
                } else {
                    std::cerr << "Could not parse line: " << s << std::endl;
                }
            }
        }
        clock_t t = clock() - t_start;
        std::cout << "Parsed file in " << t << " clocks (" << t*1000/CLOCKS_PER_SEC << "ms)" << std::endl;
    }

    graph::~graph() {
        for (auto p : vertex_list) {
            delete p;
        }
    }
    
    vertex* graph::operator[](std::string identifier) {
        std::unordered_map<std::string,vertex*>::iterator it = vertex_map.find(identifier);
        if (it != vertex_map.end()) {
            return (*it).second;
        } else {
            throw std::out_of_range(identifier + " not found in graph");
        }
    }

    vertex* graph::get_vertex(std::string identifier) {
        std::unordered_map<std::string,vertex*>::iterator it = vertex_map.find(identifier);
        if (it == vertex_map.end()) {
            auto *v = new oc::vertex(this,vertex_list.size(), identifier);
            vertex_list.push_back(v);
            vertex_map[identifier] = v;
            return v;
        } else {
            return (*it).second;
        }
    }
    
    unsigned long graph::get_num_edges() const {
        long count{0};
        for (auto v : vertex_list) {
            count += v->num_out_edges();
        }
        return count;
    }
}