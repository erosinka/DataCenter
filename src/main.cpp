#include <iostream>
#include "server_farm.h"
#include "check.h"

int main(int argc, char * const argv[]){
	ServerFarm sf("input.txt");

    std::cout << "Room: " << std::endl;    
	std::cout<<sf<<std::endl;
    std::cout << "place servers" << std::endl;
    sf.place_servers();
    std::cout << "Room after placement: " << std::endl;    
	std::cout<<sf<<std::endl;
    std::cout << "assign pools" << std::endl;
    sf.assign_pools();
    std::cout << "Print pools placement:"<< std::endl;
    sf.print_pool(std::cout);
    std::cout << "output server data" << std::endl;
    sf.output_server_data("output.txt");
    std::cout << "calculate score" << std::endl;
    size_t score = sf.calc_score("output.txt");

    std::cout << " score = " << score << std::endl;
    
    check ch;
    ch.read_input("input.txt");
    ch.read_output("output.txt");
    std::cout << "check is " << ch.check_data() << std::endl;
    std::cout << "score:  " << ch.count_score() << std::endl;
	return 0;
}
