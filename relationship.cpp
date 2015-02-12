//
//  relationship.cpp
//  AnalysisGraph
//
//  Created by Tarek Auel on 11.02.15.
//
//

#include <iostream>
#include "relationship.h"
#include "vertex.h"

namespace oc {
    
    std::ostream& relationship::print(std::ostream& out) const {
        if (!type.empty()) {
            out << source << "-[" << type << "]->" << target;
        } else {
            out << source << "-[]->" << target;
        }
        return out;
    }
    
    std::ostream& operator<<(std::ostream& out, const relationship& r) {
        if (!r.get_type().empty()) {
            out << r.get_source() << "-[" << r.get_type() << "]->" << r.get_target();
        } else {
            out << r.get_source() << "-[]->" << r.get_target();
        }
        return out;
    }
}