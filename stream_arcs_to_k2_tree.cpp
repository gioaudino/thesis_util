#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>
#include <ctime>
#include <math.h>

std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);
long double get_cpu_time(std::clock_t time_start, std::clock_t time_end, unsigned int precision = 3);

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: " << argv[0] << "<output_basename> <original_basename>" << std::endl;
        exit(1);
    }

    std::vector<std::tuple<long unsigned int,long unsigned int>> arc_vector;
    long unsigned int x, y, index = 0;

    unsigned int nodes;
    unsigned long arcs;

    auto nodes_arcs = get_nodes_arcs(argv[2]);
    nodes = nodes_arcs.first;
    arcs = nodes_arcs.second;

    std::string prop(argv[1]);
    prop.append(".properties");
    std::ofstream properties_out(prop);

    properties_out << "arcs=" << arcs << std::endl;
    properties_out << "nodes=" << nodes << std::endl;

    arc_vector = std::vector<std::tuple<long unsigned int, long unsigned int>>(arcs);
    while (std::cin >> x >> y){
        if(index % 1000000 == 0){
            time_t now = time(0);
            std::cout << ctime(&now) << "\t" << index << "/" << arcs << " arcs - " << 100*index/arcs << "\%" << std::endl;
        }
        arc_vector[index++] = std::make_tuple(x,y);
    }
    std::string output_basename(argv[1]);
    output_basename.append(".k2");
    std::ofstream outfile(output_basename);

    std::cout << "Graph loaded. Compressing..." << std::endl;

    // START MEASURING COMPRESSION TIME
    std::clock_t time_start = std::clock();
    sdsl::k2_tree<2> k2;
    k2 = sdsl::k2_tree<2>(arc_vector, nodes);

    auto written = k2.serialize(outfile);
    // END OF COMPRESSION TIME MEASURE
    std::clock_t time_end = std::clock();
    long double compression_time = get_cpu_time(time_start, time_end);


    std::cout << "Graph compressed in " << compression_time << " ms" << std::endl;



    outfile.close();
    double bpe = 8*written/arcs;
    std::setprecision(3);
    properties_out << "bitsperlink=" << bpe << std::endl;
    properties_out << "compression_time=" << compression_time << " ms" << std::endl;

    long unsigned int keep = 0;

    std::cout << "Analyzing sequential scan" << std::endl;

    time_start = std::clock();
    for(index = 0; index < nodes; index++){
        for(long unsigned int node: k2.neigh(index)){
            keep ^= node;
        }
    }
    time_end = std::clock();
    long double sequential_scan_time = get_cpu_time(time_start, time_end, 6)/arcs;

    std::cout << "Sequential scan completed: " << sequential_scan_time << " ns per link" << std::endl;


    properties_out << "sequential_scan_time=" << sequential_scan_time << " ns/link" << std::endl;

    properties_out.close();

    std::cout << std::endl << "Written " << written << " bytes: " << 8*written << " bits - " << std::endl;
    std::cout << '\t' << bpe << " bpe"<< std::endl;
    std::cout << "compression time: " << compression_time << " ms - sequential scan time: " sequential_scan_time << " ns/link" << std::endl;
    time_t t = time(0);
    std::cout << "Finished at " << ctime(&t) << std::endl;
}

long double get_cpu_time(std::clock_t time_start, std::clock_t time_end, unsigned int precision){
    unsigned int prec = pow(10, precision);
    long double cpu_time = prec*(time_end - time_start) / CLOCKS_PER_SEC;
    return cpu_time;
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
