#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <term.h>
#include <vector>
#include <string>

#include "graph.h"
#include "vertex.h"
#include "relationship.h"


namespace oc {

    void graph::add_edges_by_file(const std::string& filename, const std::string& delimiter) {
        std::ifstream in(filename, std::ifstream::in);
        std::chrono::time_point<std::chrono::system_clock> start, end_time;
        start = std::chrono::system_clock::now();

        if (!in.is_open()) {
            std::cerr << "File " << filename << " could not be opended" << std::endl;
            return;
        }
        std::string s;
        bool first = true;
        boost::regex regex("(?:^|" + delimiter + ")(?=[^\"]|(\")?)\"?((?(1)[^\"]*|[^" + delimiter + "\"]*))\"?(?=" + delimiter + "|$)");

        boost::sregex_iterator it;
        boost::sregex_iterator end;

        boost::regex buildRegex;
        boost::smatch matches;

        while (std::getline(in,s)) {
            if (first) {
                first = false;
                it = boost::sregex_iterator(s.begin(), s.end(), regex);
                std::string reg_string;
                bool inner_first = true;
                while (it != end) {
                    if (inner_first) inner_first = false;
                    else reg_string += delimiter;
                    reg_string += "([^" + delimiter + "]*)";
                    ++it;
                }
                //std::cout << reg_string << std::endl;
                buildRegex = boost::regex(reg_string);
            }
            else {
                if (boost::regex_search(s,matches,buildRegex)) {
                    std::string src = matches[1];
                    std::string dst = matches[2];
                    if (matches.size() == 5) {
                        std::string type = matches[3];
                        get_vertex(clean_string(src))->add_out(get_vertex(clean_string(dst)), clean_string(type));
                    } else {
                        get_vertex(clean_string(src))->add_out(get_vertex(clean_string(dst)));
                    }
                } else {
                    std::cerr << "Could not parse line: " << s << std::endl;
                }
            }
        }
        end_time = std::chrono::system_clock::now();
        std::cout << "Parsed file " + filename + " in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start).count() <<
                     " ms" << std::endl;
    }
    
    std::string& graph::clean_string(std::string& s) const {
        if (s.length() > 0 && s.at(0) == ',') {
            s.erase(s.begin(),s.begin() + 1);
        }
        if (s.length() > 2 && s.at(0) == '"' && s.at(s.length() - 1) == '"') {
            s.erase(s.begin(),s.begin() + 1);
            s.erase(s.end() - 1,s.end());
        }
        return s;
    }

    std::vector<std::string> graph::get_column_names(const std::string& s) const {
        boost::regex regex("(?:^|,)(?=[^\"]|(\")?)\"?((?(1)[^\"]*|[^,\"]*))\"?(?=,|$)");
        boost::sregex_iterator it (s.begin(), s.end(), regex);
        boost::sregex_iterator end;
        std::vector<std::string> column_names;
        for (; it != end ; ++it) {
            std::string column_name = it->str();
            clean_string(column_name);
            column_names.push_back(column_name);
        }
        return column_names;
    }

    void graph::add_vertices_by_file(const std::string& filename, const std::vector<std::string>& id_columns) {
        std::ifstream f(filename, std::ifstream::in);
        std::chrono::time_point<std::chrono::system_clock> start_chrono, end_chrono;
        start_chrono = std::chrono::system_clock::now();

        if (!f.is_open()) {
            std::cerr << "Could not open file: " + filename << std::endl;
            return;
        }

        bool first = true;
        std::string s;
        boost::regex regex("(?:^|,)(?=[^\"]|(\")?)\"?((?(1)[^\"]*|[^,\"]*))\"?(?=,|$)");

        std::vector<std::string> column_names;
        boost::sregex_iterator end;
        boost::sregex_iterator it;

        int column_no_identifier = -1;

        while (std::getline(f,s)) {
            if (first) {
                first = false;
                column_names = get_column_names(s);
                std::vector<std::string>::iterator id_column_it;
                for (auto id_column : id_columns) {
                    id_column_it = std::find(column_names.begin(), column_names.end(), id_column);
                    if (id_column_it != column_names.end()) {
                        break;
                    }
                }
                if (id_column_it != column_names.end()) {
                    column_no_identifier = id_column_it - column_names.begin();
                } else {
                    throw "No id column found: " + s;
                }
            } else {
                it = boost::sregex_iterator(s.begin(), s.end(), regex);
                std::vector<std::pair<std::string,std::string>> properties;
                vertex* v;
                bool found_id = false;
                for (int i=0; it != end; ++it,++i) {
                    std::string value = it->str();
                    clean_string(value);
                    if (value != "") {
                        properties.push_back(std::pair<std::string, std::string>{column_names.at(i), value});
                        if (i == column_no_identifier) {
                            v = get_vertex(value);
                        }
                        found_id = true;
                    }
                }
                if (!found_id) {
                    throw "Id " + column_names.at(column_no_identifier) + " not found in line \"" + s + "\"";
                }
                for (auto p : properties) {
                    v->add_property(p.first,p.second);
                }
            }
        }
        end_chrono = std::chrono::system_clock::now();
        std::cout << "Parsed file " + filename + " in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_chrono-start_chrono).count() <<
                " ms" << std::endl;
    }

    graph::~graph() {
        for (auto p : vertex_list) {
            delete p;
        }
    }
    
    vertex* graph::operator[](std::string identifier) {
        std::unordered_map<std::string,vertex*>::iterator it = vertex_map.find(identifier);
        if (it != vertex_map.end()) {
            return (*it).second;
        } else {
            throw std::out_of_range(identifier + " not found in graph");
        }
    }

    vertex* graph::get_vertex(std::string identifier) {
        std::unordered_map<std::string,vertex*>::iterator it = vertex_map.find(identifier);
        if (it == vertex_map.end()) {
            auto *v = new oc::vertex(this,vertex_list.size(), identifier);
            vertex_list.push_back(v);
            vertex_map[identifier] = v;
            return v;
        } else {
            return (*it).second;
        }
    }
    
    unsigned long graph::get_num_edges() const {
        unsigned long count{0};
        for (auto v : vertex_list) {
            count += v->num_out_edges();
        }
        return count;
    }
}