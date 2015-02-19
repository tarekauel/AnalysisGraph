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
        std::string& clean_string(std::string&) const;
        std::vector<std::string> get_column_names(const std::string&) const;
    public:
        graph() {}
        void add_edges_by_file(const std::string& filename,const std::string& delimiter = ",");
        void add_vertices_by_file(const std::string&,const std::vector<std::string>&);


        ~graph();
        
        vertex* get_vertex(std::string);
        vertex* operator[](unsigned long i) {
            return vertex_list.at(i);
        }
        std::vector<vertex*>& get_vertices() {
            return vertex_list;
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