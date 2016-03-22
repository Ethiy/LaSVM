#ifndef VECTOR_H
#define VECTOR_H


#include <map>
#include <vector>

#include <string>



/* ------------------------------------- */
/* SIMPLE VECTORS */


typedef std::vector< double > lasvm_vector_t;

lasvm_vector_t lasvm_vector_create(int size);

std::string lasvm_vector_print(lasvm_vector_t v);

double lasvm_vector_dot_product(lasvm_vector_t v1, lasvm_vector_t v2);


/* ------------------------------------- */
/* SPARSE VECTORS */


typedef std::map< int , double > lasvm_sparsevector_t;

double lasvm_sparsevector_get(lasvm_sparsevector_t v, int attribute);

std::string lasvm_sparsevector_print( lasvm_sparsevector_t v );

lasvm_sparsevector_t lasvm_sparsevector_combine(lasvm_sparsevector_t v1, double coeff1, lasvm_sparsevector_t v2, double coeff2);

double lasvm_sparsevector_dot_product(lasvm_sparsevector_t v1, lasvm_sparsevector_t v2);

