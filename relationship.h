#ifndef __oc_relationship__
#define __oc_relationship__

#include <string>

namespace oc {
    
    class vertex;

    class relationship {
    private:
        vertex* source;
        vertex* target;
        const std::string type;
    public:
        relationship(vertex* source, vertex* target) :source{source},target{target}{
        };
        relationship(vertex* source, vertex* target, const std::string& type) :source{source},target{target},type{type} {
        };
        
        vertex* get_source() const {
            return source;
        }
        
        vertex* get_target() const {
            return target;
        }
        
        std::string get_type() const {
            return type;
        }
        
        std::ostream& print(std::ostream&) const;
        
    };
    
    std::ostream& operator<<(std::ostream& out, const relationship& r);

}

#endif