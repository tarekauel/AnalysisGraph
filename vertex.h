#ifndef __oc_vertex__
#define __oc_vertex__
#include <vector>
#include <string>
#include <ostream>
#include <set>

#include <map>

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
        std::map<std::string,std::string> properties;
    public:
        vertex(oc::graph* g,const unsigned long id, const std::string identifier) :id{id},identifier{identifier},graph{g} {
        };

        void add_out(vertex*);
        void add_out(vertex*, const std::string&);

        void add_property(std::string k, std::string v) {
            properties.insert(std::pair<std::string,std::string>{k,v});
        }
        
        std::string get_identifier() const;
        long unsigned int get_id() const {return id;}
        
        std::vector<vertex*> get_neighbors() ;
        
        long unsigned num_out_edges() const {return rel_out.size();}
        long unsigned num_in_edges() const {return rel_in.size();}

        std::string get_alias() const;
        std::string get_alias_with_type() const;

        std::string get_property(const std::string& key) const {
            std::map<std::string,std::string>::const_iterator it = properties.find(key);
            if (it != properties.end()) {
                return it->second;
            }
            return "";
        }

        friend std::ostream& operator<<(std::ostream& out, const vertex& v) {
            out << "{\"id\":" << v.id << ",\"name\":\"" << v.identifier << "\"";
            for (auto p : v.properties) {
                out << ",\"" + p.first + "\":\"" + p.second + "\"";
            }
            out << "}";
            return out;
        }
    };

}

#endif