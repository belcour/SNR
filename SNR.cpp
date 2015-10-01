#include "SNR.hpp"

#include <string>
#include <iostream>

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		std::cout << "The required number of arguments is uncorrect" << std::endl ;
		return 1;
	}

	std::string input_file(argv[1]) ;
	std::string ref_file(argv[2]) ;

	//float snr = SNR(input_file, ref_file) ;

	float rmse = RMSE(input_file, ref_file) ;
   std::cout << rmse << std::endl;

	return 0;
}
