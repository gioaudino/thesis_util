#include <sdsl/k2_tree.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char** argv){
    int i;

    if(argc <=1){
        std::cout << "Too few arguments: filename missing" << std::endl;
        exit;
    }
    sdsl::k2_tree<2> k2;
    k2 = sdsl::k2_tree<2>(argv[1], 0);
    std::ofstream outfile(argv[2]);

    k2.serialize(outfile, nullptr, argv[1]);
    outfile.close();
}
