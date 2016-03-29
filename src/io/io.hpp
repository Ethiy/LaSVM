#ifndef IO_H
#define IO_H

#include <map>
#include <vector>

#include "../lasvm/vector.hpp"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3 

using namespace std;

void load_data_file(char *file_name, int& is_binary, unsigned long& number_of_features, unsigned long& number_of_instances, map<unsigned long, lasvm_sparsevector_t>& X, map<unsigned long, int>& Y, vector<double>& x_square, int kernel_type, double& kgamma);

#endif
