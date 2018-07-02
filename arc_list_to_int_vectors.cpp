#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <sdsl/int_vector.hpp>

std::vector<std::string> split(const std::string &s, char delim);
unsigned long get_arcs(const std::string basename);


int main(int argc, char** argv){
    const int int_size = 32;
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " <basename>" << std::endl;
        exit(1);
    }

    std::string arclist_file(argv[1]);
    arclist_file.append(".arclist");

    unsigned long arcs = get_arcs(argv[1]);

    std::vector<std::tuple<int,int>> arc_vector;

    sdsl::int_vector<int_size> fst = sdsl::int_vector<int_
    std::ifstream arclist(arclist_file);

    unsigned int x, y, index = 0;
    while (arclist >> x >> y){
        arc_vector.push_back(std::make_tuple(x,y));
        index++;
    }
    arclist.close();

    


}
unsigned long get_arcs(const std::string basename){
    std::string properties_file(basename);
    properties_file.append(".properties");
    std::ifstream properties(properties_file);

    std::string line;
    std::vector<std::string> elems;
    unsigned long arcs;
    while(properties >> line){
        elems = split(line, '=');
        if(elems[0].compare("arcs") == 0){
            arcs = atol(elems[1].c_str());
            break;
        }
    }
    properties.close();
    return arcs;
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
