#include <iostream>

#include "../boost_1_60_0/boost/program_options/options_description.hpp"

#include "../io/io_libsvm.hpp"
#include "../io/io_binary.hpp"

#include "../lasvm/vector.hpp"

using namespace std;
namespace po = boost::program_options;

map<unsigned long, lasvm_sparsevector_t> X; // feature vectors
map<unsigned long, int> Y;                   // labels
unsigned long number_of_features = 0;
unsigned long number_of_instances = 0;
int is_sparse = 1;

int main(int argc, char **argv){
	string input_file("");
	string output_file("");

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "Produces the help message"),
		("sparcity,s", po::value<int>(&is_sparse)->default_value(1), "The sparcity of the features"),
		("input_file,I", po::value<string>(&input_file)->required(), "The input file"),
		("output_file,O", po::value<string>(&output_file)->required(), "The output file");

	libsvm_loader(const_cast<char*>(input_file.c_str()), number_of_features, number_of_instances, X, Y);
	binary_saver(const_cast<char*> (output_file.c_str()), is_sparse, X, Y);
}


