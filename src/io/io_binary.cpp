#include <iostream>
#include <fstream>

#include <map>
#include <vector>


#include "../lasvm/vector.hpp"


using namespace std;

void binary_loader(char* file_name, long& number_of_features, long& number_of_instances, map<long, lasvm_sparsevector_t>& feature_vectors, map<long, int>& labels) {
	number_of_instances = 0;
	number_of_features = 0;

	ifstream binary_file;
	binary_file.open(file_name, ios::in | ios::binary);

	if (binary_file.is_open()) {

	}

	else {
		cerr << "Could not load :" << file_name << "]" << endl;
		exit( EXIT_FAILURE )
	}
}

void binary_saver(char* file_name, map<long, lasvm_sparsevector_t> feature_vectors, map<long, int> labels) {
	ofstream binary_file;
	binary_file.open(file_name, ios::out | ios::binary);

	cout << "[Saving File: ..." << file_name;

	if (binary_file.is_open()) {
		
		binary_file.close();
		cout << " File saved]" << endl;
	}

	else {
		cerr << "Could not save :" << file_name << endl;
		exit(EXIT_FAILURE);
	}
}