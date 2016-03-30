#include <cmath>
#include <ctime>

#include <vector>
#include <map>
#include <numeric>
#include <algorithm>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../lasvm/vector.hpp"
#include "../lasvm/lasvm.hpp"
#include "../io/io.hpp"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3 

#define ONLINE 0
#define ONLINE_WITH_FINISHING 1

#define RANDOM 0
#define GRADIENT 1
#define MARGIN 2

#define ITERATIONS 0
#define SVS 1
#define TIME 2

static const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};

using namespace std;

class stopwatch{
public:
    stopwatch() : start(clock()){} //start counting time
    ~stopwatch();
    double get_time(){
            return static_cast<double>(clock()-start)/CLOCKS_PER_SEC;
        }
private:
    std::clock_t start;
};
stopwatch::~stopwatch(){
    cout << "Time (in secs): "<< stopwatch::get_time() << " selected" <<endl;
}

/* Data and model */
static map<unsigned long, lasvm_sparsevector_t> X; // feature vectors
static map<unsigned long, int> Y;                   // labels
static unsigned long number_of_features = 0;
static unsigned long number_of_instances = 0;

/* Hyperparameters */
static int kernel_type = RBF;              // LINEAR, POLY, RBF or SIGMOID kernels
static double degree=3,kgamma=-1,coef0=0;// kernel params
static int use_threshold = 1;                     // use threshold via constraint \sum a_i y_i =0
static int selection_type = RANDOM;        // RANDOM, GRADIENT or MARGIN selection strategies
static int optimizer = ONLINE_WITH_FINISHING; // strategy of optimization
static double C=1;                       // C, penalty on errors
static double C_neg=1;                   // C-Weighting for negative examples
static double C_pos=1;                   // C-Weighting for positive examples
static int epochs=1;                     // epochs of online learning
static unsigned long candidates=50;				  // number of candidates for "active" selection process
static double deltamax=1000;			  // tolerance for performing reprocess step, 1000=1 reprocess only
static vector <unsigned long> select_size;      // Max number of SVs to take with selection strategy (for early stopping) 
static vector <double> x_square;         // norms of input vectors, used for RBF

/* Programm behaviour*/
static int verbosity=1;                  // verbosity level, 0=off
static int saves = 1;
static unsigned long cache_size=256;                       // 256Mb cache size as default
static double epsilon_gradient=1e-3;                       // tolerance on gradients
static unsigned long long kernel_evaluation_counter=0;                      // number of kernel evaluations
static int is_binary=0;
static map<unsigned long , int> splits;
static int termination_type=0;


/* Functions' prototypes*/

[[noreturn]]void exit_with_help();
void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name);
int libsvm_save_model(const char *model_file_name, unsigned long number_of_sv, unsigned long *svind, double threshold);
double kernel(unsigned long i, unsigned long j, void *kparam);
void finish(lasvm_t *sv, unsigned long& number_of_sv, double& threshold, vector<double>& alpha, unsigned long* svind);
void make_old(unsigned long val, vector <unsigned long>& inew, vector<unsigned long>& iold);
unsigned long select(lasvm_t *sv, vector<unsigned long>& inew, vector<unsigned long>& iold);
void train_online(char *model_file_name, vector<double>& alpha, unsigned long& number_of_sv, unsigned long *svind, vector<unsigned long>& inew, vector<unsigned long>& iold, double& threshold);
unsigned long long llrand();

unsigned long long llrand() {
	unsigned long long r = 0;

	for (int i = 0; i < 5; ++i) {
		r = (r << 15) | (rand() & 0x7FFF);
	}

	return r & 0xFFFFFFFFFFFFFFFFULL;
}

[[noreturn]]void exit_with_help(){
	cout <<
		"Usage: la_svm [options] training_set_file [model_file]" << endl <<
		"options:" << endl <<
		"-B file format : files are stored in the following format:" << endl <<
		"	0 -- libsvm ascii format (default)" << endl <<
		"	1 -- binary format" << endl <<
		"	2 -- split file format" << endl <<
		"-o optimizer: set the type of optimization (default 1)" << endl <<
		"	0 -- online " << endl <<
		"	1 -- online with finishing step " << endl <<
		"-t kernel_type : set type of kernel function (default 2)" << endl <<
		"	0 -- linear: u'*v" << endl <<
		"	1 -- polynomial: (gamma*u'*v + coef0)^degree" << endl <<
		"	2 -- radial basis function: exp(-gamma*|u-v|^2)" << endl <<
		"	3 -- sigmoid: tanh(gamma*u'*v + coef0)" << endl <<
		"-selected selection: set the type of selection strategy (default 0)" << endl <<
		"	0 -- random " << endl <<
		"	1 -- gradient-based " << endl <<
		"	2 -- margin-based " << endl <<
		"-T termination: set the type of early stopping strategy (default 0)" << endl <<
		"	0 -- number of iterations " << endl <<
		"	1 -- number of SVs " << endl <<
		"	2 -- time-based " << endl <<
		"-l sample: number of iterations/SVs/seconds to sample for early stopping (default all)" << endl <<
		" if a list of numbers is given a model file is saved for each element of the set" << endl <<
		"-C candidates : set number of candidates to search for selection strategy (default 50)" << endl <<
		"-d degree : set degree in kernel function (default 3)" << endl <<
		"-g gamma : set gamma in kernel function (default 1/k)" << endl <<
		"-r coef0 : set coef0 in kernel function (default 0)" << endl <<
		"-c cost : set the parameter C of C-SVC" << endl <<
		"-m cachesize : set cache memory size in MB (default 256)" << endl <<
		"-wi weight: set the parameter C of class i to weight*C (default 1)" << endl <<
		"-b bias: use a bias or not i.e. no constraint sum alpha_i y_i =0 (default 1=on)" << endl <<
		"-e epsilon : set tolerance of termination criterion (default 0.001)" << endl <<
		"-p epochs : number of epochs to train in online setting (default 1)" << endl <<
		"-D deltamax : set tolerance for reprocess step, 1000=1 call to reprocess >1000=no calls to reprocess (default 1000)" << endl;
    exit( EXIT_FAILURE );
}


void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name)
{
    int i= 0;
	int clss = 0; 
	double weight = 0;
    
    // parse options
    for(i=1;i<argc;i++){

        if(argv[i][0] != '-') 
			break;
        ++i;
        switch(argv[i-1][1]){

			case 'o':
				optimizer = stoi(argv[i]);
				break;
			case 't':
				kernel_type = stoi(argv[i]);
				break;
			case 's':
				selection_type = stoi(argv[i]);
				break;
			case 'l':
				while (1){
					select_size.push_back(stoul(argv[i]));
					++i;
					if ( (argv[i][0]<'0') || (argv[i][0]>'9') ) 
						break;
				}
				i--;
				break;
			case 'd':
				degree = stod(argv[i]);
				break;
			case 'g':
				kgamma = stod(argv[i]);
				break;
			case 'r':
				coef0 = stod(argv[i]);
				break;
			case 'm':
				cache_size = stoul(argv[i]);
				break;
			case 'c':
				C = stod(argv[i]);
				break;
			case 'w':
				clss = stoi(&argv[i - 1][2]);
				weight = stod(argv[i]);
				if (clss >= 1) 
					C_pos = weight; 
				else 
					C_neg = weight;
				break;
			case 'b':
				use_threshold = stoi(argv[i]);
				break;
			case 'B':
				is_binary = stoi(argv[i]);
				break;
			case 'e':
				epsilon_gradient = stod(argv[i]);
				break;
			case 'p':
				epochs = stoi(argv[i]);
				break;
			case 'D':
				deltamax = stod(argv[i]);
				break;
			case 'C':
				candidates = stoul(argv[i]);
				break;
			case 'T':
				termination_type = stoi(argv[i]);
				break;
			default:
				cerr << "Unknown option" << endl;
				exit_with_help();
        }
    }

    saves=static_cast<int>(select_size.size()); 
    if(saves==0) 
		select_size.push_back(100000000);

    // determine filenames

    if(i>=argc)
        exit_with_help();

    strncpy(input_file_name, argv[i], 1024 );

    if(i<argc-1)
        strncpy(model_file_name, argv[i+1], sizeof(model_file_name) );
    else{
        char *p = strrchr(argv[i],'/');
        if(p==NULL)
            p = argv[i];
        else
            ++p;
		snprintf(model_file_name, sizeof(model_file_name) , "%s.model", p);
    }

}


void libsvm_save_model(char *model_file_name, unsigned long number_of_sv, unsigned long *svind, double threshold){
    // saves the model in the same format as LIBSVM
	ofstream model;
	model.open(model_file_name);

	if (model.is_open()) {
		model << "Svm_type: C_svc" << endl;
		model << "Kernel_type: " << kernel_type_table[kernel_type] << endl;
		if (kernel_type == POLY)
			model << "degree = " << degree << endl;

		if (kernel_type == POLY || kernel_type == RBF || kernel_type == SIGMOID)
			model << "gamma = " << kgamma << endl;

		if (kernel_type == POLY || kernel_type == SIGMOID)
			model << "coef0 = " << coef0;

		model << "Number of classes: " << 2 << endl;
		model << "Number of support vectors: " << number_of_sv << endl;
		model << "rho = " << threshold << endl;
		model << "Labels: " << 1 << " " << -1 << endl;
		model << "SV:" << endl;
		for (unsigned long iter=0; iter < number_of_sv; iter++)
			model << Y[svind[iter]] << lasvm_sparsevector_print(X[svind[iter]]);
		model.close();
	}
	else {
		cerr << "Could not open file:" << model_file_name << endl;
		exit(EXIT_FAILURE);
	}
}

double kernel(unsigned long i, unsigned long j, void *kparam){
    double dot_product = lasvm_sparsevector_dot_product(X[i], X[j]);
    kernel_evaluation_counter++;
    
    // sparse, linear kernel
    switch(kernel_type){
		case LINEAR:
			return dot_product;
		case POLY:
			return pow(kgamma*dot_product+coef0,degree);
		case RBF:
			return exp(-kgamma*(x_square[i]+x_square[j]-2*dot_product));    
		case SIGMOID:
			return tanh(kgamma*dot_product+coef0);    
    }
    return 0;
} 
  


void finish(lasvm_t *sv, unsigned long& number_of_sv, double& threshold, vector<double>& alpha, unsigned long* svind){
	unsigned long i;

    if (optimizer == ONLINE_WITH_FINISHING){
		cout << "..[finishing]";

        unsigned long iter=0;
        do { 
            iter += lasvm_finish(sv, epsilon_gradient); 
        } while (lasvm_get_delta(sv)>epsilon_gradient);

    }

    number_of_sv= lasvm_get_l(sv);
    unsigned long svs;
	svind = new  unsigned long[number_of_sv];
    svs = lasvm_get_sv(sv,svind); 
	fill(alpha.begin(), alpha.end(), 0);

    double *svalpha = new double[number_of_sv];
    lasvm_get_alpha(sv,svalpha); 
    for(i=0;i<svs;i++) 
		alpha[svind[i]]=svalpha[i];
    threshold=lasvm_get_b(sv);
}



void make_old(unsigned long val, vector <unsigned long>& inew, vector<unsigned long>& iold){
    // move index <val> from new set into old set
	vector<unsigned long>::iterator iter = find(inew.begin(), inew.end(), val);
	if (iter != inew.end() || (iter == inew.end() && *inew.end() == val)) {
		unsigned long ind = static_cast<unsigned long>( distance(inew.begin(), iter));
		inew[ind] = inew[inew.size() - 1];
		inew.pop_back();
		iold.push_back(val);
	}
}


unsigned long select(lasvm_t *sv, vector<unsigned long>& inew, vector<unsigned long>& iold){ // selection strategy
    unsigned long selected=0;
    unsigned long t,i,r,j;
    double tmp,best;
	unsigned long ind=0;

    switch(selection_type){
		case RANDOM:   // pick a random candidate
			selected=static_cast<unsigned long>( llrand() % inew.size());
			break;

		case GRADIENT: // pick best gradient from 50 candidates
			j=candidates; 
			if(inew.size()<j) 
				j=static_cast<unsigned long>( inew.size() );
			r= static_cast<unsigned long> (llrand() % inew.size() );
			selected=r;
			best=1e20;
			for(i=0;i<j;i++){
				r=inew[selected];
				tmp=lasvm_predict(sv, r);  
				if(tmp<best) {
					best=tmp;
					ind=selected;
				}
				selected=static_cast<unsigned long>(llrand() % inew.size());
			}  
			selected=ind;
			break;

		case MARGIN:  // pick closest to margin from 50 candidates
			j=candidates;
			if(inew.size()<j) 
				j=static_cast<unsigned long>(inew.size());
			r=static_cast<unsigned long>(llrand() % inew.size());
			selected=r;
			best=1e20;
			for(i=0;i<j;i++){
				r=inew[selected];
				tmp=lasvm_predict(sv, r);  
				if (tmp<0) 
					tmp=-tmp; 
				if(tmp<best) {
					best=tmp;
					ind=selected;
				}
				selected=static_cast<unsigned long>(llrand() % inew.size());
			}  
			selected=ind;
			break;
    }
	
    t=inew[selected]; 
    inew[selected]=inew[inew.size()-1];
    inew.pop_back();
    iold.push_back(t);

    return t;
}


void train_online(char *model_file_name, vector<double>& alpha, unsigned long& number_of_sv, unsigned long *svind, vector<unsigned long>& inew, vector<unsigned long>& iold, double& threshold){
	unsigned long n_process(0), n_reprocess(0);
	unsigned long selected(0);
    double timer=0;
    stopwatch *sw; // start measuring time after loading is finished
    sw=new stopwatch;    // save timing information
	char t[1500] = {'\0'};
    strncpy(t ,model_file_name, sizeof(t) );
    strncat(t ,".time", sizeof(t) - strlen(t) - 1);
    
    lasvm_kcache_t *kcache=lasvm_kcache_create(kernel, NULL);
    lasvm_kcache_set_maximum_size(kcache, cache_size*1024*1024);
    lasvm_t *sv=lasvm_create(kcache,use_threshold,C*C_pos,C*C_neg);
	cout << "set cache size " << cache_size << endl;

    // everything is new when we start
	iota(inew.begin(), inew.begin() + number_of_instances, 0);
    
    // first add 5 examples of each class, just to balance the initial set
    int c1=0,c2=0;
    for(unsigned long i=0; i<number_of_instances; i++){
        if(Y[i]==1 && c1<5) {
			lasvm_process(sv,i,static_cast<double>(Y[i]) ); 
			c1++; 
			make_old(i, inew, iold);
		}
        if(Y[i]==-1 && c2<5){
			lasvm_process(sv,i, static_cast<double>(Y[i]) );
			c2++; 
			make_old(i, inew, iold);
		}
        if(c1==5 && c2==5) 
			break;
    }
    
    for(int j=0;j<epochs;j++){
        for(unsigned long i=0; i<number_of_instances; i++) {
            if(inew.size()==0) 
				break; // nothing more to select
            selected = select(sv,inew,iold);            // selection strategy, select new point
            
            n_process=lasvm_process(sv,selected, static_cast<double> (Y[selected]) );
            
            if (deltamax<=1000){ // potentially multiple calls to reprocess..

                n_reprocess=lasvm_reprocess(sv,epsilon_gradient);// at least one call to reprocess
                while (lasvm_get_delta(sv)>deltamax && deltamax<1000)
                    n_reprocess=lasvm_reprocess(sv,epsilon_gradient);
            }
            
            if (verbosity==2){
                number_of_sv= lasvm_get_l(sv);
				cout << "number_of_sv=" << number_of_sv << "process=" << n_process << " reprocess=" << n_reprocess << endl;
            }
            else if(verbosity==1){
                    if( (i%100)==0)
						cout << ".." << i;
					}
            
            number_of_sv= lasvm_get_l(sv);

            for(unsigned long k=0; k< static_cast<unsigned long> (select_size.size() ); k++){ 
                if   ( (termination_type==ITERATIONS && i==select_size[k]) 
                       || (termination_type==SVS && number_of_sv>=select_size[k])
                       || (termination_type==TIME && sw->get_time()>=select_size[k])
                    ) {

                    if(saves>1){ // if there is more than one model to save, give a new name
                        // save current version before potential finishing step
                        unsigned long save_l,*save_sv; 
						double *save_g, *save_alpha;
                        save_l=lasvm_get_l(sv);
                        save_alpha= new double[number_of_sv];
						lasvm_get_alpha(sv,save_alpha);				
                        save_g= new double[number_of_sv];
						lasvm_get_g(sv,save_g);
                        save_sv = new unsigned long[number_of_sv];
						lasvm_get_sv(sv,save_sv);
				
                        finish(sv, number_of_sv, threshold, alpha, svind); 
						stringstream tmp;
						tmp.clear();

                        timer+=sw->get_time();
                        if(termination_type==TIME){
							tmp << model_file_name << "_" << i << "secs";
                            cout << "..[saving model_" << i << " secs]..";
                        }
                        else{	
                            cout << "..[saving model_"<< i << " pts]..";
							tmp << model_file_name << "_" << i << "pts";
                        }
                        libsvm_save_model( const_cast<char*>(tmp.str().c_str()) , number_of_sv, svind, threshold);  
                        
                        lasvm_init(sv, save_l, save_sv, save_alpha, save_g); 
                        delete save_alpha; 
						delete save_sv;
						delete save_g;
                        delete sw;
						sw=new stopwatch;    // reset clock
                    }  
                    select_size[k]=select_size[select_size.size()-1];
                    select_size.pop_back();
                }
            }
            if(select_size.size()==0) 
				break; // early stopping, all intermediate models saved
        }

        inew.clear();
		iold.clear(); // start again for next epoch..
		iota(inew.begin(), inew.begin() + number_of_instances, 0);
    }

    if(saves<2){
        finish(sv, number_of_sv, threshold, alpha, svind); // if haven't done any intermediate saves, do final save
        timer+=sw->get_time();
    }

    if(verbosity>0) 
		cout << endl;
    cout << "nSVs=" << number_of_sv << endl;
    cout<< "||w||^2=" << lasvm_get_w2(sv) << endl;
    cout << "Kernel evaluations =" << kernel_evaluation_counter << endl;
    lasvm_destroy(sv);
    lasvm_kcache_destroy(kcache);
}


int main(int argc, char **argv){

	cout << endl << "la SVM" << endl << "______" << endl;
	
	int is_sparse = 1;
	double threshold = 0;
    
    char input_file_name[1024] = {'\0'};
    char model_file_name[1024] = {'\0'};
    parse_command_line(argc, argv, input_file_name, model_file_name);

	load_data_file(input_file_name, is_binary, number_of_features, number_of_instances, X, Y, x_square, kernel_type, kgamma, is_sparse);

	unsigned long *svind = nullptr;   // support vector indices
	vector <double> alpha;            // alpha_i, SV weights
	unsigned long number_of_sv = 0;
	static vector <unsigned long> iold, inew;		  // sets of old (already seen) points + new (unseen) points

    train_online(model_file_name, alpha, number_of_sv, svind, inew, iold, threshold);
    
    libsvm_save_model(model_file_name, number_of_sv, svind, threshold);
}

