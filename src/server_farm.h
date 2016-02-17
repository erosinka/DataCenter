#ifndef server_farm_h
#define server_farm_h
#include "utilities.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <unistd.h>

class ServerFarm {
private:
	struct cap_less{
		bool operator() (const Server &x, const Server &y) const {
			return x.cap_ < y.cap_;
		}
	};

	std::vector<std::vector<int> > matrix_;

	size_t nrows_;
	size_t nslots_;
	size_t npools_;
	size_t nunavaiable_;
	size_t nservers_;

    // id_ = row number, value_ = number of n/a slots
    std::vector<Pair> na_slots_;
    // first free slot[row id]
    std::vector<int> first_slot_;
    // vector of (row, slot) placement for servers: output
    std::vector<Pair> placement_;
    void count_na_slots();
    double avg_cperpr_; // capacity/(pools*rows)
    double median_server_length_; 
public:
	std::priority_queue<Server, std::vector<Server>, cap_less> servers_;
    std::vector <Server> servers_v_;

	ServerFarm(std::string filename);

	void print(std::ostream &out) const;

    void print_servers(std::ostream &out) const;
    void print_placement(std::ostream &out) const;
    void print_pool(std::ostream &out) const;

	void add_server();
    int find_place_inrow(size_t row, size_t width);
    Pair find_place(size_t row, size_t width);
    void place_servers();
    void assign_pools();    
    void output_server_data(std::string outfile);
    
    void sort_dens() {
        std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_dens);
    }
 
    void sort_width() {
        std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_width);
    }

    void sort_id() {
        std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
    }
    void sort_row() {
        std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_row);
    }
    void sort_slot() {
        std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_slot);
    }

    size_t calc_score(std::string file); 
    
    void count_avg_cperpr();
    double get_avg_cperpr() {return avg_cperpr_;} 
};

std::ostream &operator<<(std::ostream &out, const ServerFarm &s);

#endif
