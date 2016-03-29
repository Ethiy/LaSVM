#include <iostream>
#include <fstream>

#include <utility>
#include <algorithm>

#include "io.hpp"
#include "io_split.hpp"
#include "io_libsvm.hpp"
#include "io_binary.hpp"

using namespace std;

void load_data_file(char *file_name, int& is_binary, unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& X, map<unsigned long, int>& Y, vector<double>& x_square, int kernel_type, double& kgamma){
	map<unsigned long, int> splits;
	splits.clear();
	int is_sparse = 1;
	x_square.clear();

	if (is_binary == 0){ // if ascii, check if it isn't a split file..
		ifstream file;
		file.open(file_name);
		string buffer;
		if (file.is_open()) {
			getline(file, buffer);
			char c = buffer.at(0);
			if (c == 'f') 
				is_binary = 2;
			file.close();
		}
		else{
			cerr << "Can't open input file" << file_name << endl;
			exit( EXIT_FAILURE );
		}
	}

	switch (is_binary){  // load diferent file formats
		case 0: // libsvm format
			libsvm_loader( file_name,  number_of_features,  number_of_instances,  X, Y);
			break;
		case 1:
			binary_loader( file_name, is_sparse, number_of_features, number_of_instances,  X, Y);
			break;
		case 2:
			int instance_index_in, labels_in;
			splits = split_file_load(file_name, is_binary, instance_index_in, labels_in);
			if (is_binary == 0){
				libsvm_loader(file_name, number_of_features, number_of_instances, X, Y);
				break;
			}
			else{
				binary_loader(file_name, is_sparse, number_of_features, number_of_instances, X, Y);
				break;
			}
		default:
			cerr << "Illegal file type '-B" << is_binary << endl;
			exit( EXIT_FAILURE );
	}

	if (kernel_type == RBF)
		transform(X.begin(), X.end(), x_square.begin(), [](const pair<unsigned long, lasvm_sparsevector_t>& instance) {return lasvm_sparsevector_square(instance.second); } );

	if (kgamma == -1)
		kgamma = 1.0 / ( static_cast<double>(number_of_features) ); // same default as LIBSVM
}