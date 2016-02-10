#include <iostream>
#include "server_farm.h"

int main(int argc, char * const argv[]){
	ServerFarm sf("input.txt");
//	ServerFarm sf(argv[1]);
/*
	std::cout<<sf<<std::endl;

    sf.sort_dens();
    sf.print_servers(std::cout);

	while(!sf.servers_.empty()){
		sf.add_server();
	}
*/
    std::cout << "Room: " << std::endl;    
	std::cout<<sf<<std::endl;
    std::cout << "place servers" << std::endl;
    sf.place_servers();
    std::cout << "Room after placement: " << std::endl;    
	std::cout<<sf<<std::endl;
//    sf.print_placement(std::cout);
    std::cout << "assign pools" << std::endl;
    sf.assign_pools();
    std::cout << "Print pools placement:"<< std::endl;
    sf.print_pool(std::cout);
    std::cout << "output server data" << std::endl;
    sf.output_server_data("output.txt");
    std::cout << "calculate score" << std::endl;
    size_t score = sf.calc_score("output.txt");

    std::cout << " score = " << score << std::endl; 
	return 0;
}
