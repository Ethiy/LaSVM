// -*- mode: c++; c-file-style: "stroustrup"; -*-


using namespace std;
#include <stdio.h>
#include <vector>
#include <cmath>
#include <cstring>
#include <cctype>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "../lasvm/vector.hpp"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3 

const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};

int m,msv;                         // training and test set sizes
map <unsigned long, lasvm_sparsevector_t> X; // feature vectors for test set
map <unsigned long, lasvm_sparsevector_t> Xsv;// feature vectors for SVs
map <unsigned long, int> Y;                   // labels
map <unsigned long, double> alpha;            // alpha_i, SV weights
double threshold;                        // threshold
int use_threshold=1;                     // use threshold via constraint \sum a_i y_i =0
int kernel_type=RBF;              // LINEAR, POLY, RBF or SIGMOID kernels
double degree=3,kgamma=-1,coef0=0;// kernel params
vector <double> x_square;         // norms of input vectors, used for RBF
vector <double> xsv_square;        // norms of test vectors, used for RBF
char split_file_name[1024]="\0";         // filename for the splits
int is_binary=0;
map<unsigned long, int> splits;             
int max_index=0;




void exit_with_help()
{
    cout <<
	    "\nUsage: la_test [options] test_set_file model_file output_file\n"
	    "options:\n"
            "-B file format : files are stored in the following format:\n"
            "	0 -- libsvm ascii format (default)\n"
            "	1 -- binary format\n"
            "	2 -- split file format" << endl; 

    exit(EXIT_FAILURE*);
}


  void libsvm_load_sv_data(FILE *fp)
// loads the same format as LIBSVM
{ 
    int max_index; int oldindex=0;
    int index; double value; int i;
    lasvm_sparsevector_t* v;
    
    alpha.resize(msv);
    for(i=0;i<msv;i++){
	v=lasvm_sparsevector_create();
	Xsv.push_back(v); 
    }
    
    max_index = 0;
    for(i=0;i<msv;i++)
    {
	double label;
	fscanf(fp,"%lf",&label);
	//printf("%d:%g\n",i,label);
	alpha[i] = label;
	while(1)
	{
	    int c;
	    do {
		c = getc(fp);
		if(c=='\n') goto out2;
	    } while(isspace(c));
	    ungetc(c,fp);
	    fscanf(fp,"%d:%lf",&index,&value);
	    if(index!=oldindex)
	    {
		lasvm_sparsevector_set(Xsv[i],index,value);
	    }
	    oldindex=index;
	    if (index>max_index) max_index=index;
	}	
    out2:
	label=1; // dummy
    }
    
    printf("loading model: %d svs\n",msv);
    
    if(kernel_type==RBF)
    {
    	xsv_square.resize(msv);
    	for(i=0;i<msv;i++)
    	    xsv_square[i]=lasvm_sparsevector_dot_product(Xsv[i],Xsv[i]);
    }
    
}




int libsvm_load_model(const char *model_file_name)
// saves the model in the same format as LIBSVM
{
    int i;

    FILE *fp = fopen(model_file_name,"r");
	

    if(fp == NULL)
    {
	fprintf(stderr,"Can't open input file \"%s\"\n",model_file_name);
	exit(1);
    }

    static char tmp[1001];

    fscanf(fp,"%1000s",tmp); //svm_type
    fscanf(fp,"%1000s",tmp); //c_svc
    fscanf(fp,"%1000s",tmp); //kernel_type
    fscanf(fp,"%1000s",tmp); //rbf,poly,..

    kernel_type=LINEAR;
    for(i=0;i<4;i++)
	if (strcmp(tmp,kernel_type_table[i])==0) kernel_type=i;

    if(kernel_type == POLY)
    {
	fscanf(fp,"%1000s",tmp); 
	fscanf(fp,"%lf", &degree);
    }
    if(kernel_type == POLY || kernel_type == RBF || kernel_type == SIGMOID)
    {
	fscanf(fp,"%1000s",tmp); 
	fscanf(fp,"%lf",&kgamma);
    }
    if(kernel_type == POLY || kernel_type == SIGMOID)
    {
	fscanf(fp,"%1000s",tmp); 
	fscanf(fp,"%lf", &coef0);
    }

    fscanf(fp,"%1000s",tmp); // nr_class
    fscanf(fp,"%1000s",tmp); // 2
    fscanf(fp,"%1000s",tmp); // total_sv
    fscanf(fp,"%d",&msv); 

    fscanf(fp,"%1000s",tmp); //rho
    fscanf(fp,"%lf\n",&threshold);

    fscanf(fp,"%1000s",tmp); // label
    fscanf(fp,"%1000s",tmp); // 1
    fscanf(fp,"%1000s",tmp); // -1
    fscanf(fp,"%1000s",tmp); // nr_sv
    fscanf(fp,"%1000s",tmp); // num
    fscanf(fp,"%1000s",tmp); // num
    fscanf(fp,"%1000s",tmp); // SV
	
    // now load SV data...
    
    libsvm_load_sv_data(fp);
	
    // finished!

    fclose(fp);
    return 0;
}

double kernel(int i, int j, void *kparam){
    double dot;
    dot=lasvm_sparsevector_dot_product(X[i],Xsv[j]);
    
    // sparse, linear kernel
    switch(kernel_type)
    {
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
  

void test(char *output_name)
{	
    ofstream output_file ( output_name );
    int i,j;
    double y;
    double acc=0;

    if( output_file.is_open() ){

        for(i=0;i<m;i++){
            y=-threshold;
            for(j=0;j<msv;j++)
                y+=alpha[j]*kernel(i,j,NULL);

            if(y>=0) 
                y=1;
            else 
                y=-1;
            if( ((int)y)==Y[i] ) 
                acc++;

            output_file << y << endl;
        }

        output_file.close();
    }

    printf("accuracy= %g (%d/%d)\n",(acc/m)*100,((int)acc),m);
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

    strcpy(input_file_name, argv[i]);

    if(i<argc-1)
		strcpy(model_file_name,argv[i+1]);
    else{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		sprintf(model_file_name,"%s.model",p);
    }

    if(argc<i+3) exit_with_help();

    strcpy(input_file_name, argv[i]);
    strcpy(model_file_name, argv[i+1]);
    strcpy(output_file_name, argv[i+2]);

}



int main(int argc, char **argv)  
{

    printf("\n");
    printf("la test\n");
    printf("_______\n");
    
    char input_file_name[1024];
    char model_file_name[1024];
    char output_file_name[1024];
    parse_command_line(argc, argv, input_file_name, model_file_name, output_file_name);
     
    libsvm_load_model(model_file_name);// load model
    load_data_file(input_file_name); // load test data
    
    test(output_file_name);
}


