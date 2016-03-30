
#include <cstdlib>

#include <iostream>
#include <fstream>

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "io_libsvm.hpp"


using namespace std;


void libsvm_loader(char* file_name, unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& feature_vectors, map<unsigned long, int>& labels) {

	string buffer_line;
	number_of_instances = 0;
	number_of_features = 0;

	ifstream libsvm_file;
	libsvm_file.open(file_name);

	if (libsvm_file.is_open()) {

		cout << "[Loading file: " << file_name << "...";

		lasvm_sparsevector_t feature_vector;
		int label = 0;

		vector<string> features, features_;
		while (libsvm_file.peek() != EOF) {
			label = 0;
			feature_vector.clear();

			getline(libsvm_file, buffer_line);
			features.clear();
			features_.clear();
			boost::split(features, buffer_line, boost::is_any_of("\t "));
			label = stoi(features[0].c_str());
			for (unsigned long iter = 1; iter < features.size(); iter++) {
				features_.clear();
				boost::split(features_, features[iter], boost::is_any_of(":") );
				feature_vector[stoul(features_[0])] = stod(features_[1]);
			}
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

void libsvm_saver(char* file_name, map<unsigned long, lasvm_sparsevector_t> feature_vectors, map<unsigned long, int> labels) {
	ofstream libsvm_file;
	libsvm_file.open(file_name);

	cout << "[Saving File: ..." << file_name;

	if (libsvm_file.is_open()) {
		for (unsigned long iter = 0; iter < feature_vectors.size(); iter++) {
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