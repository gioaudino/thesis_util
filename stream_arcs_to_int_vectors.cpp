#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>
#include <sdsl/int_vector.hpp>
#include <ctime>

std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);

int main(int argc, char** argv){
    const int int_len = 32;

    if(argc < 3){
        std::cout << "Usage: " << argv[0] << "<output> <original_basename> [--no-output]" << std::endl;
        exit(1);
    }

    unsigned int nodes;
    unsigned long arcs;
    auto nodes_arcs = get_nodes_arcs(argv[2]);
    nodes = nodes_arcs.first;
    arcs = nodes_arcs.second;

    bool debug = argc != 4;

    sdsl::int_vector<int_len> x_vector, y_vector;
    x_vector = sdsl::int_vector<int_len>(arcs, 0);
    y_vector = sdsl::int_vector<int_len>(arcs, 0);

    long unsigned int x, y, index = 0;
    time_t now;

    while (std::cin >> x >> y){
        if(debug && index % 1000000 == 0){
            now = time(0);
            std::cout << ctime(&now) << "\t" << index << "/" << arcs << " arcs - " << 100*index/arcs << "%%" << std::endl;
        }
        x_vector.set_int(index*int_len, x, int_len);
        y_vector.set_int(index*int_len, y, int_len);
        index++;
    }
    if(debug)
        std::cout << "Streaming completed. Serializing vectors" << std::endl;

    std::string x_n(argv[1]);
    x_n.append(".x");

    std::string y_n(argv[1]);
    y_n.append(".y");

    std::ofstream x_stream(x_n);
    std::ofstream y_stream(y_n);

    long unsigned int x_w = x_vector.serialize(x_stream);
    long unsigned int y_w = y_vector.serialize(y_stream);

    x_stream.close();
    y_stream.close();

    std::string prop(argv[1]);
    prop.append(".properties");
    std::ofstream properties_out(prop);

    properties_out << "arcs=" << arcs << std::endl;
    properties_out << "nodes=" << nodes << std::endl;
    if(debug){
        std::cout << "Written " << x_w+y_w << " bytes: " << 8*(x_w+y_w) << " bits" << std::endl;
        time_t t = time(0);
        std::cout << "Finished at " << ctime(&t) << std::endl;
    }
}

std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename){
    std::string properties_file(basename);
    properties_file.append(".properties");
    std::ifstream properties(properties_file);

    std::string line;
    std::vector<std::string> elems;
    unsigned long arcs;
    unsigned int nodes;
    while(properties >> line){
        elems = split(line, '=');
        if(elems[0].compare("arcs") == 0){
            arcs = std::stol(elems[1]);
        }
        if(elems[0].compare("nodes") == 0){
            nodes = std::stoi(elems[1]);
        }
    }
    properties.close();
    return std::make_pair(nodes, arcs);
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(std::move(item));
    }
    return elems;
}
