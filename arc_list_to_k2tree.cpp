#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <tuple>
#include <sdsl/int_vector.hpp>
#include <sdsl/k2_tree.hpp>

std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);


int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " <basename> [<arclist_file>] [<result_filename>]" << std::endl;
        exit(1);
    }
    std::string arclist_file(argv[1]);
    arclist_file.append(".arclist");

    std::string result_filename(argv[1]);
    result_filename.append(".k2compressed");

    if(argc == 4){
        arclist_file = argv[2];
        result_filename = argv[3];
    }
    if(argc ==3){
        result_filename = argv[2];
    }

    auto nodes_arcs = get_nodes_arcs(argv[1]);
    unsigned int nodes = nodes_arcs.first;
    unsigned long arcs = nodes_arcs.second;

    std::vector<std::tuple<long unsigned int,long unsigned int>> arc_vector;

    std::ifstream arclist(arclist_file);

    long unsigned int x, y;
    while (arclist >> x >> y){
        arc_vector.push_back(std::make_tuple(x,y));
    }
    arclist.close();
    sdsl::k2_tree<2> k2_representation;
    k2_representation = sdsl::k2_tree<2>(arc_vector, nodes);

    std::ofstream outfile(result_filename);

    auto written = k2_representation.serialize(outfile);
    outfile.close();

    std::cout << "Written " << written << " bytes: " << 8*written << " bits" << std::endl;

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
