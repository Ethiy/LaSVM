#ifndef IO_BINARY_H
#define IO_BINARY_H

#include "../lasvm/vector.hpp"

using namespace std;

void binary_loader(char* file_name, int& is_sparse, unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& feature_vectors, map<unsigned long, int>& labels);
void binary_saver(char* file_name, int is_sparse, map<unsigned long, lasvm_sparsevector_t> feature_vectors, map<unsigned long, int> labels);

#endif