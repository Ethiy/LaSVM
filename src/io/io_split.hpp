#ifndef IO_SPLIT_H
#define IO_SPLIT_H

#include <map>

using namespace std;

map<unsigned long , int> split_file_load( char* file_name , int& is_binary_file , int& instance_index_in , int& labels_in );

#endif
