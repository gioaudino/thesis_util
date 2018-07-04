#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>
#include <ctime>


std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);

int main(int argc, char** argv){
    const int int_len = 32;

    if(argc < 2){
        std::cout << "Usage: " << argv[0] << "<output>" << std::endl;
        exit(1);
    }

    unsigned int nodes;
    unsigned long arcs;
    auto nodes_arcs = get_nodes_arcs(argv[1]);
    nodes = nodes_arcs.first;
    arcs = nodes_arcs.second;

    std::cout << "Will try to use files " << argv[1] << ".x and " << argv[1] << ".y - This graphs has " << nodes << " nodes and " << arcs << " arcs" << std::endl;

    sdsl::k2_tree<2> k2 = sdsl::k2_tree<2>(argv[1], nodes);

    std::string output(argv[1]);
    output.append(".k2");

    std::ofstream out(output);

    auto written = k2.serialize(out);
    out.close();

    double bpe = 8*written/arcs;

    std::string prop(argv[1]);
    prop.append(".properties");
    std::ofstream properties_out(prop);

    properties_out << "arcs=" << arcs << std::endl;
    properties_out << "nodes=" << nodes << std::endl;
    properties_out << "bitsperlink=" << bpe << std::endl;
    properties_out.close();

    std::cout << "Written " << written << " bytes: " << 8*written << " bits - " << bpe << " bpe" << std::endl;
    time_t t = time(0);
    std::cout << "Finished at " << ctime(&t) << std::endl;

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
