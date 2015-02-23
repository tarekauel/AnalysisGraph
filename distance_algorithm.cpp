#include "spreading_activation.h"
#include "vertex.h"
#include "graph.h"
#include "relationship.h"
#include "impuls.h"
#include "shortest_path.h"
#include "distance_algorithm.h"

#include <boost/algorithm/string/replace.hpp>

#include <iostream>
#include <string>
#include <ostream>

#include "stdio.h"


namespace oc {

    double distance_algorithm::algorithm(graph& g, const std::string& a, const std::string& b, int count_nodes, std::string& result) {

        spreading_activation sa;
        shortest_path sp;

        std::string json_result;

        std::vector<std::tuple<oc::vertex*,double,std::vector<oc::Impuls<double>*>>>
                result_a = sa.algorithm(g, a, 10, 8, 0.0001,"",json_result);
        std::vector<std::tuple<oc::vertex*,double,std::vector<oc::Impuls<double>*>>>
                result_b = sa.algorithm(g, b, 10, 8, 0.0001,"",json_result);

        std::vector<long unsigned> forbidden_nodes;
        for (int j=0; j != count_nodes; ++j) {
            forbidden_nodes.push_back(std::get<0>(result_a[j])->get_id());
            forbidden_nodes.push_back(std::get<0>(result_b[j])->get_id());
        }

        double distance = 0.0;
        double divisor = 0;
        for (int i=1; i != count_nodes + 1; ++i) {
            divisor += i;
        }

        for (int i=0; i != count_nodes; ++i) {
            if (i != 0) {
                std::vector<long unsigned> fb_nodes_copy;
                std::copy(forbidden_nodes.begin(),forbidden_nodes.end(),std::back_inserter(fb_nodes_copy));
                std::vector<long unsigned>::iterator it_target_node = std::find(fb_nodes_copy.begin(),fb_nodes_copy.end(),std::get<0>(result_b[i])->get_id());
                fb_nodes_copy.erase(it_target_node, it_target_node + 1);
                double result = sp.algorithm(g, std::get<0>(result_a[i])->get_identifier(), std::get<0>(result_b[i])->get_identifier(), fb_nodes_copy);
                if (result == -1) result = 10;
                distance += ((count_nodes-i)/divisor) * result;
            } else {
                double result = sp.algorithm(g, std::get<0>(result_a[i])->get_identifier(), std::get<0>(result_b[i])->get_identifier(), {});
                if (result == -1) result = 10;
                distance += ((count_nodes-i)/divisor) * result;
            }
        }

        vertex* v_a = g[a];
        vertex* v_b = g[b];

        std::cout << "Total distance " << distance << std::endl;

        result += "{\"companyA\": {\"path\":\"" + v_a->get_identifier() + "\", \"name\":\"" + v_a->get_alias() + "\", \"type\":\"" + v_a->get_property("type") + "\"},";
        result += "\"companyB\": {\"path\":\"" + v_b->get_identifier() + "\", \"name\":\"" + v_b->get_alias() + "\", \"type\":\"" + v_b->get_property("type") + "\"},";
        result += "\"distance\": " + std::to_string(distance) + "}";

        return distance;
    }
}