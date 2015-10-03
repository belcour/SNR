#include "SNR.hpp"

#include <string>
#include <iostream>

int main(int argc, char** argv)
{
	if(argc != 3) {
		std::cout << "Usage: SNR input_file.exr reference.exr" << std::endl;
		//BOOST_LOG_TRIVIAL(fatal) << "The required number of arguments is uncorrect";
		return EXIT_FAILURE;
	}

	std::string input_file(argv[1]) ;
	std::string ref_file(argv[2]) ;

	//float snr = SNR(input_file, ref_file) ;
	try {
	   float snr = Metric<SnrStatistics>(input_file, ref_file);
	   std::cout << "SNR  = " << snr << std::endl;

		float rmse = RMSE(input_file, ref_file) ;
	   std::cout << "RMSE = " << rmse << std::endl;
	} catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(fatal) << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
