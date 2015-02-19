#include <string>
#include <iostream>
#include "relationship.h"
#include "vertex.h"
#include "graph.h"

namespace oc {
    
    void vertex::add_out(vertex* target) {
        oc::relationship r (this,target);
        rel_out.push_back(r);
        neighbors.push_back((*graph)[target->get_id()]);
        sort( neighbors.begin(), neighbors.end() );
        neighbors.erase( unique( neighbors.begin(), neighbors.end() ), neighbors.end() );
        
        target->rel_in.push_back(r);
        target->neighbors.push_back((*graph)[get_id()]);
        sort( target->neighbors.begin(), target->neighbors.end() );
        target->neighbors.erase( unique( target->neighbors.begin(), target->neighbors.end() ), target->neighbors.end() );
        
    }
    
    void vertex::add_out(vertex* target,const std::string& type) {
        oc::relationship r (this,target,type);
        rel_out.push_back(r);
        neighbors.push_back((*graph)[target->get_id()]);
        sort( neighbors.begin(), neighbors.end() );
        neighbors.erase( unique( neighbors.begin(), neighbors.end() ), neighbors.end() );
        
        target->rel_in.push_back(r);
        target->neighbors.push_back((*graph)[get_id()]);
        sort( target->neighbors.begin(), target->neighbors.end() );
        target->neighbors.erase( unique( target->neighbors.begin(), target->neighbors.end() ), target->neighbors.end() );
    }
    
    std::string vertex::get_identifier() const {
        return identifier;
    }
    
    std::vector<vertex*> vertex::get_neighbors() {
        return neighbors;
    }

    std::string vertex::get_alias() const {
        std::map<std::string,std::string>::const_iterator it;
        std::string alias;
        it = properties.find("name");
        if (it != properties.end()) {
            alias = it->second;
        }
        it = properties.find("label");
        if (it != properties.end()) {
            alias = it->second;
        }
        it = properties.find("first_name");
        if (it != properties.end()) {
            std::string first_name = it->second;
            it = properties.find("last_name");
            if (it != properties.end()) {
                alias = first_name + " " + it->second;
            }
        }
        if (alias == "") alias = get_identifier();
        /*it = properties.find("type");
        if (it != properties.end()) {
            alias += " (" + it->second + ")";
        }*/
        return alias;
    }

    std::string vertex::get_alias_with_type() const {
        std::string alias = get_alias();
        std::string type = get_property("type");
        if (type != "") {
            return alias + " (" + type + ")";
        } else {
            return alias;
        }
    }

}