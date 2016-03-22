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
  int length;
  int *i2r_swap;
  int *r2i_swap;
  /* Rows */
  int    *row_size;
  float  *row_diag_position;
  float **row_data;
  int    *row_next;
  int    *row_previous;
  int    *qnext;
  int    *qprev;
};

static void * xmalloc(int n){
  void *ptr = malloc(n);
  if (! ptr) 
    lasvm_error("Function malloc() has returned zero\n");

  return ptr;
}

static void * xrealloc(void *ptr, int n){
  if (! ptr)
    ptr = malloc(n);
  else
    ptr = realloc(ptr, n);
  if (! ptr) 
    lasvm_error("Function realloc has returned zero\n");

  return ptr;
}

static void xminsize(lasvm_kcache_t *self, int n)
{
  int ol = self->length;
  if (n > ol)
    {
      int i;
      int nl = max(256,ol);
      while (nl < n)
	nl = nl + nl;
      self->i2r_swap = (int*)xrealloc(self->i2r_swap, nl*sizeof(int));
      self->r2i_swap = (int*)xrealloc(self->r2i_swap, nl*sizeof(int));
      self->row_size = (int*)xrealloc(self->row_size, nl*sizeof(int));
      self->qnext = (int*)xrealloc(self->qnext, (1+nl)*sizeof(int));
      self->qprev = (int*)xrealloc(self->qprev, (1+nl)*sizeof(int));
      self->row_diag_position = (float*)xrealloc(self->row_diag_position, nl*sizeof(float));
      self->row_data = (float**)xrealloc(self->row_data, nl*sizeof(float*));
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
  self->qprev = (int*)xmalloc(sizeof(int));
  self->qnext = (int*)xmalloc(sizeof(int));
  self->row_next = self->qnext + 1;
  self->row_previous = self->qprev + 1;
  self->row_previous[-1] = -1;
  self->row_next[-1] = -1;
  return self;
}

void lasvm_kcache_destroy(lasvm_kcache_t *self){
  if (self){
      int i;
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

int * lasvm_kcache_i2r(lasvm_kcache_t *self, int n){
  xminsize(self, n);
  return self->i2r_swap;
}

int * lasvm_kcache_r2i(lasvm_kcache_t *self, int n){
  xminsize(self, n);
  return self->r2i_swap;
}

static void xextend(lasvm_kcache_t *self, int k, int nlen){
  int olen = self->row_size[k];
  if (nlen > olen)
    {
      float *ndata = (float*)xmalloc(nlen*sizeof(float));
      if (olen > 0)
	{
	  float *odata = self->row_data[k];
	  memcpy(ndata, odata, olen * sizeof(float));
	  free(odata);
	}
      self->row_data[k] = ndata;
      self->row_size[k] = nlen;
      self->current_size += (long)(nlen - olen) * sizeof(float);
    }
}

static void xtruncate(lasvm_kcache_t *self, int k, int nlen)
{
  int olen = self->row_size[k];
  if (nlen < olen)
    {
      float *ndata;
      float *odata = self->row_data[k];
      if (nlen >  0)
	{
	  ndata = (float*)xmalloc(nlen*sizeof(float));
	  memcpy(ndata, odata, nlen * sizeof(float));
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
      self->current_size += (long)(nlen - olen) * sizeof(float);
    }
}

static void xswap(lasvm_kcache_t *self, int i1, int i2, int r1, int r2){
  int k = self->row_next[-1];
  while (k >= 0)
    {
      int nk = self->row_next[k];
      int n  = self->row_size[k];
      int rr = self->i2r_swap[k];
      float *d = self->row_data[k];
      if (r1 < n)
	{
	  if (r2 < n)
	    {
	      float t1 = d[r1];
	      float t2 = d[r2];
	      d[r1] = t2;
	      d[r2] = t1;
	    }
	  else if (rr == r2)
            {
              d[r1] = self->row_diag_position[k];
            }
          else
            {
	      int arsize = self->row_size[i2];
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
	      int arsize = self->row_size[i1];
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

void lasvm_kcache_swap_rr(lasvm_kcache_t *self, int r1, int r2){
  xminsize(self, 1+max(r1,r2));
  xswap(self, self->r2i_swap[r1], self->r2i_swap[r2], r1, r2);
}

void lasvm_kcache_swap_ii(lasvm_kcache_t *self, int i1, int i2){
  xminsize(self, 1+max(i1,i2));
  xswap(self, i1, i2, self->i2r_swap[i1], self->i2r_swap[i2]);
}

void lasvm_kcache_swap_ri(lasvm_kcache_t *self, int r1, int i2){
  xminsize(self, 1+max(r1,i2));
  xswap(self, self->r2i_swap[r1], i2, r1, self->i2r_swap[i2]);
}

double lasvm_kcache_query(lasvm_kcache_t *self, int i, int j){
  int length = self->length;
  ASSERT(i>=0);
  ASSERT(j>=0);
  if (i<length && j<length)
    {
      /* check cache */
      int s = self->row_size[i];
      int p = self->i2r_swap[j];
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
      int k = self->row_previous[-1];
      while (self->current_size>self->max_size && k!=self->row_next[-1])
	{
	  int pk = self->row_previous[k];
          xtruncate(self, k, 0);
	  k = pk;
	}
    }
}

float * lasvm_kcache_query_row(lasvm_kcache_t *self, int i, int len){
  ASSERT(i>=0);
  if (i<self->length && len<=self->row_size[i])
    {
      self->row_next[self->row_previous[i]] = self->row_next[i];
      self->row_previous[self->row_next[i]] = self->row_previous[i];
    }
  else
    {
      int olen, p, q;
      float *d;
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
	  int j = self->r2i_swap[p];
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

int lasvm_kcache_status_row(lasvm_kcache_t *self, int i){
  ASSERT(self);
  ASSERT(i>=0);
  if (i < self->length)
    return max(0,self->row_size[i]);
  return 0;
}

void lasvm_kcache_discard_row(lasvm_kcache_t *self, int i){
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

