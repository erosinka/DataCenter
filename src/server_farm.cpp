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
	myfile >> nunavaiable_;
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
	for(size_t i = 0; i < nunavaiable_; ++i){
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


void ServerFarm::place_servers() {
    // sort is preserving order: 2 sorts will make servers be sorted by capacity (decreasing)
    // and for each capacity value - by size (increasing)
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_width);
    std::stable_sort(servers_v_.begin(), servers_v_.end(), Server::cmp_cap);


    size_t current_row = 0;
    size_t unwrapped_row = 0;
    for (size_t s = 0; s < nservers_; s++) {
        Server * server = &servers_v_[s];
//  std::cout << "Server: " << servers_v_[s];
            // for each r_th server we start placing with 0 (first) row
            // in case the server does not fit in current row R we move to next
            // until find place in the row F
            // and for next server has to start from row R until fitting except row F
            current_row = unwrapped_row % nrows_;
            Pair place = find_place(current_row, server->width_);
//  std::cout << "cur row = " << current_row << " place: " << place ;
            placement_[server->id_] = place;
            server->row_ = place.id_;
            server->slot_ = place.value_;
            if (place.id_ >= 0) {
                for (int i = 0; i < server->width_; i++) {
                    // +1 because 0 is for free slot in matrix
                    matrix_[place.id_][place.value_ + i] = server->id_ + 1;
                }
                if (place.id_ == current_row) {
                    unwrapped_row++;
                }
            }
    }
}

Pair ServerFarm::find_place(size_t row, size_t width) {
    for (size_t i = 0; i < nrows_; i++) {
        // example: nrows_ = 5 row = 3 r = {3 4 0 1 2}
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
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id); 
    for (int i = 0; i < nrows_; i++) {
        for (int j = 0; j < nslots_; j++) {
            if (matrix_[i][j] > 0) {
                int sid = matrix_[i][j] - 1;
      //          std::cout << "i="<< i << " j=" << j << " sid=" << sid <<  " cur_pool = " << cur_pool % npools_;
            
                servers_v_[sid].pool_ = cur_pool % npools_;
                cur_pool++;
                j = j + servers_v_[sid].width_ - 1;
       //         std::cout << "    server =" << servers_v_[sid] << " new j = " << j << " new cur_pool = " << cur_pool % npools_;
            }
       //     std::cout << std::endl;
        }
    }
}

void ServerFarm::output_server_data(std::string outfile){
    std::ofstream ofile(outfile.c_str());
    if(!ofile.is_open()){
       std::cout << "Unable to open output file"<<std::endl;
       exit(EXIT_FAILURE);
    }
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id); 
    for(size_t i = 0; i < nservers_; ++i){
        Server s = servers_v_[i];
        if(s.row_ >= 0)
            ofile << s.row_ << " " << s.slot_ << " " << s.pool_ << std::endl;
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


void ServerFarm::optimize_pools() {
    std::sort(servers_v_.begin(), servers_v_.end(), Server::cmp_id);
 
    //put this in separate func
    for(std::vector<size_t>& v : pool_caps)
        std::fill(v.begin(), v.end(), 0.);
    for (int s = 0; s<nservers_; s++){
        Server ser =servers_v_[s];
        if (ser.row_ >= 0)
            pool_caps[ser.pool_][ser.row_] += ser.cap_;
    }
    //find pool and row with smallest and larges capacity in some row
    Pair min_place = min_cap();
    Pair max_place = max_cap();
    // check the case when nothing happens in minmax func

    //take these two rows
    std::vector <int> row_min = matrix_[min_place.value_];
    std::vector <int> row_max = matrix_[max_place.value_];
    std::vector <int>:: iterator it;

    int cur_min_cap = MAX_INPUT;
    int cur_max_cap = 0;
    int id_min = -1;
    int id_max = -1;
    //find server for min row and pool with min capacity
    for (it = row_min.begin(); it != row_min.end(); it++) {
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
    for (it = row_max.begin(); it != row_max.end(); it++) {
        if (*it > 0 and servers_v_[*it-1].pool_ == max_place.id_ and servers_v_[*it-1].cap_ > cur_max_cap) {
            id_max = *it - 1;
        }
    }
    //swap servers
    //swap(&servers_v_[id_min].pool_, &servers_v_[id_max].pool_);
    int tmp_pool = servers_v_[id_min].pool_;
    servers_v_[id_min].pool_ = servers_v_[id_max].pool_;
    servers_v_[id_max].pool_ = tmp_pool;
    
    for(std::vector<size_t>& v : pool_caps)
        std::fill(v.begin(), v.end(), 0.);
    for (int s = 0; s<nservers_; s++){
        Server ser =servers_v_[s];
        if (ser.row_ >= 0)
            pool_caps[ser.pool_][ser.row_] += ser.cap_;
    }
    
    Pair p = min_cap();
}


//find pool giving min garanteed and row for this pool giving min row capacity
Pair ServerFarm::min_cap() {
    std::vector <size_t> pools;
    pools.resize(npools_);
    std::vector <size_t>::iterator it;
    int row, pool;
    Pair place(-1, -1);
    // for each pool choose the row with max part of that's pool capacity
    for(size_t p = 0; p < npools_; p++){
        size_t max_cap = *(std::max_element(pool_caps[p].begin(), pool_caps[p].end()));
        //for each pool count it's garanteed
        // may be zero if pool has servers only in 1 row
        if(p==38 or p==10) {
            int a = 6;
        }
        pools[p] = std::accumulate(pool_caps[p].begin(), pool_caps[p].end(),0) - max_cap;
        if (!pools[p]) {
            std::cout << "Pool " << p << " id represented only in 1 row." << std::endl;
            
        }
    }
    //pool with min garanteed
    it = std::min_element(pools.begin(), pools.end());
    pool = std::distance(pools.begin(), it);
    
    // optimize: find row in which this pool has min capacity: if zero, than re-assign pool for one of the servers for which pool
    // this won't affect garanteed capacity
    
    //row in which this pool has min non zero cap
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
    
    //row in which this pool has min cap
    // *it zero for the row in which pool is not represented
    //it = std::min_element(pool_caps[pool].begin(), pool_caps[pool].end());
    
    //custom min func for finding non-zero min element
    // *it = 54, row = 16
   // pool_caps[11][0]=111;
    it = std::max_element(pool_caps[pool].begin(),  pool_caps[pool].end());
    row = std::distance(pool_caps[pool].begin(), it);
    if (!*it) {
        std::cout << "Pool's " << pool << " min capacity is zero in row " << row << std::endl;
    }
    return Pair(pool, row);
}

std::vector<size_t>::iterator ServerFarm::find_min_nz(std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end) {
    size_t val = SIZE_T_MAX;
    std::vector<size_t>::iterator min_it;
    for (std::vector<size_t>::iterator it = begin; it != end; it++){
        size_t it_val = *it;
        if (it_val and val > it_val) {
            val = it_val;
            min_it = it;
        }
    }
    return min_it;
}

void ServerFarm::print(std::ostream &out) const{
	for(size_t i = 0; i < nrows_; ++i){
		for(size_t j = 0; j < nslots_; ++j){
            if (matrix_[i][j] > 0)
			    out<<matrix_[i][j]<<"\t";
                //out << servers_v_[matrix_[i][j] - 1].pool_ << "\t";
            else
			    out<<matrix_[i][j]<<"\t";
		}
		out<<std::endl;
	}
}

void ServerFarm::print_pool(std::ostream &out) const{
	for(size_t i = 0; i < nrows_; ++i){
		for(size_t j = 0; j < nslots_; ++j){
            if (matrix_[i][j] > 0)
			    //out<<matrix_[i][j]<<"\t";
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
		out<<servers_v_[i];
		out<<std::endl;
	}
}

std::ostream &operator<<(std::ostream &out, const ServerFarm &s){
	s.print(out);
	return out;
}

