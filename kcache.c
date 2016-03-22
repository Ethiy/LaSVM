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

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "messages.hpp"
#include "kcache.hpp"


#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
#endif

struct lasvm_kcache_s {
  lasvm_kernel_t func;
  void *closure;
  long maxsize;
  long cursize;
  long l;
  long *i2r;
  long *r2i;
  /* Rows */
  long    *rsize;
  double  *rdiag;
  double **rdata;
  long    *rnext;
  long    *rprev;
  long    *qnext;
  long    *qprev;
};

static void *
xmalloc(long n)
{
  void *p = malloc(n);
  if (! p) 
    lasvm_error("Function malloc() has returned zero\n");
  return p;
}

static void *
xrealloc(void *ptr, long n)
{
  if (! ptr)
    ptr = malloc(n);
  else
    ptr = realloc(ptr, n);
  if (! ptr) 
    lasvm_error("Function realloc has returned zero\n");
  return ptr;
}

static void
xminsize(lasvm_kcache_t *self, long n)
{
  long ol = self->l;
  if (n > ol)
    {
      long i;
      long nl = max(256,ol);
      while (nl < n)
	nl = nl + nl;
      self->i2r = static_cast<long*>(xrealloc(self->i2r, nl*sizeof(int)));
      self->r2i = static_cast<long*>(xrealloc(self->r2i, nl*sizeof(int)));
      self->rsize = static_cast<long*>(xrealloc(self->rsize, nl*sizeof(int)));
      self->qnext = static_cast<long*>(xrealloc(self->qnext, (1+nl)*sizeof(int)));
      self->qprev = static_cast<long*>(xrealloc(self->qprev, (1+nl)*sizeof(int)));
      self->rdiag = static_cast<double*>(xrealloc(self->rdiag, nl*sizeof(double)));
      self->rdata = static_cast<double**>(xrealloc(self->rdata, nl*sizeof(double*)));
      self->rnext = self->qnext + 1;
      self->rprev = self->qprev + 1;
      for (i=ol; i<nl; i++)
	{
	  self->i2r[i] = i;
	  self->r2i[i] = i;
	  self->rsize[i] = -1;
	  self->rnext[i] = i;
	  self->rprev[i] = i;
	  self->rdata[i] = 0;
	}
      self->l = nl;
    }
}

lasvm_kcache_t* 
lasvm_kcache_create(lasvm_kernel_t kernelfunc, void *closure)
{
  lasvm_kcache_t *self;
  self = static_cast<lasvm_kcache_t*>(xmalloc(sizeof(lasvm_kcache_t)));
  memset(self, 0, sizeof(lasvm_kcache_t));
  self->l = 0;
  self->func = kernelfunc;
  self->closure = closure;
  self->cursize = sizeof(lasvm_kcache_t);
  self->maxsize = 256*1024*1024;
  self->qprev = static_cast<long*>(xmalloc(sizeof(int)));
  self->qnext = static_cast<long*>(xmalloc(sizeof(int)));
  self->rnext = self->qnext + 1;
  self->rprev = static_cast<long*> (self->qprev + 1);
  self->rprev[-1] = -1;
  self->rnext[-1] = -1;
  return self;
}

void 
lasvm_kcache_destroy(lasvm_kcache_t *self)
{
  if (self)
    {
      long i;
      if (self->i2r)
	free(self->i2r);
      if (self->r2i)
	free(self->r2i);
      if (self->rdata)
	for (i=0; i<self->l; i++)
	  if (self->rdata[i])
	    free(self->rdata[i]);
      if (self->rdata)
	free(self->rdata);
      if (self->rsize)
	free(self->rsize);
      if (self->rdiag)
	free(self->rdiag);
      if (self->qnext)
	free(self->qnext);
      if (self->qprev)
	free(self->qprev);
      memset(self, 0, sizeof(lasvm_kcache_t));
      free(self);
    }
}

long *
lasvm_kcache_i2r(lasvm_kcache_t *self, long n)
{
  xminsize(self, n);
  return self->i2r;
}

long *
lasvm_kcache_r2i(lasvm_kcache_t *self, long n)
{
  xminsize(self, n);
  return self->r2i;
}

static void
xextend(lasvm_kcache_t *self, long k, long nlen)
{
  long olen = self->rsize[k];
  if (nlen > olen)
    {
      double *ndata = static_cast<double*>(xmalloc(nlen*sizeof(double)));
      if (olen > 0)
	{
	  double *odata = self->rdata[k];
	  memcpy(ndata, odata, olen * sizeof(double));
	  free(odata);
	}
      self->rdata[k] = ndata;
      self->rsize[k] = nlen;
      self->cursize += static_cast<long>(nlen - olen) * sizeof(double);
    }
}

static void
xtruncate(lasvm_kcache_t *self, long k, long nlen)
{
  long olen = self->rsize[k];
  if (nlen < olen)
    {
      double *ndata;
      double *odata = self->rdata[k];
      if (nlen >  0)
	{
	  ndata = static_cast<double*>(xmalloc(nlen*sizeof(double)));
	  memcpy(ndata, odata, nlen * sizeof(double));
	}
      else
	{
	  ndata = 0;
	  self->rnext[self->rprev[k]] = self->rnext[k];
	  self->rprev[self->rnext[k]] = self->rprev[k];
	  self->rnext[k] = self->rprev[k] = k;
	}
      free(odata);
      self->rdata[k] = ndata;
      self->rsize[k] = nlen;
      self->cursize += static_cast<long>(nlen - olen) * sizeof(double);
    }
}

static void
xswap(lasvm_kcache_t *self, long i1, long i2, long r1, long r2)
{
  long k = self->rnext[-1];
  while (k >= 0)
    {
      long nk = self->rnext[k];
      long n  = self->rsize[k];
      long rr = self->i2r[k];
      double *d = self->rdata[k];
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
              d[r1] = self->rdiag[k];
            }
          else
            {
	      long arsize = self->rsize[i2];
              if (rr < arsize && rr != r1)
                d[r1] = self->rdata[i2][rr];
              else
                xtruncate(self, k, r1);
            }
	}
      else if (r2 < n)
        {
          if (rr == r1)
            {
              d[r2] = self->rdiag[k];
            }
          else 
            {
	      long arsize = self->rsize[i1];
              if (rr < arsize && rr != r2)
                d[r2] = self->rdata[i1][rr];
              else
                xtruncate(self, k, r2);
            }
        }
      k = nk;
    }
  self->r2i[r1] = i2;
  self->r2i[r2] = i1;
  self->i2r[i1] = r2;
  self->i2r[i2] = r1;
}

void 
lasvm_kcache_swap_rr(lasvm_kcache_t *self, long r1, long r2)
{
  xminsize(self, 1+max(r1,r2));
  xswap(self, self->r2i[r1], self->r2i[r2], r1, r2);
}

void 
lasvm_kcache_swap_ii(lasvm_kcache_t *self, long i1, long i2)
{
  xminsize(self, 1+max(i1,i2));
  xswap(self, i1, i2, self->i2r[i1], self->i2r[i2]);
}

void 
lasvm_kcache_swap_ri(lasvm_kcache_t *self, long r1, long i2)
{
  xminsize(self, 1+max(r1,i2));
  xswap(self, self->r2i[r1], i2, r1, self->i2r[i2]);
}

double 
lasvm_kcache_query(lasvm_kcache_t *self, long i, long j)
{
  long l = self->l;
  ASSERT(i>=0);
  ASSERT(j>=0);
  if (i<l && j<l)
    {
      /* check cache */
      long s = self->rsize[i];
      long p = self->i2r[j];
      if (p < s)
	return self->rdata[i][p];
      else if (i == j && s >= 0)
	return self->rdiag[i];
      p = self->i2r[i];
      s = self->rsize[j];
      if (p < s)
	return self->rdata[j][p];
    }
  /* compute */
  return (*self->func)(i, j, self->closure);
}

static void 
xpurge(lasvm_kcache_t *self)
{
  if (self->cursize>self->maxsize)
    {
      long k = self->rprev[-1];
      while (self->cursize>self->maxsize && k!=self->rnext[-1])
	{
	  long pk = self->rprev[k];
          xtruncate(self, k, 0);
	  k = pk;
	}
    }
}

double *
lasvm_kcache_query_row(lasvm_kcache_t *self, long i, long len)
{
  ASSERT(i>=0);
  if (i<self->l && len<=self->rsize[i])
    {
      self->rnext[self->rprev[i]] = self->rnext[i];
      self->rprev[self->rnext[i]] = self->rprev[i];
    }
  else
    {
      long olen, p, q;
      double *d;
      if (i >= self->l || len >= self->l)
	xminsize(self, max(1+i,len));
      olen = self->rsize[i];
      if (olen < 0)
	{
	  self->rdiag[i] = (*self->func)(i, i, self->closure);
	  olen = self->rsize[i] = 0;
	}
      xextend(self, i, len);
      q = self->i2r[i];
      d = self->rdata[i];
      for (p=olen; p<len; p++)
	{
	  long j = self->r2i[p];
	  if (i == j)
	    d[p] = self->rdiag[i];
	  else if (q < self->rsize[j])
	    d[p] = self->rdata[j][q];
	  else
	    d[p] = (*self->func)(i, j, self->closure);
	}
      self->rnext[self->rprev[i]] = self->rnext[i];
      self->rprev[self->rnext[i]] = self->rprev[i];
      xpurge(self);
    }
  self->rprev[i] = -1;
  self->rnext[i] = self->rnext[-1];
  self->rnext[self->rprev[i]] = i;
  self->rprev[self->rnext[i]] = i;
  return self->rdata[i];
}

long 
lasvm_kcache_status_row(lasvm_kcache_t *self, long i)
{
  ASSERT(self);
  ASSERT(i>=0);
  if (i < self->l)
    return max(0,self->rsize[i]);
  return 0;
}

void 
lasvm_kcache_discard_row(lasvm_kcache_t *self, long i)
{
  ASSERT(self);
  ASSERT(i>=0);
  if (i<self->l && self->rsize[i]>0)
    {
      self->rnext[self->rprev[i]] = self->rnext[i];
      self->rprev[self->rnext[i]] = self->rprev[i];
      self->rprev[i] = self->rprev[-1];
      self->rnext[i] = -1;
      self->rnext[self->rprev[i]] = i;
      self->rprev[self->rnext[i]] = i;
    }
}

void 
lasvm_kcache_set_maximum_size(lasvm_kcache_t *self, long entries)
{
  ASSERT(self);
  ASSERT(entries>0);
  self->maxsize = entries;
  xpurge(self);
}

long
lasvm_kcache_get_maximum_size(lasvm_kcache_t *self)
{
  ASSERT(self);
  return self->maxsize;
}

long
lasvm_kcache_get_current_size(lasvm_kcache_t *self)
{
  ASSERT(self);
  return self->cursize;
}

