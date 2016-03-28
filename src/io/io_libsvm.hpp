#ifndef IO_LIBSVM_H
#define IO_LIBSVM_H

#include "../lasvm/vector.hpp"

using namespace std;

void libsvm_loader(char* file_name, long& number_of_features, long& number_of_instances, map<long, lasvm_sparsevector_t>& feature_vectors, map<long, int>& labels);
void libsvm_saver(char* file_name, map<long, lasvm_sparsevector_t> feature_vectors, map<long, int> labels);

#endif