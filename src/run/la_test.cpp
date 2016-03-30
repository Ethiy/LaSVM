
#include <algorithm>
#include <vector>
#include <map>

#include <cmath>
#include <cstring>
#include <cctype>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "../lasvm/vector.hpp"
#include "../io/io.hpp"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3 

static const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};

using namespace std;

static map <unsigned long, lasvm_sparsevector_t> X; // feature vectors for test set
static map <unsigned long, lasvm_sparsevector_t> Xsv;// feature vectors for SVs
static map <unsigned long, int> Y;                   // labels
static vector<double> alpha;            // alpha_i, SV weights
static int use_threshold=1;                     // use threshold via constraint \sum a_i y_i =0
static int kernel_type=RBF;              // LINEAR, POLY, RBF or SIGMOID kernels
static double degree=3,kgamma=-1,coef0=0;// kernel params
static vector <double> x_square;         // norms of input vectors, used for RBF
static vector <double> xsv_square;        // norms of test vectors, used for RBF
static map<unsigned long, int> splits;
static int is_binary = 0;


void exit_with_help();
void libsvm_load_model(char* model_file_name, unsigned long& number_of_sv, unsigned long& number_of_features, double& threshold, double& degree,
	double& kgamma, double& coef0, map<unsigned long, lasvm_sparsevector_t> Xsv, vector<double> xsv_square, vector<double> alpha);
double kernel(int i, int j, void *kparam);
void test(char *output_name, unsigned long number_of_instances, unsigned long number_of_sv, vector<double> alpha, map<unsigned long, int> Y, double threshold);
void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name, char *output_file_name);

void exit_with_help(){
    cout << endl <<
	    "Usage: la_test [options] test_set_file model_file output_file" << endl <<
	    "options:" << endl <<
            "-B file format : files are stored in the following format:" << endl <<
            "	0 -- libsvm ascii format (default)" << endl <<
            "	1 -- binary format" << endl <<
            "	2 -- split file format" << endl; 

    exit(EXIT_FAILURE);
}


void libsvm_load_model(char* model_file_name, unsigned long& number_of_sv, unsigned long& number_of_features, double& threshold, double& degree,
							double& kgamma, double& coef0, map<unsigned long, lasvm_sparsevector_t> Xsv, vector<double> xsv_square, vector<double> alpha) {

	  cout << "[Loading file: " << model_file_name << "...";

	  ifstream model;
	  model.open(model_file_name);
	  number_of_sv = 0;
	  number_of_features = 0;

	  if (model.is_open()) {
		  string buffer("");
		  vector<string> strings, strings_;
		  strings.clear();
		  strings_.clear();
		  while (*strings.begin() != "SV:") {
			  getline(model,buffer);
			  boost::split(strings, buffer, boost::is_any_of("\t "));
			  if (strings[1] == "degree" || strings[1] == "degree=")
				  degree = stod(*strings.end());
			  else if (strings[1] == "gamma" || strings[1] == "gamma=")
				  kgamma = stod(*strings.end());
			  else if (strings[1] == "coef0" || strings[1] == "coef0=")
				  coef0 = stod(*strings.end());
			  else if (strings[1] == "Number of support vectors :" )
				  number_of_sv = stoul(*strings.end());
			  else if (strings[1] == "rho" || strings[1] == "rho=")
				  threshold = stod(*strings.end());
		  }
		  strings.clear();
		  strings_.clear();

		  lasvm_sparsevector_t feature_vector;
		  int label = 0;
		  unsigned long counter = 0;

		  vector<string> features, features_;
		  while (model.peek() != EOF) {
			  label = 0;
			  feature_vector.clear();

			  getline(model, buffer);
			  features.clear();
			  features_.clear();
			  boost::split(features, buffer, boost::is_any_of("\t "));
			  label = stoi(features[0].c_str());
			  for (unsigned long iter = 1; iter < features.size(); iter++) {
				  features_.clear();
				  boost::split(features_, features[iter], boost::is_any_of(":"));
				  feature_vector[stoul(features_[0])] = stod(features_[1]);
			  }
			  if (number_of_features < feature_vector.rbegin()->first)
				  number_of_features = feature_vector.rbegin()->first;
			  Xsv[counter] = feature_vector;
			  alpha[counter] = label;
			  counter++;
		  }

		  number_of_sv = min<unsigned long>(number_of_sv, counter);
		  cout << " Number of support vectors: " << number_of_sv << ", number of features: " << number_of_features << " ]" << endl;
		  model.close();
	  }
	  else {
		  cerr << "Could not load file:" << model_file_name << endl;
		  exit(EXIT_FAILURE);
	  }
  }



double kernel(int i, int j, void *kparam){
    double dot;
    dot=lasvm_sparsevector_dot_product(X[i],Xsv[j]);
    
    // sparse, linear kernel
    switch(kernel_type){
    case LINEAR:
        return dot; 
    case POLY:
	return pow(kgamma*dot+coef0,degree);
    case RBF:
	return exp(-kgamma*(x_square[i]+xsv_square[j]-2*dot));    
    case SIGMOID:
	return tanh(kgamma*dot+coef0);    
    }
    return 0;
}  
  

void test(char *output_name, unsigned long number_of_instances, unsigned long number_of_sv, vector<double> alpha, map<unsigned long, int> Y, double threshold){	
    ofstream output_file ( output_name );
    double label_pred = -threshold;
    double accuracy=0;
	double false_positive = 0;
	double false_negative = 0;

    if( output_file.is_open() ){

        for(unsigned long i = 0; i < number_of_instances ; i++){
            for(unsigned long j=0 ; j < number_of_sv ; j++)
                label_pred+=alpha[j]*kernel(i,j,NULL);

            if(label_pred >= 0) 
                label_pred = 1;
            else 
                label_pred = -1;
			output_file << label_pred << endl;

			if (static_cast<int>(label_pred) == Y[i])
				accuracy++;
			else if (static_cast<int>(label_pred) == 1)
				false_positive++;
			else
				false_negative++;
        }

        output_file.close();
		cout << "Accuracy = " << accuracy << "/" << number_of_instances << "=" << accuracy / number_of_instances * 100 << endl <<
			"False postive rate = " << false_positive << "/" << number_of_instances << "=" << false_positive / number_of_instances * 100 << endl <<
			"False postive rate = " << false_negative << "/" << number_of_instances << "=" << false_negative / number_of_instances * 100 << endl;
    }
	else {
		cerr << "Could not open :" << output_name << endl;
		exit( EXIT_FAILURE );
	}
}


void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name, char *output_file_name)
{
    int i; 
    
    // parse options
    for(i=1;i<argc;i++){
		if(argv[i][0] != '-') 
			break;
		++i;
		switch(argv[i-1][1]){
			case 'B':
				is_binary=stoi(argv[i]);
				break;
			default:
				cerr << "Unknown option" << endl;
				exit_with_help();
	}
    }

    // determine filenames

    if(i>=argc)
		exit_with_help();

	strncpy(input_file_name, argv[i], sizeof(input_file_name));

    if(i<argc-1)
		strncpy(model_file_name, argv[i + 1], sizeof(model_file_name));
    else{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		snprintf(model_file_name, sizeof(model_file_name), "%s.model", p);
    }

    if(argc<i+3) 
		exit_with_help();

    strncpy(input_file_name, argv[i], sizeof(input_file_name));
    strncpy(model_file_name, argv[i+1], sizeof(model_file_name));
    strncpy(output_file_name, argv[i+2], sizeof(output_file_name));

}



int main(int argc, char **argv)  
{
	cout << endl << "la test" << endl << "_______" << endl;
    
	char input_file_name[1024] = {'\0'};
    char model_file_name[1024] = {'\0'};
    char output_file_name[1024] = {'\0'};
    parse_command_line(argc, argv, input_file_name, model_file_name, output_file_name);

	double threshold;
	unsigned long number_of_sv(0), number_of_features(0), number_of_instances(0);
	int is_sparse = 1;
     
	libsvm_load_model( model_file_name, number_of_sv, number_of_features, threshold, degree, kgamma, coef0, Xsv, xsv_square, alpha);
	load_data_file(input_file_name, is_binary, number_of_features, number_of_instances, X, Y, x_square, kernel_type, kgamma, is_sparse);
    
	test(output_file_name, number_of_instances, number_of_sv, alpha, Y, threshold);
}


