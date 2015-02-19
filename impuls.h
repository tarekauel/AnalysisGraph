#ifndef __AnalysisGraph_impuls__
#define __AnalysisGraph_impuls__

#include <tbb/parallel_for_each.h>

#include "vertex.h"
#include "relationship.h"

namespace oc {
    template <typename T>
    class Impuls {
    private:
    public:
        Impuls() : node{NULL}, prev_impuls{nullptr} {
        };

        Impuls(vertex *v, T p) : node{v}, power{p}, prev_impuls{nullptr} {
        };

        vertex *node;
        T power;
        Impuls *prev_impuls;
        int hops = 0;

        static bool check_history(Impuls<T>* i, const oc::vertex* v) {
            if (i->node == v) {
                return true;
            } else {
                if (i->prev_impuls != nullptr) {
                    return check_history(i->prev_impuls, v);
                } else {
                    return false;
                }
            }
        }

        static std::vector<vertex*>& resolve_history(Impuls<T>* i,std::vector<vertex*>& history) {
            history.insert(history.begin(),i->node);
            if (i->prev_impuls != nullptr) {
                return resolve_history(i->prev_impuls,history);
            } else {
                return history;
            }
        }

        static void clean_list(std::vector<Impuls<T>*>*);
    };

    template<typename T>
    void Impuls<T>::clean_list(std::vector<Impuls<T>*>* vector) {
        tbb::parallel_for_each(vector->begin(), vector->end(), [](Impuls<T>* i){delete i;});
        delete vector;
    }
}

#endif