#include <iostream>
#include <fstream> 
#include <string.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

using namespace std;
struct number_of_rows{
   static const int value(3);
};

//in each row index is a number of pools, value - is sum of capacity of pool i in this row
class row {
    private:
        vector<int> v;
    public:
        explicit row(const int size = 1024) {v.reserve(size);}
        // READ
        int& operator() (const unsigned int i) const {
            return v[i]; 
        }

        //WRITE
        int& operator() (const unsigned int i) {
            return v[i]; 
        }

        void push_back(const int val) {
           v.push_back(val); 
        }
        int size() const {return v.size();}
        int accumulate() {return accumulate(v.begin(), v.end(), 0);}
};

int partial_sum (const row& r, const unsigned int i ){
    return r.accumulate() - r(i);
}

class data {
    private:
        vector <row> v;
    public:
    
    explicit data (unsigned int nrows = 1024, unsigned int row_size = 1024) {
        v.reserve(nrows);
    }
  
    // READ 
    int& operator()(const unsigned int row_, const unsigned int pool_) const {return v[i](j);}
    // WRITE
    int& operator()(const unsigned int row_, const unsigned int pool_) {return v[i](j);}
};

vector<int> servers({1,2,3,4,5});

int compute (string fname) {
    ifstream output;
    output.open(fname.c_str());
    string s;
    char c;
    int row_, slot_, pool_; 
    int server_id = 0;
    vector <Pools> data;
    data.reserve(P);
    
    
    while (output.getline(s, LS) {
        if (!strcmp(s, NA))
            //line number in input is server_id 
            server_id++;
            continue;
        else {
           stringstream str(s);
           str >> row_;
           str >> slot_
           str >> pool_;
           data(row_, pool_) += servers[server_id];
        }

        server_id++;
    }
    return 1;

}

int main () {
    compute("output.txt") ;   
    return 0;
}
