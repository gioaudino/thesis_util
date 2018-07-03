#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>

std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);

int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << "<output> [<original_basename>]" << std::endl;
        exit(1);
    }

    std::vector<std::tuple<long unsigned int,long unsigned int>> arc_vector;
    long unsigned int x, y, max = 0, index = 0;
    while (std::cin >> x >> y){
        arc_vector.push_back(std::make_tuple(x,y));
        max = std::max(max, std::max(x,y));
        index++;
    }

    sdsl::k2_tree<2> k2;
    k2 = sdsl::k2_tree<2>(arc_vector, max+1);

    std::ofstream outfile(argv[1]);


    auto written = k2.serialize(outfile);
    outfile.close();

    unsigned int nodes;
    unsigned long arcs;
    std::string prop(argv[1]);
    prop.append(".properties");
    std::ofstream properties_out(prop);
    if(argc ==3){
        auto nodes_arcs = get_nodes_arcs(argv[2]);
        nodes = nodes_arcs.first;
        arcs = nodes_arcs.second;
    } else {
        nodes = max+1;
        arcs = index;
    }
    properties_out << "arcs=" << arcs << std::endl;
    properties_out << "nodes=" << nodes << std::endl;
    properties_out.close();

    double bpe = 8*written/arcs;

    std::cout << "Written " << written << " bytes: " << 8*written << " bits - " << std::setprecision(3) << bpe << " bpe" << std::endl;
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
