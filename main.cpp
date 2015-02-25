#include <iostream>
#include <fstream>
#include <execinfo.h>
#include <unistd.h>
#include "vertex.h"
#include "relationship.h"
#include "graph.h"
#include "spreading_activation.h"
#include "shortest_path.h"
#include "distance_algorithm.h"

#include <execinfo.h>
#include <errno.h>
#include <cxxabi.h>

static inline void printStackTrace( FILE *out = stderr, unsigned int max_frames = 63 ) {
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    unsigned int addrlen = backtrace( addrlist, sizeof( addrlist ) / sizeof( void* ));

    if ( addrlen == 0 )
    {
        fprintf( out, "  \n" );
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // Actually it will be ## program address function + offset
    // this array must be free()-ed
    char** symbollist = backtrace_symbols( addrlist, addrlen );

    size_t funcnamesize = 1024;
    char funcname[1024];

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for ( unsigned int i = 4; i < addrlen; i++ )
    {
        char* begin_name   = NULL;
        char* begin_offset = NULL;
        char* end_offset   = NULL;

        // find parentheses and +address offset surrounding the mangled name

        // OSX style stack trace
        for ( char *p = symbollist[i]; *p; ++p )
        {
            if (( *p == '_' ) && ( *(p-1) == ' ' ))
                begin_name = p-1;
            else if ( *p == '+' )
                begin_offset = p-1;
        }

        if ( begin_name && begin_offset && ( begin_name < begin_offset ))
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():
            int status;
            char* ret = abi::__cxa_demangle( begin_name, &funcname[0],
                    &funcnamesize, &status );
            if ( status == 0 )
            {
                auto funcname = ret; // use possibly realloc()-ed string
                fprintf( out, "  %-30s %-40s %s\n",
                        symbollist[i], funcname, begin_offset );
            } else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                fprintf( out, "  %-30s %-38s() %s\n",
                        symbollist[i], begin_name, begin_offset );
            }
        } else {
            // couldn't parse the line? print the whole line.
            fprintf(out, "  %-40s\n", symbollist[i]);
        }
    }

    free(symbollist);
}

void handler(int sig) {
    printStackTrace();
    exit(sig);
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
    init(g,data_path,output_path);

    oc::distance_algorithm da;

    std::string json_result;

    da.algorithm(g,"organization/big-data-elephants","organization/graphlab",5,json_result);

    std::cout << json_result << std::endl;


    return 0;
}