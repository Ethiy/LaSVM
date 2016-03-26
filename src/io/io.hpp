#ifndef IO_H
#define IO_H

#include <map>

class ID{ // class to hold split file indices and labels
public:
    unsigned long x;
    int y;
    ID() : x(0), y(0) {}
    ID(unsigned long x1,int y1) : x(x1), y(y1) {}
};

// IDs will be sorted by index, not by label.
bool operator<(const ID& x, const ID& y){
    return x.x < y.x;
}

using namespace std;

map<long , int> split_file_load( char* file_name , int& is_binary_file , int& instance_index_in , int& labels_in );

#endif
