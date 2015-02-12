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

    void vertex::print_rel() {
        bool first = true;
        std::cout << "[" << std::endl;
        for (auto p = rel_out.begin(); p != rel_out.end(); ++p) {
            if (first) first = false;
            else std::cout << "," << std::endl;
            std::cout << "\t" << *p;
        }
        for (auto p = rel_in.begin(); p != rel_in.end(); ++p) {
            if (first) first = false;
            else std::cout << "," << std::endl;
            std::cout << "\t" << *p;
        }
        std::cout << std::endl << "]";
    }

}