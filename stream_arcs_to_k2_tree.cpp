#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>
#include <ctime>
#include <math.h>
#include <cstdlib>
#include <random>
#include "ef/elias_fano.h"

std::pair<unsigned int, unsigned int> get_min_max(const std::vector<unsigned int> times);
std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);
long double get_cpu_time(std::clock_t time_start, std::clock_t time_end, unsigned int precision = 3);
std::vector<double> create_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes, unsigned long arcs);
double get_variance(std::vector<unsigned int> times, double average);
double get_mean(std::vector<unsigned int> times);
std::vector<uint64_t> get_proportionally_random_nodes(const elias_fano ef, unsigned long nodes, int count);

std::vector<uint64_t> build_prefixed_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes);
elias_fano build_ef_from_k2tree(const sdsl::k2_tree<2> &tree, unsigned int nodes);
elias_fano get_ef_from_out_degree_array(std::vector<uint64_t> sum_out_degrees);

uint64_t nanos();
uint64_t micros();
uint64_t millis();

const std::vector<int> counts = {10,100,1000,10000};
const std::string MU = "\u03BC";

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
    uint64_t time_start = millis();
    sdsl::k2_tree<2> k2;
    k2 = sdsl::k2_tree<2>(arc_vector, nodes);

    auto written = k2.serialize(outfile);

    // END OF COMPRESSION TIME MEASURE
    uint64_t time_end = millis();

    uint64_t compression_time = time_end - time_start;


    std::cout << "Graph compressed in " << compression_time << " ms" << std::endl;

    outfile.close();
    double bpe = 8*written/((double)arcs);
    std::setprecision(3);
    properties_out << "bitsperlink=" << bpe << std::endl;
    properties_out << "compression_time=" << compression_time << " ms" << std::endl;



    std::cout << std::endl << "Analyzing sequential scan" << std::endl;

    // START - SEQUENTIAL SCAN

    int keep = 0;
    time_start = millis();
    for(index = 0; index < nodes; index++){
        for(long unsigned int node: k2.neigh(index)){
            keep ^= node;
        }
    }
    time_end = millis();
    const volatile int ignored = keep;
    // END - SEQUENTIAL SCAN


    long double sequential_scan_time = (double)(time_end-time_start)/arcs;
    long double edges_per_second = (double) arcs / (1000*(time_end-time_start));

    std::cout << "Sequential scan completed: " << sequential_scan_time << " ns per link - " << edges_per_second << " links per s" << std::endl << std::endl;
    properties_out << "sequential_scan_time_nspl=" << sequential_scan_time << " ns/link" << std::endl;
    properties_out << "sequential_scan_time_lps=" << edges_per_second << " link/s" << std::endl;


    std::cout << "Analyzing list scan with randomly selected nodes" << std::endl;

    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<int> dist(0, nodes);

    for(int count: counts){
        uint64_t time_0, time_1;
        std::vector<unsigned int> times(count,0);
        int prevent = 0;
        for(int c = 0; c < count; c++){
            unsigned int node = dist(gen);
            time_0 = micros();
            for(long unsigned int n: k2.neigh(node)){
                prevent ^= n;
            }
            time_1 = micros();
            times[c] = time_1 - time_0;
        }
        const volatile int unused = prevent;

        double avg = get_mean(times);
        double variance = get_variance(times, avg);
        auto min_max = get_min_max(times);
        std::cout << "Random scan @ " << count << std::endl;
        std::cout << "avg: " << avg << " " << MU << "s - min " << min_max.first << " " << MU << "s - max " << min_max.second << " " << MU << "s - variance: " << variance << " - std dev: " << sqrt(variance) << std::endl;
        properties_out << "random_" << count << "_avg=" << avg << " " << MU << "s" << std::endl;
        properties_out << "random_" << count << "_min=" << min_max.first << " " << MU << "s" << std::endl;
        properties_out << "random_" << count << "_max=" << min_max.second << " " << MU << "s" << std::endl;
        properties_out << "random_" << count << "_variance=" << variance << std::endl;
        properties_out << "random_" << count << "_stddev=" << sqrt(variance) << std::endl;

        // std::string of(std::to_string(count));
        // of.append("random");
        // std::ofstream os(of);
        // for(int t: times){
        //     os << t << std::endl;
        // }
        // os.close();
    }

    std::cout << std::endl << "Analyzing list scan with proportionally selected random nodes" << std::endl;

    // std::vector<double> out_degrees = create_out_degree_array(k2, nodes, arcs);

    elias_fano ef = build_ef_from_k2tree(k2, nodes);
    std::cout << "Elias Fano compressed list built" << std::endl;

    for(int count: counts){
        uint64_t time_0, time_1;
        std::vector<unsigned int> times(count,0);
        std::vector<uint64_t> random_nodes = get_proportionally_random_nodes(ef, arcs, count);
        std::cout << "Created random node list" << std::endl;
        //
        // for(int el: random_nodes){
        //     std::cout << el << " ";
        // }
        // std::cout << std::endl;

        int prevent = 0;
        for(int c = 0; c < count; c++){
            time_0 = micros();
            for(long unsigned int n: k2.neigh(random_nodes[c])){
                prevent ^= n;
            }
            time_1 = micros();
            times[c] = time_1 - time_0;
        }
        const volatile int unused = prevent;

        auto min_max = get_min_max(times);
        double avg = get_mean(times);
        double variance = get_variance(times, avg);
        std::cout << "Proportionally random scan @ " << count << std::endl;
        std::cout << "avg: " << avg << " " << MU << "s - min " << min_max.first << " " << MU << "s - max " << min_max.second << " " << MU << "s - variance: " << variance << " - std dev: " << sqrt(variance) << std::endl;
        properties_out << "proportionally_random_" << count << "_avg=" << avg << " " << MU << "s" << std::endl;
        properties_out << "proportionally_random_" << count << "_min=" << min_max.first << " " << MU << "s" << std::endl;
        properties_out << "proportionally_random_" << count << "_max=" << min_max.second << " " << MU << "s" << std::endl;
        properties_out << "proportionally_random_" << count << "_variance=" << variance << std::endl;
        properties_out << "proportionally_random_" << count << "_stddev=" << sqrt(variance) << std::endl;

        // std::string of(std::to_string(count));
        // of.append("prop");
        // std::ofstream os(of);
        // for(int t: times){
        //     os << t << std::endl;
        // }
        // os.close();
    }

    std::cout << std::endl << "Testing node adjacency" << std::endl;

    std::random_device random_seed;
    std::mt19937 random_generator(random_seed());
    std::uniform_int_distribution<int> distribution(0, nodes);

    for(int count: counts){
        std::vector<unsigned int> times(count,0);
        bool acc = false;
        uint64_t time_0, time_1;
        for(int i=0; i<count; i++){
            int j = distribution(random_generator), k = distribution(random_generator);
            time_0 = nanos();
            acc &= k2.adj(j, k);
            time_1 = nanos();
            times[i] = time_1 - time_0;
        }
        const volatile bool r = acc;

        auto min_max = get_min_max(times);
        double avg = get_mean(times);
        double variance = get_variance(times, avg);
        std::cout << "Adjacency @ " << count << std::endl;
        std::cout << "avg: " << avg << " ns" << " - min " << min_max.first << " \u03BCs - max " << min_max.second << " ns - variance: " << variance << " - std dev: " << sqrt(variance) << std::endl;
        properties_out << "adjacency_" << count << "_avg=" << avg << " ns" << std::endl;
        properties_out << "adjacency_" << count << "_min=" << min_max.first << " ns" << std::endl;
        properties_out << "adjacency_" << count << "_max=" << min_max.second << " ns" << std::endl;
        properties_out << "adjacency_" << count << "_variance=" << variance << std::endl;
        properties_out << "adjacency_" << count << "_stddev=" << sqrt(variance) << std::endl;
    }


    properties_out.close();
    std::cout << std::endl << "Written " << written << " bytes: " << 8*written << " bits - " << bpe << " bpe"<< std::endl;
    std::cout << "compression time: " << compression_time << " ms - sequential scan time: " << sequential_scan_time << " ns/link" << std::endl;
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

std::pair<unsigned int, unsigned int> get_min_max(const std::vector<unsigned int> times){
    if(times.empty())
        return std::make_pair(-1, -1);
    int min = times[0], max = times[0];
    for(int t: times){
        if(t < min) min = t;
        if(t > max) max = t;
    }
    return std::make_pair(min, max);
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

std::vector<uint64_t> build_prefixed_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes){
    uint64_t a = millis();
    std::vector<uint64_t> out_degrees(nodes, 0);
    uint64_t b = millis();

    // std::cout << "Array allocated in " << b-a << "ms" << std::endl;
    a = millis();
    out_degrees[0] = tree.neigh(0).size();
    int index, sum = 0;
    for(index = 1; index < nodes; index++){
        out_degrees[index] = out_degrees[index-1] + tree.neigh(index).size();
    }
    b = millis();
    std::cout << "Prefixed sum filled in " << b-a << "ms" << std::endl;

    return out_degrees;
}

elias_fano get_ef_from_out_degree_array(std::vector<uint64_t> sum_out_degrees){
    uint64_t a = millis();
    auto res = elias_fano(sum_out_degrees, sum_out_degrees[sum_out_degrees.size()-1]+1);
    uint64_t b = millis();

    std::cout << "EF built in " << b-a << "ms" << std::endl;
    return res;
}

elias_fano build_ef_from_k2tree(const sdsl::k2_tree<2> &tree, unsigned int nodes){
    return get_ef_from_out_degree_array(build_prefixed_out_degree_array(tree, nodes));
}

std::vector<uint64_t> get_proportionally_random_nodes(const elias_fano ef, unsigned long arcs, int count){
    // std::cout << "Requesting " << count << " random nodes out of " << arcs << " arcs" << std::endl;
    std::random_device seed;
    // std::cout << "ALPHA" << std::endl;
    std::mt19937 gen(seed());
    // std::cout << "BRAVO" << std::endl;
    std::uniform_int_distribution<int> dist(0, arcs);
    // std::cout << "CHARLIE" << std::endl;
    std::vector<uint64_t> random_nodes(count, 0);
    // std::cout << "DELTA" << std::endl;
    for(int i=0; i < count; i++){
        // std::cout << "ECHO " << i << std::endl;
        uint64_t r = dist(gen);
        // std::cout << "FOXTROT " << i << " | " << r << std::endl;
        uint64_t node = ef.rank(r);
        // std::cout << "Generated random number: " << r << " whose rank is " << node << std::endl;
        random_nodes[i] = node;
    }
    return random_nodes;
}

double get_variance(std::vector<unsigned int> times, double average){
    int sum = 0;
    for(int tim: times){
        sum += pow(tim-average, 2);
    }
    return sum/(times.size()-1);
}

double get_mean(std::vector<unsigned int> times){
    int sum = 0;
    for(int t: times){
        sum += t;
    }
    return (double) sum / (double) times.size();
}

uint64_t nanos() {
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
    return ns;
}

uint64_t micros() {
    uint64_t ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
    return ms;
}

uint64_t millis() {
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
    return ms;
}
