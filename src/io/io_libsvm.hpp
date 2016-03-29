#ifndef IO_LIBSVM_H
#define IO_LIBSVM_H

#include "../lasvm/vector.hpp"

using namespace std;

void libsvm_loader(char* file_name, unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& feature_vectors, map<unsigned long, int>& labels);
void libsvm_saver(char* file_name, map<unsigned long, lasvm_sparsevector_t> feature_vectors, map<unsigned long, int> labels);

#endif