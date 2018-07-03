#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <tuple>
#include <sdsl/k2_tree.hpp>

std::vector<std::string> split(const std::string &s, char delim);

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: " << argv[0] << " <k2compressed_filename> <nodes> [<result_filename>]" << std::endl;
        exit(1);
    }
    auto input_name = split(argv[1], '.');
    std::string result_filename(input_name[input_name.size()-2]);
    unsigned int nodes = std::stoi(argv[2]);

    if(argc == 4){
        result_filename = argv[3];
    }
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
    out_stream.close();

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