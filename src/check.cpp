//
//  check.cpp
//  datacenter
//
//  Created by Elena Rosinskaya on 16.02.16.
//
//

#include "check.h"


void check::read_input(std::string filename){
    std::ifstream ifile(filename.c_str());
    if (!ifile.is_open()) {
        std::cout<<"Unable to open file"<<std::endl;
        exit(EXIT_FAILURE);
    }
    ifile >> nrows_;
    ifile >> nslots_;
    ifile >> na_slots;
    ifile >> npools_;
    ifile >> nservers_;
    
    room.resize(nrows_, std::vector <int>(nslots_));
    servers.resize(nservers_);
    
    int row, slot;
                  
    for (int i = 0; i < na_slots; i++) {
        ifile >> row;
        ifile >> slot;
        room[row][slot] = -1;
    }
    
    for (int s = 0; s < nservers_; s++) {
        ifile >> servers[s].width_;
        ifile >> servers[s].cap_;
        servers[s].id_ = s;
    }
   
    ifile.close();
}
                  
void check::read_output(std::string filename){
    std::ifstream ofile(filename.c_str());
    if (!ofile.is_open()) {
        std::cout << "Unable to open file with output" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string line;
    int s = 0;
    while (getline(ofile, line)) {
        if (s > nservers_) {
            std::cout << "Number of servers exceeded number in input file" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (!line.compare( "x")) {
            servers[s].pool_ = -1;
            servers[s].row_ = -1;
            servers[s].slot_ = -1;
        } else {
            std::stringstream str(line);
            str >> servers[s].row_;
            str >> servers[s].slot_;
            str >> servers[s].pool_;
            if (servers[s].row_ >= nrows_ or servers[s].slot_ >= nslots_ or servers[s].pool_ >= npools_) {
                std::cout << "Invalid value for server " << servers[s].id_ << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        s++;
    }
    if (s < nservers_) {
        std::cout << "Number of servers is less than number in input file" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    ofile.close();
    
}

bool check::check_data() {
    for (int s = 0; s < nservers_; s++) {
        int row = servers[s].row_;
        int slot = servers[s].slot_;
        int width = servers[s].width_;
        int id = servers[s].id_;
  
        if (row < 0) continue;
        
        if (nslots_ - slot < width) {
            std::cout << "Not enough space for server " << id << " in row " << row << " starting with slot " << slot << std::endl;
            exit(EXIT_FAILURE);
        }
        /*
        int i = 0;
        int val = room[row][slot];
        //check this (val + 1 = i)
        while (!val and i < width) {
            val = room[row][slot + i];
            i++;
            
        }
                */
        int val = 0, i = 0;
        for (i = 0; i < width; i++){
            val = room[row][slot + i];
            if (!val)
                room[row][slot + i] = id + 1;
            else break;
        }

        if (val == -1) {
            std::cout << "Server " << id << " is put into n/a slot " << slot + i << " row = " << row << std::endl;
            exit(EXIT_FAILURE);
        }
        if (val > 0) {
            std::cout << "Server " << id << " is put in occupied slot "<< slot + i <<" by server " << val-1<< " row = " << row << std::endl;
            exit(EXIT_FAILURE);
        }

        
    }
    return true;
}

size_t check::count_score() {
    
    std::vector< std::vector <int> > pools;
    pools.resize(npools_, std::vector<int> (nrows_));
    
    for (int s = 0; s< nservers_; s++) {
        if (servers[s].pool_ >= 0){
            pools[servers[s].pool_][servers[s].row_] += servers[s].cap_;
        }
    }
    size_t garanteed = SIZE_T_MAX;
    for (int p = 0; p<npools_; p++) {
        size_t max_row = *(std::max_element(pools[p].begin(), pools[p].end()));
        size_t full_cap = std::accumulate(pools[p].begin(), pools[p].end(), 0);
        garanteed = std::min(full_cap - max_row, garanteed);
    }
    return garanteed;
}