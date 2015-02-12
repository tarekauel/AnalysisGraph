#ifndef __oc__graph__
#define __oc__graph__

#include <vector>
#include <unordered_map>

namespace oc {

    class vertex;

    class graph {
    private:
        std::vector<vertex*> vertex_list;
        std::unordered_map<std::string,vertex*> vertex_map;
    public:
        graph() {}
        void build_graph(std::ifstream&);
        ~graph();
        
        vertex* get_vertex(std::string);
        vertex* operator[](unsigned long i) {
            return vertex_list.at(i);
        }
        
        vertex* operator[](const std::string);
        
        unsigned long get_num_vertices() const {return vertex_list.size();}
        unsigned long get_num_edges() const;
        
        friend std::ostream& operator<<(std::ostream& out, const graph& g) {
            out << "{\"num_vertices\":" << g.get_num_vertices() << ", \"num_edges\":" << g.get_num_edges() << "}";
            return out;
        }
    };

}

#endif