#include "messages.h"
#include "vector.hpp"

#include <boost/lexical_cast.hpp>




#ifndef min
# define min(a,b) (((a)<(b))?(a):(b))
#endif


/* ------------------------------------- */
/* SIMPLE VECTORS */

lasvm_vector_t lasvm_vector_create(int size){
  std::vector< double > *v = new std::vector< double >(size);
  return *v;
}

std::string lasvm_vector_print(lasvm_vector_t v){
    std::string s( "" ) ;
    for( unsigned int iter = 0; iter < v.size() ; iter++)
        s.append(" ").append(  boost::lexical_cast<std::string>( iter ) ).append(":").append(  boost::lexical_cast<std::string>( v[iter] ) ) ;   
    return s.append("\n");
}

double lasvm_vector_dot_product(lasvm_vector_t v1, lasvm_vector_t v2){
  int min_size = min( v1.size(), v2.size() );

  double dot_product = 0;

  for (int i=0; i<min_size ; i++)
    dot_product += v1[i] * v2[i];
  return dot_product;
}



/* ------------------------------------- */
/* SPARSE VECTORS */


double lasvm_sparsevector_get(lasvm_sparsevector_t v, int attribute){
  return v[ attribute ];
}

std::string lasvm_sparsevector_print( lasvm_sparsevector_t v ){
    std::string s( "" ) ;
    for( lasvm_sparsevector_t::iterator iter = v.begin(); iter != v.end() ; iter++)
        s.append(" ").append(  boost::lexical_cast<std::string>( iter->first ) ).append(":").append(  boost::lexical_cast<std::string>( iter->second ) ) ;   
    return s.append("\n");
}

lasvm_sparsevector_t lasvm_sparsevector_combine(lasvm_sparsevector_t v1, double coeff1, lasvm_sparsevector_t v2, double coeff2){
  lasvm_sparsevector_t r ;
  lasvm_sparsevector_t::iterator iterator_1 = v1.begin();
  lasvm_sparsevector_t::iterator iterator_2 = v2.begin();
  while( iterator_1 != v1.end() and iterator_2 != v2.end() ){

    if (iterator_1->first < iterator_2->first){
      r[ iterator_1->first ] = coeff1 * iterator_1->second;
      iterator_1 ++;
    }
    else if (iterator_2->first < iterator_1->first){
      r[ iterator_2->first ] = coeff2 * iterator_2->second;
      iterator_2 ++;
    }
    else{
      r[ iterator_1->first ] = coeff1 * iterator_1->second + coeff2 * iterator_2->second;
      iterator_1 ++;
      iterator_2 ++;
    }
  }

  while( iterator_1 != v1.end() ){
    r[ iterator_1->first ] = coeff1 * iterator_1->second;
    iterator_1 ++;
  }

   while( iterator_2 != v2.end() ){
    r[ iterator_2->first ] = coeff2 * iterator_2->second;
    iterator_2 ++;
  }

  return r;
}

lasvm_sparsevector_t lasvm_sparsevector_scalar_product(lasvm_sparsevector_t v , double lambda){
  lasvm_sparsevector_t r;
  return lasvm_sparsevector_combine( v , lambda , r , 0);
}


double lasvm_sparsevector_dot_product(lasvm_sparsevector_t v1, lasvm_sparsevector_t v2){
  double dot_product = 0;

  lasvm_sparsevector_t::iterator iterator_1 = v1.begin();
  lasvm_sparsevector_t::iterator iterator_2 = v2.begin();

  while( iterator_1 != v1.end() and iterator_2 != v2.end() ){

    if (iterator_1->first < iterator_2->first)
      iterator_1 ++;
    else if (iterator_2->first < iterator_1->first)
      iterator_2 ++;

    else{
      dot_product += iterator_1->second * iterator_2->second;
      iterator_1 ++;
      iterator_2 ++;
    }
  }

  return dot_product;
}


