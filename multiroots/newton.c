#include <config.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <gsl_math.h>
#include <gsl_errno.h>
#include <gsl_multiroots.h>
#include <gsl_linalg.h>

typedef struct
  {
    gsl_matrix * lu;
    gsl_vector_int * permutation;
  }
newton_state_t;

int newton_alloc (void * vstate, size_t n);
int newton_set (void * vstate, gsl_multiroot_function_fdf * fdf, gsl_vector * x, gsl_vector * f, gsl_matrix * J, gsl_vector * dx);
int newton_iterate (void * vstate, gsl_multiroot_function_fdf * fdf, gsl_vector * x, gsl_vector * f, gsl_matrix * J, gsl_vector * dx);
void newton_free (void * vstate);

int
newton_alloc (void * vstate, size_t n)
{
  newton_state_t * state = (newton_state_t *) vstate;
  gsl_vector_int * p;
  gsl_matrix * m;

  m = gsl_matrix_calloc (n,n);
  
  if (m == 0) 
    {
      GSL_ERROR_RETURN ("failed to allocate space for lu", GSL_ENOMEM, 0);
    }

  state->lu = m ;

  p = gsl_vector_int_calloc (n);

  if (p == 0)
    {
      gsl_matrix_free(m);

      GSL_ERROR_RETURN ("failed to allocate space for permutation", GSL_ENOMEM, 0);
    }

  state->permutation = p ;

  return GSL_SUCCESS;
}

int 
newton_set (void * vstate, gsl_multiroot_function_fdf * FDF, gsl_vector * x, gsl_vector * f, gsl_matrix * J, gsl_vector * dx)
{
  newton_state_t * state = (newton_state_t *) vstate;

  size_t i, n = FDF->n ;

  state = 0 ; /* avoid warnings about unused parameters */

  GSL_MULTIROOT_FN_EVAL_F_DF (FDF, x, f, J);

  for (i = 0; i < n; i++)
    {
      gsl_vector_set (dx, i, 0.0);
    }

  return GSL_SUCCESS;
}

int
newton_iterate (void * vstate, gsl_multiroot_function_fdf * fdf, gsl_vector * x, gsl_vector * f, gsl_matrix * J, gsl_vector * dx)
{
  newton_state_t * state = (newton_state_t *) vstate;
  
  int signum ;

  size_t i;

  size_t n = fdf->n ;

  gsl_matrix_copy (state->lu, J);

  gsl_la_decomp_LU_impl (state->lu, state->permutation, &signum);

  gsl_la_solve_LU_impl (state->lu, state->permutation, f, dx);
      
  for (i = 0; i < n; i++)
    {
      double e = gsl_vector_get (dx, i);
      double y = gsl_vector_get (x, i);
      gsl_vector_set (dx, i, -e);
      gsl_vector_set (x, i, y - e);
    }

  GSL_MULTIROOT_FN_EVAL_F_DF (fdf, x, f, J);

  return GSL_SUCCESS;
}


void
newton_free (void * vstate)
{
  newton_state_t * state = (newton_state_t *) vstate;

  gsl_matrix_free(state->lu);

  gsl_vector_int_free(state->permutation);
}


static const gsl_multiroot_fdfsolver_type newton_type =
{"newton",				/* name */
 sizeof (newton_state_t),
 &newton_alloc,
 &newton_set,
 &newton_iterate,
 &newton_free};

const gsl_multiroot_fdfsolver_type  * gsl_multiroot_fdfsolver_newton = &newton_type;
