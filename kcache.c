/***********************************************************************
 * 
 *  LUSH Lisp Universal Shell
 *    Copyright (C) 2002 Leon Bottou, Yann Le Cun, AT&T Corp, NECI.
 *  Includes parts of TL3:
 *    Copyright (C) 1987-1999 Leon Bottou and Neuristique.
 *  Includes selected parts of SN3.2:
 *    Copyright (C) 1991-2001 AT&T Corp.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA
 * 
 ***********************************************************************/

/***********************************************************************
 * $Id: kcache.c,v 1.4 2005/11/16 00:10:01 agbs Exp $
 **********************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "messages.h"
#include "kcache.h"

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
#endif

struct lasvm_kcache_s {
  lasvm_kernel_t kernel_function;
  void *closure;
  long max_size;
  long current_size;
  long length;
  long *i2r_swap;
  long *r2i_swap;
  /* Rows */
  long    *row_size;
  double  *row_diag_position;
  double **row_data;
  long    *row_next;
  long    *row_previous;
  long    *qnext;
  long    *qprev;
};

static void * xmalloc(long n){
  void *ptr = malloc(n);
  if (! ptr) 
    lasvm_error("Function malloc() has returned zero\n");

  return ptr;
}

static void * xrealloc(void *ptr, long n){
  if (! ptr)
    ptr = malloc(n);
  else
    ptr = realloc(ptr, n);
  if (! ptr) 
    lasvm_error("Function realloc has returned zero\n");

  return ptr;
}

static void xminsize(lasvm_kcache_t *self, long n)
{
  long ol = self->length;
  if (n > ol)
    {
      long i;
      long nl = max(256,ol);
      while (nl < n)
	nl = nl + nl;
      self->i2r_swap = (long*)xrealloc(self->i2r_swap, nl*sizeof(long));
      self->r2i_swap = (long*)xrealloc(self->r2i_swap, nl*sizeof(long));
      self->row_size = (long*)xrealloc(self->row_size, nl*sizeof(long));
      self->qnext = (long*)xrealloc(self->qnext, (1+nl)*sizeof(long));
      self->qprev = (long*)xrealloc(self->qprev, (1+nl)*sizeof(long));
      self->row_diag_position = (double*)xrealloc(self->row_diag_position, nl*sizeof(double));
      self->row_data = (double**)xrealloc(self->row_data, nl*sizeof(double*));
      self->row_next = self->qnext + 1;
      self->row_previous = self->qprev + 1;
      for (i=ol; i<nl; i++)
	{
	  self->i2r_swap[i] = i;
	  self->r2i_swap[i] = i;
	  self->row_size[i] = -1;
	  self->row_next[i] = i;
	  self->row_previous[i] = i;
	  self->row_data[i] = 0;
	}
      self->length = nl;
    }
}

lasvm_kcache_t* lasvm_kcache_create(lasvm_kernel_t kernelfunc, void *closure){
  lasvm_kcache_t *self;
  self = (lasvm_kcache_t*)xmalloc(sizeof(lasvm_kcache_t));
  memset(self, 0, sizeof(lasvm_kcache_t));
  self->length = 0;
  self->kernel_function = kernelfunc;
  self->closure = closure;
  self->current_size = sizeof(lasvm_kcache_t);
  self->max_size = 256*1024*1024;
  self->qprev = (long*)xmalloc(sizeof(long));
  self->qnext = (long*)xmalloc(sizeof(long));
  self->row_next = self->qnext + 1;
  self->row_previous = self->qprev + 1;
  self->row_previous[-1] = -1;
  self->row_next[-1] = -1;
  return self;
}

void lasvm_kcache_destroy(lasvm_kcache_t *self){
  if (self){
      long i;
      if (self->i2r_swap)
        free(self->i2r_swap);
      if (self->r2i_swap)
        free(self->r2i_swap);
      if (self->row_data){
        for (i=0; i<self->length; i++){
          if (self->row_data[i])
            free(self->row_data[i]);
          if (self->row_data)
            free(self->row_data);
          if (self->row_size)
            free(self->row_size);
          if (self->row_diag_position)
            free(self->row_diag_position);
          if (self->qnext)
            free(self->qnext);
          if (self->qprev)
            free(self->qprev);
          memset(self, 0, sizeof(lasvm_kcache_t));
          free(self);
        }
      }
  }
}

long * lasvm_kcache_i2r(lasvm_kcache_t *self, long n){
  xminsize(self, n);
  return self->i2r_swap;
}

long * lasvm_kcache_r2i(lasvm_kcache_t *self, long n){
  xminsize(self, n);
  return self->r2i_swap;
}

static void xextend(lasvm_kcache_t *self, long k, long nlen){
  long olen = self->row_size[k];
  if (nlen > olen)
    {
      double *ndata = (double*)xmalloc(nlen*sizeof(double));
      if (olen > 0)
	{
	  double *odata = self->row_data[k];
	  memcpy(ndata, odata, olen * sizeof(double));
	  free(odata);
	}
      self->row_data[k] = ndata;
      self->row_size[k] = nlen;
      self->current_size += (long)(nlen - olen) * sizeof(double);
    }
}

static void xtruncate(lasvm_kcache_t *self, long k, long nlen)
{
  long olen = self->row_size[k];
  if (nlen < olen)
    {
      double *ndata;
      double *odata = self->row_data[k];
      if (nlen >  0)
	{
	  ndata = (double*)xmalloc(nlen*sizeof(double));
	  memcpy(ndata, odata, nlen * sizeof(double));
	}
      else
	{
	  ndata = 0;
	  self->row_next[self->row_previous[k]] = self->row_next[k];
	  self->row_previous[self->row_next[k]] = self->row_previous[k];
	  self->row_next[k] = self->row_previous[k] = k;
	}
      free(odata);
      self->row_data[k] = ndata;
      self->row_size[k] = nlen;
      self->current_size += (long)(nlen - olen) * sizeof(double);
    }
}

static void xswap(lasvm_kcache_t *self, long i1, long i2, long r1, long r2){
  long k = self->row_next[-1];
  while (k >= 0)
    {
      long nk = self->row_next[k];
      long n  = self->row_size[k];
      long rr = self->i2r_swap[k];
      double *d = self->row_data[k];
      if (r1 < n)
	{
	  if (r2 < n)
	    {
	      double t1 = d[r1];
	      double t2 = d[r2];
	      d[r1] = t2;
	      d[r2] = t1;
	    }
	  else if (rr == r2)
            {
              d[r1] = self->row_diag_position[k];
            }
          else
            {
	      long arsize = self->row_size[i2];
              if (rr < arsize && rr != r1)
                d[r1] = self->row_data[i2][rr];
              else
                xtruncate(self, k, r1);
            }
	}
      else if (r2 < n)
        {
          if (rr == r1)
            {
              d[r2] = self->row_diag_position[k];
            }
          else 
            {
	      long arsize = self->row_size[i1];
              if (rr < arsize && rr != r2)
                d[r2] = self->row_data[i1][rr];
              else
                xtruncate(self, k, r2);
            }
        }
      k = nk;
    }
  self->r2i_swap[r1] = i2;
  self->r2i_swap[r2] = i1;
  self->i2r_swap[i1] = r2;
  self->i2r_swap[i2] = r1;
}

void lasvm_kcache_swap_rr(lasvm_kcache_t *self, long r1, long r2){
  xminsize(self, 1+max(r1,r2));
  xswap(self, self->r2i_swap[r1], self->r2i_swap[r2], r1, r2);
}

void lasvm_kcache_swap_ii(lasvm_kcache_t *self, long i1, long i2){
  xminsize(self, 1+max(i1,i2));
  xswap(self, i1, i2, self->i2r_swap[i1], self->i2r_swap[i2]);
}

void lasvm_kcache_swap_ri(lasvm_kcache_t *self, long r1, long i2){
  xminsize(self, 1+max(r1,i2));
  xswap(self, self->r2i_swap[r1], i2, r1, self->i2r_swap[i2]);
}

double lasvm_kcache_query(lasvm_kcache_t *self, long i, long j){
  long length = self->length;
  ASSERT(i>=0);
  ASSERT(j>=0);
  if (i<length && j<length)
    {
      /* check cache */
      long s = self->row_size[i];
      long p = self->i2r_swap[j];
      if (p < s)
	return self->row_data[i][p];
      else if (i == j && s >= 0)
	return self->row_diag_position[i];
      p = self->i2r_swap[i];
      s = self->row_size[j];
      if (p < s)
	return self->row_data[j][p];
    }
  /* compute */
  return (*self->kernel_function)(i, j, self->closure);
}

static void xpurge(lasvm_kcache_t *self){
  if (self->current_size>self->max_size)
    {
      long k = self->row_previous[-1];
      while (self->current_size>self->max_size && k!=self->row_next[-1])
	{
	  long pk = self->row_previous[k];
          xtruncate(self, k, 0);
	  k = pk;
	}
    }
}

double * lasvm_kcache_query_row(lasvm_kcache_t *self, long i, long len){
  ASSERT(i>=0);
  if (i<self->length && len<=self->row_size[i])
    {
      self->row_next[self->row_previous[i]] = self->row_next[i];
      self->row_previous[self->row_next[i]] = self->row_previous[i];
    }
  else
    {
      long olen, p, q;
      double *d;
      if (i >= self->length || len >= self->length)
	xminsize(self, max(1+i,len));
      olen = self->row_size[i];
      if (olen < 0)
	{
	  self->row_diag_position[i] = (*self->kernel_function)(i, i, self->closure);
	  olen = self->row_size[i] = 0;
	}
      xextend(self, i, len);
      q = self->i2r_swap[i];
      d = self->row_data[i];
      for (p=olen; p<len; p++)
	{
	  long j = self->r2i_swap[p];
	  if (i == j)
	    d[p] = self->row_diag_position[i];
	  else if (q < self->row_size[j])
	    d[p] = self->row_data[j][q];
	  else
	    d[p] = (*self->kernel_function)(i, j, self->closure);
	}
      self->row_next[self->row_previous[i]] = self->row_next[i];
      self->row_previous[self->row_next[i]] = self->row_previous[i];
      xpurge(self);
    }
  self->row_previous[i] = -1;
  self->row_next[i] = self->row_next[-1];
  self->row_next[self->row_previous[i]] = i;
  self->row_previous[self->row_next[i]] = i;
  return self->row_data[i];
}

long lasvm_kcache_status_row(lasvm_kcache_t *self, long i){
  ASSERT(self);
  ASSERT(i>=0);
  if (i < self->length)
    return max(0,self->row_size[i]);
  return 0;
}

void lasvm_kcache_discard_row(lasvm_kcache_t *self, long i){
  ASSERT(self);
  ASSERT(i>=0);
  if (i<self->length && self->row_size[i]>0)
    {
      self->row_next[self->row_previous[i]] = self->row_next[i];
      self->row_previous[self->row_next[i]] = self->row_previous[i];
      self->row_previous[i] = self->row_previous[-1];
      self->row_next[i] = -1;
      self->row_next[self->row_previous[i]] = i;
      self->row_previous[self->row_next[i]] = i;
    }
}

void lasvm_kcache_set_maximum_size(lasvm_kcache_t *self, long entries){
  ASSERT(self);
  ASSERT(entries>0);
  self->max_size = entries;
  xpurge(self);
}

long lasvm_kcache_get_maximum_size(lasvm_kcache_t *self){
  ASSERT(self);
  return self->max_size;
}

long lasvm_kcache_get_current_size(lasvm_kcache_t *self){
  ASSERT(self);
  return self->current_size;
}

