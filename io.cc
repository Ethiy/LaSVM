
#include <cstdlib>

#include <iostream>
#include <fstream>

#include <vector>

#include <boost/algorithm/string.hpp>

#include "io.hpp"


using namespace std;


map<long,int> split_file_load( char* split_file_name , int& is_binary_file ){
    map<long , int> splits;
    int instance_index_in = 0;
    int labels_in = 0;

    string buffer_line;
    unsigned long line_num = 0;

    ifstream split_file;
    split_file.open( split_file_name);
    string name (split_file_name) ;
    
    if( split_file.is_open() ){

        vector<string> strings,strings_ ;
        getline( split_file , buffer_line );
        strings.clear();
        boost::split(strings , buffer_line , boost::is_any_of("\t "));
        boost::split( strings_ , name , boost::is_any_of("/\\"));
        strings_.pop_back();
        strings_.push_back( strings.back() );
        name = boost::algorithm::join(strings_, "/");
        split_file_name = const_cast<char*>( name.c_str() );
        line_num ++;

        getline( split_file , buffer_line );
        strings.clear();
        boost::split(strings , buffer_line , boost::is_any_of("\t "));
        is_binary_file = atoi( (strings.back()).c_str() );
        line_num ++;

        getline( split_file , buffer_line );
        strings.clear();
        boost::split(strings , buffer_line , boost::is_any_of("\t "));
        instance_index_in =  atoi( (strings.back()).c_str() );
        line_num ++;

        getline( split_file , buffer_line );
        strings.clear();
        boost::split(strings , buffer_line , boost::is_any_of("\t "));
        labels_in =  atoi( (strings.back()).c_str() );
        line_num ++;

        cout << "[split file: binary: " << is_binary_file << ", new_indices: " << instance_index_in << ", new_labels:" << labels_in << "]" << endl;

        if( !instance_index_in ) 
            return splits;
        int label;
        long index;
        while( split_file.peek() != EOF ){
            cout << "line_number : " << line_num - 3 << endl;
            label =0;
            index = 0;
            getline( split_file , buffer_line );
            strings.clear();
            boost::split(strings , buffer_line , boost::is_any_of("\t "));
            index =  stol( strings[0].c_str() );
            if( labels_in )
                label = stoi( strings[1].c_str() );
            splits[ index-1 ] = label ; // C++ starts from 0
            line_num ++;
        }
        split_file.close();
    }

    else{
        cerr << "Could not load split file:" << split_file_name << endl; 
        exit( EXIT_FAILURE );
    }
	return splits;
}