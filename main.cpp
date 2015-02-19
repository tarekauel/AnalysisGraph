#include <iostream>
#include <fstream>
#include <execinfo.h>
#include <unistd.h>
#include "vertex.h"
#include "relationship.h"
#include "graph.h"
#include "spreading_activation.h"
#include "shortest_path.h"

void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void init(oc::graph& g, const std::string& data_path, const std::string& output_path) {

    std::vector<std::string> vertex_files {"City", "Country", "Region","Advisor", "Category", "Founder", "FundingRound", "HQ", "keywords", "Member", "Office", "organizations", "PrimaryImage", "TeamMember", "Website","companies_acquired_by_sap"};

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    try {
        for (auto f : vertex_files) {
            g.add_vertices_by_file(data_path + f + ".csv", {"path","url"});
        }
    } catch (std::string str) {
        std::cerr << str << std::endl;
        throw str;
    }

    g.add_edges_by_file(data_path + "edges_dump.csv");

    end = std::chrono::system_clock::now();
    std::cout << "Parsed all in " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() <<
            " ms" << std::endl;

    std::cout << std::endl << g << std::endl;

    std::vector<std::string> types {"Organization","City","Category","keyword","Region","Country","Person","TeamMember","Founder","Advisor"};
    std::ofstream auto_file {output_path + "auto.csv"};
    auto_file << "__id,type,name";
    for (auto v : g.get_vertices()) {
        if (std::find(types.begin(),types.end(),v->get_property("type")) != types.end()) {
            auto_file << std::endl << "\"" << v->get_identifier() << "\",\"" << v->get_property("type") << "\",\"" << v->get_alias() << "\"";
        }
    }
}

int main(int argc, char* argv[]) {
    signal(SIGSEGV, handler);

    std::string data_path = "/Users/tarek/ClionProjects/AnalysisGraph/data/";
    std::string output_path = "/Users/tarek/WebstormProjects/BackendDemo/public/";

    oc::graph g{};

    oc::spreading_activation spreading_activation;

    g.add_edges_by_file(data_path + "sa_demo.csv");
    //g.add_edges_by_file(data_path + "facebook_combined.csv");
    //g.add_edges_by_file(data_path + "output.txt");

    oc::shortest_path shortest_path;



    //init(g,data_path,output_path);
    std::vector<std::pair<oc::vertex*,std::pair<double,std::vector<oc::Impuls<double>*>>>> result;
    std::string id {"organization/big-data-elephants"};

    std::vector<unsigned long> forbidden_ids;
    forbidden_ids.push_back(g["D"]->get_id());

    //shortest_path.algorithm(g,"organization/big-data-elephants","organization/graphlab");
    //shortest_path.algorithm(g,"1","1489");
    shortest_path.algorithm(g,"A","E",forbidden_ids);
    //shortest_path.algorithm(g,"25","2");

    /*result = spreading_activation.algorithm(g, id, 10, 8, 0.00001, output_path + "spreading_activation_result.json");

    int i=0;
    for (auto p = result.begin(); p != result.end(); ++p) {
        std::cout << p->first->get_alias() << ":" << p->second.first << std::endl;
        ++i;
        if (i!=10) {
            continue;
        }
        break;
    }*/

    return 0;
}