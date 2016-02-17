//
//  check.h
//  datacenter
//
//  Created by Elena Rosinskaya on 16.02.16.
//
//

#ifndef __datacenter__check__
#define __datacenter__check__

#include "utilities.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

struct na_server {
    static const char value = 'x';
};

class check {
private:
    std::vector <std::vector<int> > room;
    std::vector <Server> servers;
    size_t nrows_, nslots_, na_slots, npools_, nservers_;
    
public:
    void read_input(std::string filename);
    void read_output(std::string filename);
    bool check_data();
    size_t count_score();
};

#endif /* defined(__datacenter__check__) */
