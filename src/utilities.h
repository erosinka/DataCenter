#ifndef utilities_h
#define utilities_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <unistd.h>

struct Pair {
    int id_;
    int value_;

    Pair (int id = 0, int value = 0) {
        id_ = id;
        value_ = value;
    }
    static inline bool cmp (const Pair& a, const Pair& b) {
        return a.value_ < b.value_;
    }

	void print(std::ostream &out) const;
};

struct Server {
	int width_;
	int cap_;
    int id_;
    double dens_;
    int row_;
    int slot_;
    int pool_;
/*
    inline bool operator() (const Server& a, const Server& b) {
        return a.dens_ < b.dens_;
    }
*/    
    static inline bool cmp_dens (const Server& a, const Server& b) {
        return a.dens_ < b.dens_;
    }
    static inline bool cmp_width (const Server& a, const Server& b) {
        return a.width_ < b.width_;
    }
    //sort in decreasing order
    static inline bool cmp_cap (const Server& a, const Server& b) {
        return a.cap_ > b.cap_;
    }

    static inline bool cmp_id (const Server& a, const Server& b) {
        return a.id_ < b.id_;
    }
    static inline bool cmp_row (const Server& a, const Server& b) {
        return a.row_ < b.row_;
    }
    static inline bool cmp_slot (const Server& a, const Server& b) {
        return a.slot_ < b.slot_;
    }
    static inline bool cmp_pool (const Server& a, const Server& b) {
        return a.pool_ < b.pool_;
    }
	void print(std::ostream &out) const;
};

std::ostream &operator<<(std::ostream &out, const Server &s);
std::ostream &operator<<(std::ostream &out, const Pair &s);

std::vector<size_t>::iterator find_min_nz(std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end);


#endif
