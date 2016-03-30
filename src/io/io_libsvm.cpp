
#include <cstdlib>

#include <iostream>
#include <fstream>

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "io_libsvm.hpp"


using namespace std;


void libsvm_loader(char* file_name, unsigned long& number_of_features, unsigned long& number_of_instances,
					 map<unsigned long, lasvm_sparsevector_t>& feature_vectors, map<unsigned long, int>& labels) {

	cout << "[Loading file: " << file_name << "...";
	string buffer_line("");
	cout << buffer_line << endl;
	number_of_instances = 0;
	number_of_features = 0;

	ifstream libsvm_file;
	libsvm_file.open(file_name);
	cout << file_name << endl;

	if (libsvm_file.is_open()) {

		lasvm_sparsevector_t feature_vector;
		int label = 0;

		vector<string> features, features_;
		while (libsvm_file.peek() != EOF) {
			label = 0;
			feature_vector.clear();
			feature_vector[0] = 0;

			getline(libsvm_file, buffer_line);
			features.clear();
			features_.clear();
			boost::split(features, buffer_line, boost::is_any_of("\t \n"));
			label = stoi(features[0].c_str());
			cout << number_of_instances << "," << label << endl;
			cout << features.size() << endl;
			for(int j = 0; j<features.size(); j++)
				cout << j << "," << features[j] << "," << endl;
			cout << "----------" << endl;

			if(features.size()>1){
				for (unsigned long iter = 1; iter < features.size(); iter++) {
					features_.clear();
					if(features[iter] != ""){
						boost::split(features_, features[iter], boost::is_any_of(":") );
						for(int i = 0; i<features_.size(); i++)
							cout << i << "," << features_[i] << "," << endl;
						feature_vector[stoul(features_[0])] = stod(features_[1]);
					}
				}
			}
			
			cout << lasvm_sparsevector_print(feature_vector) << endl;
			if (number_of_features < feature_vector.rbegin()->first)
				number_of_features = feature_vector.rbegin()->first;
			feature_vectors[number_of_instances] = feature_vector;
			labels[number_of_instances] = label;
			number_of_instances++;
		}
		libsvm_file.close();
		cout << " Number of instances: " << number_of_instances << ", number of features: " << number_of_features << " ]" << endl;
	}

	else {
		cerr << "Could not load :" << file_name << "]" << endl;
		exit(EXIT_FAILURE);
	}
}

void libsvm_saver(char* file_name, map<unsigned long, lasvm_sparsevector_t> feature_vectors, map<unsigned long, int> labels, vector<double> x_square) {
	ofstream libsvm_file;
	libsvm_file.open(file_name);

	cout << "[Saving File: ..." << file_name;

	if (libsvm_file.is_open()) {
		for (unsigned long iter = 0; iter < feature_vectors.size(); iter++) {
			if(x_square[iter] != 0)
				libsvm_file << labels[iter] << lasvm_sparsevector_print(feature_vectors[iter]);
		}
		libsvm_file.close();
		cout << " File saved]" << endl;
	}

	else {
		cerr << "Could not save :" << file_name << endl;
		exit(EXIT_FAILURE);
	}
}