#include "server_farm.h"

ServerFarm::ServerFarm(std::string filename){
	std::string line;
	std::ifstream myfile(filename.c_str());
	if (!myfile.is_open()){
		std::cout<<"Unable to open file"<<std::endl;
		exit(EXIT_FAILURE);
    }
	myfile >> nrows_;
	myfile >> nslots_;
	myfile >> nunavailable_;
	myfile >> npools_;
	myfile >> nservers_;

	matrix_.resize(nrows_);
	for(size_t i = 0; i < nrows_; ++i){
		matrix_[i].resize(nslots_);
	}
    na_slots_.resize(nrows_);
    first_slot_.resize(nrows_);
    servers_v_.resize(nservers_);
    placement_.resize(nservers_);
    
	size_t x,y;
    //read positions of unavailable slots
	for(int i = 0; i < nunavailable_; ++i){
		myfile >> x;
		myfile >> y;
		matrix_[x][y] = -1;
        // count unavailable slots per row
        na_slots_[x].id_ = x;
        na_slots_[x].value_ += 1; ;
	}
    
    for(size_t i = 0; i < nservers_; ++i){
		Server newServer;
		myfile >> newServer.width_;
		myfile >> newServer.cap_;
        newServer.id_ = i;
        newServer.dens_ = newServer.cap_ * 1.0 / newServer.width_;
		servers_.push(newServer);
        servers_v_[i] = newServer;
	}

	myfile.close();
}

void ServerFarm::count_na_slots(){
        for (size_t i = 0; i < nrows_; i++) {
            int num_slots = std::accumulate(matrix_[i].begin(), matrix_[i].end(), 0);
            // minus because in matrix there are 0 and -1
            na_slots_[i] = Pair(i, -num_slots);
        }
}

void ServerFarm::count_avg_cperpr() {
    double sum_capacity = 0.0;
    for (int i = 0; i < nservers_; i++){
        sum_capacity += servers_v_[i].cap_;   
    }
    avg_cperpr_ = sum_capacity * 1.0/(npools_ * nrows_); 
}

/*
void ServerFarm::place_servers() {
    // sort is preserving order: 2 sorts will make servers be sorted by capacity (decreasing)
    // and for each capacity value - by size (increasing)
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_width);
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_cap);
    
    size_t current_row = 0;
    size_t unwrapped_row = 0;
    size_t last_width = SIZE_T_MAX;
    
    for (size_t s = 0; s < nservers_; s++) {
        Server * server = &servers_v_[s];
        //  std::cout << "Server: " << servers_v_[s];
        // for first server start placing with 0 row.
        // for each r_th server we start placing with next row
        // in case the server does not fit in current row R we move to next row
        // until find place in the row F
        // and for next server has to start from row R until fitting except row F
        current_row = unwrapped_row % nrows_;
        Pair place = find_place(current_row, server->width_);
        //  std::cout << "cur row = " << current_row << " place: " << place ;
        placement_[server->id_] = place;
        server->row_ = place.id_;
        server->slot_ = place.value_;
        if (place.id_ >= 0) {
       // if (place.id_ < 0) {
       //     last_width = server->width_;
       // } else {
            for (int i = 0; i < server->width_; i++) {
                // id+1 because 0 is for free slot in matrix
                matrix_[place.id_][place.value_ + i] = server->id_ + 1;
            }
            if (place.id_ == current_row) {
                unwrapped_row++;
            }
        }
    }
}
*/
 

void ServerFarm::place_servers() {
    // sort is preserving order: 2 sorts will make servers be sorted by capacity (decreasing)
    // and for each capacity value - by size (increasing)
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_width);
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_cap);

    size_t current_row = 0;
    size_t unwrapped_row = 0;
    
    for (std::vector<Server>::iterator it = servers_v_.begin(); it!=servers_v_.end(); it++) {
            // for first server start placing with 0 row.
            // for each r_th server we start placing with next row
            // in case the server does not fit in current row R we move to next row
            // until find place in the row F
            // and for next server has to start from row R until fitting
            current_row = unwrapped_row % nrows_;
            Pair place = find_place(current_row, it->width_);
            placement_[it->id_] = place;
            it->row_ = place.id_;
            it->slot_ = place.value_;
  
            if (place.id_ >= 0) {
                for (int i = 0; i < it->width_; i++) {
                    // id+1 because 0 is for free slot in matrix
                    matrix_[place.id_][place.value_ + i] = it->id_ + 1;
                }
                if (place.id_ == current_row) {
                    unwrapped_row++;
                }
            } else {
                // if server was not placed and width of next one is not less - skip next server
                if (it+1 != servers_v_.end() )
                    if ((it+1)->width_ >= it->width_) {
                        it++;
                        it->row_ = -1;
                        it->slot_ = -1;
                    }
            }
    }
    //put servers in right order
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
}


Pair ServerFarm::find_place(size_t row, size_t width) {
    for (size_t i = 0; i < nrows_; i++) {
        // example: nrows_ = 5; row = 3; r = {3 4 0 1 2};
        int r = (i+row) % nrows_;
        int slot = find_place_inrow(r, width);
        if (slot >= 0) {
            return Pair(r, slot);
        }
    }
    return Pair(-1, -1);
}

// for rows and available slots we can use sparse matrix format: (number of element, number of consecutive free slots)
int ServerFarm::find_place_inrow(size_t row, size_t width) {
    size_t consecutive = 0;
    //for (size_t sl = first_slot_[row]; sl < nslots_; sl++) {
    for (size_t sl = 0; sl < nslots_; sl++) {
        if (matrix_[row][sl] == 0) {
            consecutive++;
        }
        if (consecutive == width) {
            return (sl - consecutive + 1);
        }
        if (matrix_[row][sl] != 0 ) {
            consecutive = 0;
        }
    }
    return -1;
}


void ServerFarm::assign_pools () {
    int cur_pool = 0;
    // sort servers by id for assigning pools
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id); 
    for (int i = 0; i < nrows_; i++) {
        for (int j = 0; j < nslots_; j++) {
            if (matrix_[i][j] > 0) {
                int sid = matrix_[i][j] - 1;
                servers_v_[sid].pool_ = cur_pool % npools_;
                cur_pool++;
                j = j + servers_v_[sid].width_ - 1;
                if (j < 0) {
                    std::cout << "Server "<< sid << "must have zero width." << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void ServerFarm::output_server_data(std::string outfile) {
    std::ofstream ofile(outfile.c_str());
    if(!ofile.is_open()){
       std::cout << "Unable to open output file"<<std::endl;
       exit(EXIT_FAILURE);
    }
    // sort servers by id for output
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
    for (std::vector<Server>::iterator it = servers_v_.begin(); it!=servers_v_.end(); it++) {
        if (it->row_ >= 0)
            ofile << it->row_ << " " << it->slot_ << " " << it->pool_ << std::endl;
        else
            ofile <<"x"<< std::endl;
    }
    ofile.close();
}

//separate into read output file and calc score
size_t ServerFarm::calc_score(std::string file) {
    std::ifstream myfile(file.c_str());
    std::string line;

    if (!myfile.is_open()) {
        std::cout<<"Unable to open file (calculate)"<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    //pool_caps tracks pool capacities per row, initialize to zero
    pool_caps.resize(npools_, std::vector<size_t>(nrows_, 0));
    for(std::vector<size_t>& v : pool_caps)
        std::fill(v.begin(), v.end(), 0.);
    
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
    //extract data from file
    size_t row;
    size_t slot;
    size_t pool;
    size_t index = 0;

    while (getline(myfile, line)) {
        //if line does not equal "x"
        if (line.compare("x") != 0){
            std::stringstream stream(line);
            stream >> row;
            stream >> slot;
            stream >> pool;
            (void)slot; //col is not used
    
            //servers_ has the same order as the output file after sorting them with cmp_id
            //so can get capacity from there
            pool_caps[pool][row] += servers_v_[index].cap_;
        }
        ++index;
    }
        
    myfile.close();
    //now use extracted data to calculate guaranteed capacity
    size_t guaranteed = (size_t)-1;
    for(size_t i = 0; i < npools_; ++i){
        // for each pool choose the row with max part of that's pool capacity
        size_t max_cap = *(std::max_element(pool_caps[i].begin(), pool_caps[i].end()));
        size_t sum = std::accumulate(pool_caps[i].begin(), pool_caps[i].end(),0);

        //guaranteed = min of (sum - max_cap) for all pools
        guaranteed = std::min(guaranteed, (sum - max_cap));
    }
    return guaranteed;
}

int ServerFarm::score() {
    count_pool_caps();
    std::vector <size_t> pools;
    pools.resize(npools_);
    std::vector <size_t>::iterator it;
    for (size_t p = 0; p < npools_; p++){
        size_t max_cap = *(std::max_element(pool_caps[p].begin(), pool_caps[p].end()));
        // pools[p] - garanteed capacity for pool p (in case one row with max capacity for this pool is out of order)
        pools[p] = std::accumulate(pool_caps[p].begin(), pool_caps[p].end(),0) - max_cap;
        if (!pools[p]) {
            std::cout << "Pool " << p << " id represented only in 1 row." << std::endl;
        }
    }
    //pool with min garanteed
    it = std::min_element(pools.begin(), pools.end());
    return *it;
}

void ServerFarm::optimize_pools() {
    int prev_score = score();
    int i = 1;
    server_exchange();
    int cur_score = score();
    //while (prev_score < cur_score) {
    for (int k = 0; k < 25; k++) {
        prev_score = cur_score;
        server_exchange();
        cur_score = score();
        std::cout << "exchange #" << i++ << " score = " << cur_score << std::endl;
    }
}

void ServerFarm::server_exchange() {
//    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
 
    count_pool_caps();
    
    //find pool and row with smallest and larges capacity in some row
    Pair min_place = min_cap();
    Pair max_place = max_cap();
    //?? check the case when nothing happens in minmax func

    //??
    int cur_min_cap = MAX_INPUT;
    
    int cur_max_cap = 0;
    int id_min = -1;
    int id_max = -1;
    
    //find server for min row and pool with min capacity
    for (std::vector <int>::iterator it = matrix_[min_place.value_].begin(); it != matrix_[min_place.value_].end(); it++) {
        // if there is server id in slot
        if (*it > 0 )
            // if pool of this server is chosen one
            if (servers_v_[(*it) - 1].pool_ == min_place.id_)
                // choose the server in this row and pool with largest capacity
                if (servers_v_[*it - 1].cap_ < cur_min_cap) {
                    id_min = *it - 1;
                }
    }
    //find server for max row and pool with max capacity
    for (std::vector <int>::iterator it = matrix_[max_place.value_].begin(); it != matrix_[max_place.value_].end(); it++) {
        if (*it > 0 and servers_v_[*it-1].pool_ == max_place.id_ and servers_v_[*it-1].cap_ > cur_max_cap) {
            id_max = *it - 1;
        }
    }
    
    if(id_max < 0 or id_min < 0) {
        std::cout << "Id_min or id_max not found" << std::endl;
        return;
    }
    
    // update pools' capacities
    pool_caps[servers_v_[id_min].pool_][servers_v_[id_min].row_] += servers_v_[id_max].cap_ - servers_v_[id_min].cap_;
    pool_caps[servers_v_[id_max].pool_][servers_v_[id_max].row_] += servers_v_[id_min].cap_ - servers_v_[id_max].cap_;
 
    //swap servers
    std::swap((servers_v_[id_min].pool_), (servers_v_[id_max].pool_));
}


//find pool which gives min garanteed and row for this pool with min row capacity
Pair ServerFarm::min_cap() {
    std::vector <size_t> pools;
    pools.resize(npools_);
    std::vector <size_t>::iterator it;
    int row, pool;
    Pair place(-1, -1);
    // for each pool choose the row with max part of that's pool capacity
    for(size_t p = 0; p < npools_; p++){
        size_t max_cap = *(std::max_element(pool_caps[p].begin(), pool_caps[p].end()));
        // pools[p] - garanteed capacity for pool p (in case one row with max capacity for this pool is out of order)
        pools[p] = std::accumulate(pool_caps[p].begin(), pool_caps[p].end(),0) - max_cap;
        if (!pools[p]) {
            std::cout << "Pool " << p << " id represented only in 1 row." << std::endl;
        }
    }
    //pool with min garanteed
    it = std::min_element(pools.begin(), pools.end());
    pool = std::distance(pools.begin(), it);
    if (*it == 0) {
        //can happen if number of placed servers is less than number of pools
        std::cout << "Pool " << pool << " is not in datacenter" <<std::endl;
        exit(EXIT_FAILURE);
    }
    
    // optimize: find row in which this pool has min capacity: if zero, than re-assign pool for one of the servers for which pool
    // this won't affect garanteed capacity
    
    //row in which this pool has min non zero cap (that's why std::min_element is not used)
    it = find_min_nz(pool_caps[pool].begin(), pool_caps[pool].end());
    row = std::distance(pool_caps[pool].begin(), it);
    std::cout << "min place: pool = " << pool << " row = " << row <<std::endl;
    return Pair(pool, row);
}

Pair ServerFarm::max_cap() {
    std::vector <size_t> pools;
    pools.resize(npools_);
    std::vector <size_t>::iterator it;
    int row, pool;
    Pair place(-1, -1);
    // for each pool choose the row with min part of that's pool capacity
    for(size_t p = 0; p < npools_; p++){
        size_t min_cap = *(find_min_nz(pool_caps[p].begin(), pool_caps[p].end()));
        //for each pool count it's garanteed
        // zero only if pool is not represented
        pools[p] = std::accumulate(pool_caps[p].begin(), pool_caps[p].end(),0) - min_cap;
    }
    //pool with max garanteed
    it = std::max_element(pools.begin(), pools.end());
    pool = std::distance(pools.begin(), it);
    
    it = std::max_element(pool_caps[pool].begin(), pool_caps[pool].end());
    row = std::distance(pool_caps[pool].begin(), it);
    if (!(*it)) {
        std::cout << "Pool's " << pool << " min capacity is zero in row " << row << std::endl;
    }
    std::cout << "max place: pool = " << pool << " row = " << row <<std::endl;
    return Pair(pool, row);
}

void ServerFarm::count_pool_caps() {
    for(std::vector<size_t>& v : pool_caps)
        std::fill(v.begin(), v.end(), 0.);
    
    for (std::vector<Server>::iterator it = servers_v_.begin(); it!=servers_v_.end(); it++) {
        if (it->row_ >= 0)
            pool_caps[it->pool_][it->row_] += it->cap_;
    }
}

void ServerFarm::print_room(std::ostream &out) const{
	for(size_t i = 0; i < nrows_; ++i){
		for(size_t j = 0; j < nslots_; ++j){
          //  out<<matrix_[i][j]<<"\t";
            
            if (matrix_[i][j] > 0)
                out << matrix_[i][j] - 1 <<"\t";
            else {
                if (matrix_[i][j] == 0)
                    out<<"x\t";
                else
                    out<<matrix_[i][j]<<"\t";
            }
            
		}
		out<<std::endl;
	}
}

void ServerFarm::print_pool(std::ostream &out) const{
	for(size_t i = 0; i < nrows_; ++i){
		for(size_t j = 0; j < nslots_; ++j){
            if (matrix_[i][j] > 0)
                out << servers_v_[matrix_[i][j] - 1].pool_ << "\t";
            else
			    out<<matrix_[i][j]<<"\t";
		}
		out<<std::endl;
	}
}

void ServerFarm::print_placement(std::ostream &out) const{
	for(size_t i = 0; i < nservers_; ++i){
		out << placement_[i] << std::endl;
	}
    out << std::endl;
}

void ServerFarm::print_servers(std::ostream &out) const{
	for(size_t i = 0; i < nservers_; ++i){
        out<<servers_v_[i] << std::endl;
    }
}

std::ostream &operator<<(std::ostream &out, const ServerFarm &s){
	s.print_room(out);
	return out;
}

