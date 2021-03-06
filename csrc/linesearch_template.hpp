/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%  BFGS tools Copyright (C) 2013  Wilmer Henao
%%  This program is free software: you can redistribute it and/or modify
%%  it under the terms of the GNU General Public License as published by
%%  the Free Software Foundation, either version 3 of the License, or
%%  (at your option) any later version.
%%
%%  This program is distributed in the hope that it will be useful,
%%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%%  GNU General Public License for more details.
%%
%%  You should have received a copy of the GNU General Public License
%%  along with this program.  If not, see <http://www.gnu.org/licenses/>.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include <cmath>
#include "libmatrix_template.hpp"
#include <cstdio>
#include <iostream>
#include <qd/dd_real.h>

template <class T>
T linesearch_ww (T*& x, T*& f, T*& g, T*& d, const T& C1, const T& C2, int& n, 
		 int(*&)(T*, T*, T*, int), int*&, T& ftarget, int*& exitflag);

template <class T>
T linesearch_ww (T*& x, T*& f, T*& g, T*& d, const T& C1, const T& C2, int& n, 
		 int(*&testFunction)(T*, T*, T*, int), int*& nfeval, T& ftarget,
		 int*& exitflag){
  T alpha = 0;		/* lower bound on step length*/
  T beta_max = 1e100;	/* upper bound on step length*/
  T beta  = beta_max;	
    
  T t = 1.0;               /* try step length 1 first*/
  T tprev = 0;           /* variable to store prev. t*/
    
  double done = 0;
    
  T nbisect = 0;
  T nexpand = 0;
  T dnorm = vecnorm<T>(d,n);
  
  /* MATLAB:
     nbisectmax = max(30, round(log2(1e5*dnorm))); % allows more if ||d|| big
     nexpandmax = max(10, round(log2(1e5/dnorm))); % allows more if ||d|| small
  */
  T nbisectmax, nexpandmax;
  if (0 == dnorm){
    nbisectmax = 100;
    nexpandmax = 1000;
  }
  else{
    nbisectmax = ceil( log( 100000*dnorm )/log(2.0) );
    if (nbisectmax < 100) nbisectmax = 100;
    nexpandmax = ceil( log( 100000 / dnorm )/log(2.0) );    
    if (nexpandmax < 100) nexpandmax = 100;
  }
  
  T gtd  = 0;
  T g0td;
  T armijo_rhs_p, wwolfe_rhs, f0;
  f0 = *f;
    
  g0td = vecip<T>(g, d, n);/* 
			    first gradient * search direction d (g0 in overton 
			    matlab)*/
    
  armijo_rhs_p = g0td*C1;
  wwolfe_rhs   = g0td*C2;
  while (!done) {
    /* x = x0 + t*d   (next x to check at):
       this is the same as x = x + (t-tprev)*d
       because x is overwritten each time*/
    vpv<T>(x, d, (t - tprev), n);
        
    testFunction(f,g,x,n);              /* evaluate f and g at new x*/
    *nfeval = *nfeval + 1;              /* counting number of function evaluations*/
        
    /* break out if f below target:*/
    if (*f < ftarget) {
      done = 1;
      *exitflag = 1;
      return t;
    }
    
    /* inner product of current g and d (result is gtd):*/
    gtd = vecip<T>(g,d,n);
    if (*f > f0 + t*armijo_rhs_p) {	/* armijo fails, gone too far*/
      beta = t;
    }
    else if (gtd < wwolfe_rhs) {	/* weak wolfe fails, not gone far enough*/
      alpha = t;
    }
    else {		                /* both conditions are ok, so quit:*/
      alpha = t;
      beta = t;
      done = 1;
    }
        
    tprev = t;	                        /* store t before changing it*/
    /* next t:*/
    if (beta < beta_max) {		/* bisection:*/
      if (nbisect < nbisectmax) {
	t = (alpha + beta) / 2; 
	nbisect+=1;
      }
      else {
	done = 1;
	*exitflag = -4;
      }
    }							
    else {		                /* expansion:*/
      if (nexpand < nexpandmax) {
	t = 2 * alpha;
	nexpand += 1;
      }
      else {
	done = 1;
	*exitflag = -5;
      }
    }
  } /* end while*/

  return t;
}

template <class T>
T linesearch_ww_lbfgsb(T*& x, T*& f, T*& g, T*& d, const T& C1, const T& C2, int& n, 
		 int(*&testFunction)(T*, T*, T*, int), int*& nfeval, T& ftarget){
  T alpha = 0;		/* lower bound on step length*/
  T beta_max = 1.00001;	/* upper bound on step length*/
  T beta  = beta_max;	
    
  T t = 1.0;               /* try step length 1 first*/
  T tprev = 0;           /* variable to store prev. t*/
    
  double done = 0;
    
  T nbisect = 0;
  T nexpand = 0;
  T dnorm = vecnorm<T>(d,n);
  
  /* MATLAB:
     nbisectmax = max(30, round(log2(1e5*dnorm))); % allows more if ||d|| big
     nexpandmax = max(10, round(log2(1e5/dnorm))); % allows more if ||d|| small
  */
  T nbisectmax, nexpandmax;
  if (0 == dnorm){
    nbisectmax = 100;
    nexpandmax = 1000;
  }
  else{
    nbisectmax = ceil( log( 100000*dnorm )/log(2.0) );
    if (nbisectmax < 100) nbisectmax = 100;
    nexpandmax = ceil( log( 100000 / dnorm )/log(2.0) );    
    if (nexpandmax < 100) nexpandmax = 100;
  }
  
  T gtd  = 0;
  T g0td;
  T armijo_rhs_p, wwolfe_rhs, f0;
  f0 = *f;
    
  g0td = vecip<T>(g, d, n);/* 
			    first gradient * search direction d (g0 in overton 
			    matlab)*/
    
  armijo_rhs_p = g0td*C1;
  wwolfe_rhs   = g0td*C2;
  while (!done) {
    /* x = x0 + t*d   (next x to check at):
       this is the same as x = x + (t-tprev)*d
       because x is overwritten each time*/
    vpv<T>(x, d, (t - tprev), n);
        
    testFunction(f,g,x,n);              /* evaluate f and g at new x*/
    *nfeval = *nfeval + 1;              /* counting number of function evaluations*/
        
    /* break out if f below target:*/
    if (*f < ftarget) {
      done = 1;
      return t;
    }
    
    /* inner product of current g and d (result is gtd):*/
    gtd = vecip<T>(g,d,n);
    if (*f > f0 + t*armijo_rhs_p) {	/* armijo fails, gone too far*/
      beta = t;
    }
    else if (gtd < wwolfe_rhs) {	/* weak wolfe fails, not gone far enough*/
      alpha = t;
    }
    else {		                /* both conditions are ok, so quit:*/
      alpha = t;
      beta = t;
      done = 1;
    }
        
    tprev = t;	                        /* store t before changing it*/
    /* next t:*/
    if (beta < beta_max) {		/* bisection:*/
      if (nbisect < nbisectmax) {
	t = (alpha + beta) / 2; 
	nbisect+=1;
      }
      else {
	done = 1;
      }
    }							
    else {		                /* expansion:*/
      if (nexpand < nexpandmax) {
	t = 2 * alpha;
	nexpand += 1;
      }
      else {
	done = 1;
      }
    }
  } /* end while*/

  return t;
}
