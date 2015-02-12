#ifndef __oc_vertex__
#define __oc_vertex__
#include <vector>
#include <string>
#include <ostream>

#include <set>

namespace oc {

    class relationship;
    class graph;

    class vertex {
    private:
        const long unsigned id;
        const std::string identifier;
        graph* graph;
        std::vector<vertex*> neighbors;
        std::vector<relationship> rel_out;
        std::vector<relationship> rel_in;
    public:
        vertex(oc::graph* g,const unsigned long id, const std::string identifier) :id{id},identifier{identifier},graph{g} {
        };

        void add_out(vertex*);
        //void add_in(vertex*);
        
        void add_out(vertex*, const std::string&);
        //void add_in(vertex*, const std::string&);
        
        std::string get_identifier() const;
        long unsigned int get_id() const {return id;}
        
        std::vector<vertex*> get_neighbors() ;
        
        long unsigned num_out_edges() const {return rel_out.size();}
        long unsigned num_in_edges() const {return rel_in.size();}

        void print_rel();

        friend std::ostream& operator<<(std::ostream& out, const vertex& v) {
            out << "{\"id\":" << v.id << ",\"name\":\"" << v.identifier << "\"}";
            return out;
        }
    };

}

#endif