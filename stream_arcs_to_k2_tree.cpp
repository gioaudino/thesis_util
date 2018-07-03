#include <iostream>
#include <fstream>
#include <vector>
#include <sdsl/k2_tree.hpp>


int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << "<output>" << std::endl;
        exit(1);
    }

    std::vector<std::tuple<long unsigned int,long unsigned int>> arc_vector;
    long unsigned int x, y, max = 0;
    while (std::cin >> x >> y){
        arc_vector.push_back(std::make_tuple(x,y));
        max = std::max(max, std::max(x,y));
    }

    sdsl::k2_tree<2> k2;
    k2 = sdsl::k2_tree<2>(arc_vector, max+1);

    std::ofstream outfile(argv[2]);

    auto written = k2.serialize(outfile);
    outfile.close();

    std::cout << "Written " << written << " bytes: " << 8*written << " bits" << std::endl;
}
