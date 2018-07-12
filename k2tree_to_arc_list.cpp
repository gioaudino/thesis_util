#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <tuple>
#include <sdsl/k2_tree.hpp>

std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);

int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " <k2_compressed_basename> [<result_filename>]" << std::endl;
        exit(1);
    }

    std::string result_filename(argv[1]);
    result_filename.append(".arclist")

    if(argc == 3){
        result_filename = argv[2];
    }
    unsigned int nodes;
    unsigned long arcs;
    auto nodes_arcs = get_nodes_arcs(argv[1]);
    nodes = nodes_arcs.first;
    arcs = nodes_arcs.second;

    std::ifstream in_k2(argv[1]);

    sdsl::k2_tree<2> k2;
    k2.load(in_k2);

    long unsigned int x;
    int index;

    std::ofstream out(result_filename);

    for(x = 0; x < nodes; x++){
        auto neighbors = k2.neigh(x);
        for(index = 0; index < neighbors.size(); index++){
            out << x << '\t' << neighbors[index] << std::endl;
        }
    }
    out.close();

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
