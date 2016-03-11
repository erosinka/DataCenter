#include "utilities.h"


void Pair::print(std::ostream &out) const{
	out<<id_<<" " << value_ << std::endl;
}
void Server::print(std::ostream &out) const{
	//out<< "id: " <<id_<<" w: " << width_ << " cap: " << cap_ << " " << dens_ << std::endl;
	out<< "id: " <<id_<<" w: " << width_ << " cap: " << cap_  << " pool: "<< pool_ << std::endl;
}

std::ostream &operator<<(std::ostream &out, const Server &s){
	s.print(out);
	return out;
}

std::ostream &operator<<(std::ostream &out, const Pair &s){
	s.print(out);
	return out;
}


std::vector<size_t>::iterator find_min_nz(std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end) {
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