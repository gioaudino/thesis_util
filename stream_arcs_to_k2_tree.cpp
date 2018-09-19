#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sdsl/k2_tree.hpp>
#include <ctime>
#include <math.h>
#include <cstdlib>
#include <random>
#include <ef/elias_fano.h>

std::pair<unsigned int, unsigned int> get_min_max(const std::vector<unsigned int> times);
std::vector<std::string> split(const std::string &s, char delim);
std::pair<unsigned int,unsigned long> get_nodes_arcs(const std::string basename);
long double get_cpu_time(std::clock_t time_start, std::clock_t time_end, unsigned int precision = 3);
std::vector<double> create_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes, unsigned long arcs);
int get_proportionally_random_node(std::vector<double> out_degrees);
int double_binary_search(std::vector<double> out_degrees, int left, int right, double target);
double get_variance(std::vector<unsigned int> times, double average);
std::vector<int> get_proportionally_random_nodes(std::vector<double> out_degrees, int count);

std::vector<uint64_t> build_prefixed_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes);
elias_fano build_ef_from_k2tree(const sdsl::k2_tree<2> &tree, unsigned int nodes);
elias_fano get_ef_from_out_degree_array(std::vector<uint64_t> sum_out_degrees);

const std::vector<int> counts = {10,100,1000,10000};
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
    double bpe = 8*written/((double)arcs);
    std::setprecision(3);
    properties_out << "bitsperlink=" << bpe << std::endl;
    properties_out << "compression_time=" << compression_time << " ms" << std::endl;



    std::cout << std::endl << "Analyzing sequential scan" << std::endl;

    // START - SEQUENTIAL SCAN

    int keep = 0;
    time_start = std::clock();
    for(index = 0; index < nodes; index++){
        for(long unsigned int node: k2.neigh(index)){
            keep ^= node;
        }
    }
    time_end = std::clock();
    const volatile int ignored = keep;
    // END - SEQUENTIAL SCAN


    long double sequential_scan_time = get_cpu_time(time_start, time_end, 6)/arcs;
    long double edges_per_second = (double) arcs / get_cpu_time(time_start, time_end, 1);

    std::cout << "Sequential scan completed: " << sequential_scan_time << " ns per link - " << edges_per_second << " links per s" << std::endl << std::endl;
    properties_out << "sequential_scan_time_nspl=" << sequential_scan_time << " ns/link" << std::endl;
    properties_out << "sequential_scan_time_lps=" << edges_per_second << " link/s" << std::endl;


    std::cout << "Analyzing list scan with randomly selected nodes" << std::endl;

    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<int> dist(0, nodes);

    for(int count: counts){
        std::vector<unsigned int> times(count,0);
        int prevent = 0;
        for(int c = 0; c < count; c++){
            unsigned int node = dist(gen);
            time_start = std::clock();
            for(long unsigned int n: k2.neigh(node)){
                prevent ^= n;
            }
            time_end = std::clock();
            times[c] = get_cpu_time(time_start, time_end, 6);
        }
        const volatile int unused = prevent;
        int sum = 0;
        for(int t: times){
            sum += t;
        }
        double avg = (double) sum/count;
        double variance = get_variance(times, avg);
        auto min_max = get_min_max(times);
        std::cout << "Random scan @ " << count << std::endl;
        std::cout << "avg: " << avg << " ns" << " - min " << min_max.first << " ns - max " << min_max.second << " ns - variance: " << variance << " - std dev: " << sqrt(variance) << std::endl;
        properties_out << "random_" << count << "_avg=" << avg << " ns" << std::endl;
        properties_out << "random_" << count << "_min=" << min_max.first << " ns" << std::endl;
        properties_out << "random_" << count << "_max=" << min_max.second << " ns" << std::endl;
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

    for(int count: counts){
        std::vector<unsigned int> times(count,0);
        std::vector<int> random_nodes = get_proportionally_random_node_list(ef, nodes, count);
        int prevent = 0;
        for(int c = 0; c < count; c++){
            time_start = std::clock();
            for(long unsigned int n: k2.neigh(random_nodes[c])){
                keep ^= n;
            }
            time_end = std::clock();
            times[c] = get_cpu_time(time_start, time_end, 6);
        }
        const volatile int unused = prevent;
        int sum = 0;
        for(int t: times){
            sum += t;
        }
        auto min_max = get_min_max(times);
        double avg = (double) sum/count;
        double variance = get_variance(times, avg);
        std::cout << "Proportionally random scan @ " << count << std::endl;
        std::cout << "avg: " << avg << " ns" << " - min " << min_max.first << " ns - max " << min_max.second << " ns - variance: " << variance << " - std dev: " << sqrt(variance) << std::endl;
        properties_out << "proportionally_random_" << count << "_avg=" << avg << " ns" << std::endl;
        properties_out << "proportionally_random_" << count << "_min=" << min_max.first << " ns" << std::endl;
        properties_out << "proportionally_random_" << count << "_max=" << min_max.second << " ns" << std::endl;
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

std::vector<double> create_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes, unsigned long arcs){
    std::vector<double> out_degrees(nodes, 0);
    int index, sum = 0;
    for(index = 0; index < nodes; index++){
        sum += tree.neigh(index).size();
        out_degrees[index] = (double) sum/arcs;
    }
    return out_degrees;
}

std::vector<uint64_t> build_prefixed_out_degree_array(const sdsl::k2_tree<2> &tree, unsigned int nodes){
    std::vector<uint64_t> out_degrees(nodes, 0);
    out_degrees[0] = tree.neigh(0).size();
    int index, sum = 0;
    for(index = 1; index < nodes; index++){
        out_degrees[index] = out_degrees[index-1] + tree.neigh(index).size();
    }
    return out_degrees;
}

elias_fano get_ef_from_out_degree_array(std::vector<uint64_t> sum_out_degrees){
    return elias_fano(sum_out_degrees, sum_out_degrees[sum_out_degrees.size()-1]+1);
}

elias_fano build_ef_from_k2tree(const sdsl::k2_tree<2> &tree, unsigned int nodes){
    return get_ef_from_out_degree_array(build_prefixed_out_degree_array(tree, nodes));
}

std::vector<int> get_proportionally_random_node_list(elias_fano ef, unsigned int nodes, int count){
    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<int> dist(0, nodes);

    std::vector<int> random_nodes(count, 0);
    for(int i=0; i < count; i++){
        random_nodes[i] = ef.rank(dist(gen));
    }
    return random_nodes;
}

std::vector<int> get_proportionally_random_nodes(std::vector<double> out_degrees, int count){
    srand(time(NULL));
    std::vector<int> nodes(count, 0);
    double target;
    for(int i = 0; i < count; i++){
        target = (double) rand()/RAND_MAX;
        nodes[i] = double_binary_search(out_degrees, 0, out_degrees.size(), target);
    }
    return nodes;
}

int double_binary_search(std::vector<double> out_degrees, int left, int right, double target){
    if (right == left) return right;
    int mid = left + (right-left)/2;
    if(out_degrees[mid] == target || out_degrees[mid] > target && out_degrees[mid-1] < target) return mid;
    if (target < out_degrees[mid]) return double_binary_search(out_degrees, left, mid, target);
    return double_binary_search(out_degrees, mid, right, target);
}

double get_variance(std::vector<unsigned int> times, double average){
    int sum = 0;
    for(int tim: times){
        sum += pow(tim-average, 2);
    }
    return sum/(times.size()-1);
}
