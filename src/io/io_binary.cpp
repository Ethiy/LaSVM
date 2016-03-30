#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include <cmath>


#include "../lasvm/vector.hpp"


using namespace std;

void binary_loader(char* file_name, int& is_sparse , unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& feature_vectors, map<unsigned long, int>& labels) {
	number_of_instances = 0;
	number_of_features = 0;
	is_sparse = 1;

	ifstream binary_file;
	binary_file.open(file_name, ios::in | ios::binary);

	if (binary_file.is_open()) {
		cout << "[Loading file: " << file_name << "...";

		unsigned long size[2];
		lasvm_sparsevector_t feature_vector;
		int label = 0;
		vector <double> buffer_vector;
		vector <unsigned long> indexes;
		unsigned long attribute = 0;
		unsigned long size_of_buffer = 0;

		binary_file.read( reinterpret_cast<char*>(size), 2 * sizeof(unsigned long));
		number_of_instances = size[0];
		number_of_features = size[1];

		if (number_of_features > 0)
			is_sparse = 0;
		
		for (unsigned long index = 0; index < number_of_instances; index++) {

			binary_file.read(reinterpret_cast<char*>(&label), sizeof(int));
			labels[index] = label;
			buffer_vector.clear();
			feature_vector.clear();

			if (is_sparse) {
				binary_file.read(reinterpret_cast<char*>(&size_of_buffer), sizeof(unsigned long));
				indexes.clear();
				binary_file.read(reinterpret_cast<char*>(&indexes[0]), size_of_buffer*sizeof(unsigned long));
				binary_file.read(reinterpret_cast<char*>(&buffer_vector[0]), size_of_buffer*sizeof(unsigned long));
				for (attribute = 0; attribute < size_of_buffer; attribute++) {
					if( fabs(buffer_vector[attribute]) > numeric_limits<double>::epsilon() )
						feature_vector[indexes[attribute]] = buffer_vector[attribute];
					if (indexes[attribute] > number_of_features)
						number_of_features = indexes[attribute];
				}
					
			}

			else {
				binary_file.read(reinterpret_cast<char*>(&buffer_vector[0]), number_of_features*sizeof(double) );
				for (attribute = 0; attribute < number_of_features; attribute++)
					feature_vector[attribute] = buffer_vector[attribute];
				feature_vectors[index] = feature_vector;
			}
		}
		binary_file.close();
		cout << " Number of instances: " << number_of_instances << ", number of features: " << number_of_features << " ]" << endl;
	}

	else {
		cerr << "Could not load :" << file_name << "]" << endl;
		exit(EXIT_FAILURE);
	}
}

void binary_saver(char* file_name, int is_sparse, map<unsigned long, lasvm_sparsevector_t> feature_vectors, map<unsigned long, int> labels) {
	ofstream binary_file;
	binary_file.open(file_name, ios::out | ios::binary);

	cout << "[Saving File: ..." << file_name;

	if (binary_file.is_open()) {
		lasvm_sparsevector_t feature_vector;
		int label = 0;
		vector <double> buffer_vector;
		vector <unsigned long> indexes;
        unsigned long size_of_buffer = 0;
		unsigned long number_of_instances = static_cast<unsigned long>(labels.size());
		unsigned long number_of_features = 0;

		if (! is_sparse) 
			number_of_features = static_cast<unsigned long>(feature_vectors[1].size());

		binary_file.write(reinterpret_cast<char*>(&number_of_instances), sizeof(unsigned long));
		binary_file.write(reinterpret_cast<char*>(&number_of_features), sizeof(unsigned long));

		for (unsigned long index = 0; index < number_of_instances; index++) {

			labels[index] = label;
			binary_file.write(reinterpret_cast<char*>(&label), sizeof(int));
			feature_vector.clear();
			buffer_vector.clear();
			size_of_buffer = static_cast<unsigned long>(feature_vectors[index].size());
			feature_vector = feature_vectors[index];

			if (is_sparse) {
				binary_file.write(reinterpret_cast<char*>(&size_of_buffer), sizeof(unsigned long));
				indexes.clear();
				for (lasvm_sparsevector_t::iterator iter = feature_vector.begin(); iter != feature_vector.end(); iter++) {
					indexes.push_back(iter->first);
					buffer_vector.push_back(iter->second);
				}
				binary_file.write(reinterpret_cast<char*>(&indexes[0]), size_of_buffer*sizeof(unsigned long));
				binary_file.write(reinterpret_cast<char*>(&buffer_vector[0]), size_of_buffer*sizeof(unsigned long));

			}

			else {
				for (lasvm_sparsevector_t::iterator iter = feature_vector.begin(); iter != feature_vector.end(); iter++)
					buffer_vector.push_back(iter->second);
				binary_file.write(reinterpret_cast<char*>(&buffer_vector[0]), number_of_features*sizeof(double));
			}
		}

		binary_file.close();
		cout << " File saved]" << endl;
	}

	else {
		cerr << "Could not save :" << file_name << endl;
		exit(EXIT_FAILURE);
	}
}
