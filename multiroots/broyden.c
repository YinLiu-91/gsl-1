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

#include "enorm.c"

/* Broyden's method. It is not an efficient or modern algorithm but
   gives an example of a rank-1 update.

   C.G. Broyden, "A Class of Methods for Solving Nonlinear
   Simultaneous Equations", Mathematics of Computation, vol 19 (1965),
   p 577-593

 */

typedef struct
  {
    gsl_matrix *H;
    gsl_matrix *lu;
    gsl_vector_int *permutation;
    gsl_vector *v;
    gsl_vector *w;
    gsl_vector *y;
    gsl_vector *p;
    gsl_vector *fnew;
    gsl_vector *x_trial;
    double phi;
  }
broyden_state_t;

int broyden_alloc (void *vstate, size_t n);
int broyden_set (void *vstate, gsl_multiroot_function * function, gsl_vector * x, gsl_vector * f, gsl_vector * dx);
int broyden_iterate (void *vstate, gsl_multiroot_function * function, gsl_vector * x, gsl_vector * f, gsl_vector * dx);
void broyden_free (void *vstate);


int
broyden_alloc (void *vstate, size_t n)
{
  broyden_state_t *state = (broyden_state_t *) vstate;
  gsl_vector *v, *w, *y, *fnew, *x_trial, *p;
  gsl_vector_int *perm;
  gsl_matrix *m, *H;

  m = gsl_matrix_calloc (n, n);

  if (m == 0)
    {
      GSL_ERROR_RETURN ("failed to allocate space for lu", GSL_ENOMEM, 0);
    }

  state->lu = m;

  perm = gsl_vector_int_calloc (n);

  if (perm == 0)
    {
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for permutation", GSL_ENOMEM, 0);
    }

  state->permutation = perm;

  H = gsl_matrix_calloc (n, n);

  if (H == 0)
    {
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for d", GSL_ENOMEM, 0);
    }

  state->H = H;

  v = gsl_vector_calloc (n);

  if (v == 0)
    {
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for v", GSL_ENOMEM, 0);
    }

  state->v = v;

  w = gsl_vector_calloc (n);

  if (w == 0)
    {
      gsl_vector_free (v);
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for w", GSL_ENOMEM, 0);
    }

  state->w = w;

  y = gsl_vector_calloc (n);

  if (y == 0)
    {
      gsl_vector_free (w);
      gsl_vector_free (v);
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for y", GSL_ENOMEM, 0);
    }

  state->y = y;

  fnew = gsl_vector_calloc (n);

  if (fnew == 0)
    {
      gsl_vector_free (y);
      gsl_vector_free (w);
      gsl_vector_free (v);
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for fnew", GSL_ENOMEM, 0);
    }

  state->fnew = fnew;

  x_trial = gsl_vector_calloc (n);

  if (x_trial == 0)
    {
      gsl_vector_free (fnew);
      gsl_vector_free (y);
      gsl_vector_free (w);
      gsl_vector_free (v);
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for x_trial", GSL_ENOMEM, 0);
    }

  state->x_trial = x_trial;

  p = gsl_vector_calloc (n);

  if (p == 0)
    {
      gsl_vector_free (x_trial);
      gsl_vector_free (fnew);
      gsl_vector_free (y);
      gsl_vector_free (w);
      gsl_vector_free (v);
      gsl_matrix_free (H);
      gsl_vector_int_free (perm);
      gsl_matrix_free (m);

      GSL_ERROR_RETURN ("failed to allocate space for p", GSL_ENOMEM, 0);
    }

  state->p = p;

  return GSL_SUCCESS;
}

int
broyden_set (void *vstate, gsl_multiroot_function * function, gsl_vector * x, gsl_vector * f, gsl_vector * dx)
{
  broyden_state_t *state = (broyden_state_t *) vstate;
  size_t i, j, n = function->n;
  int signum = 0;

  GSL_MULTIROOT_FN_EVAL (function, x, f);

  gsl_multiroot_fdjacobian (function, x, f, GSL_SQRT_DBL_EPSILON, state->lu);
  gsl_la_decomp_LU_impl (state->lu, state->permutation, &signum);
  gsl_la_invert_LU (state->lu, state->permutation, state->H);

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      gsl_matrix_set(state->H,i,j,-gsl_matrix_get(state->H,i,j));

  for (i = 0; i < n; i++)
    {
      gsl_vector_set (dx, i, 0.0);
    }

  state->phi = enorm (f);

  return GSL_SUCCESS;
}

int
broyden_iterate (void *vstate, gsl_multiroot_function * function, gsl_vector * x, gsl_vector * f, gsl_vector * dx)
{
  broyden_state_t *state = (broyden_state_t *) vstate;

  double phi0, phi1, t, lambda;

  gsl_matrix *H = state->H;
  gsl_vector *p = state->p;
  gsl_vector *y = state->y;
  gsl_vector *v = state->v;
  gsl_vector *w = state->w;
  gsl_vector *fnew = state->fnew;
  gsl_vector *x_trial = state->x_trial;
  gsl_matrix *lu = state->lu;
  gsl_vector_int *perm = state->permutation;

  size_t i, j, iter;

  size_t n = function->n;

restart:

  /* p = H f */

  for (i = 0; i < n; i++)
    {
      double sum = 0;

      for (j = 0; j < n; j++)
	{
	  sum += gsl_matrix_get (H, i, j) * gsl_vector_get (f, j);
	}
      gsl_vector_set (p, i, sum);
    }

  t = 1;
  iter = 0;

  phi0 = state->phi;

new_step:

  for (i = 0; i < n; i++)
    {
      double pi = gsl_vector_get (p, i);
      double xi = gsl_vector_get (x, i);
      gsl_vector_set (x_trial, i, xi + t * pi);
    }

  GSL_MULTIROOT_FN_EVAL (function, x_trial, fnew);

  phi1 = enorm (fnew);

  iter++ ;

  if (phi1 > phi0 && iter < 10)
    {
      /* full step goes uphill, take a reduced step instead */
      
      if (t > 0.1)
        {
          double theta = phi1 / phi0;
          t *= (sqrt (1.0 + 6.0 * theta) - 1.0) / (3.0 * theta);
          goto new_step;
        }
      else
        {
          /* need to recompute Jacobian */
          int signum = 0;
          
          gsl_multiroot_fdjacobian (function, x, f, GSL_SQRT_DBL_EPSILON, lu);
          gsl_la_decomp_LU_impl (lu, perm, &signum);
          gsl_la_invert_LU (lu, perm, H);
          
          for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
              gsl_matrix_set(H,i,j,-gsl_matrix_get(H,i,j));

          goto restart;
        }
    }

  /* y = f' - f */

  for (i = 0; i < n; i++)
    {
      double yi = gsl_vector_get (fnew, i) - gsl_vector_get (f, i);
      gsl_vector_set (y, i, yi);
    }

  /* v = H y */

  for (i = 0; i < n; i++)
    {
      double sum = 0;

      for (j = 0; j < n; j++)
	{
	  sum += gsl_matrix_get (H, i, j) * gsl_vector_get (y, j);
	}

      gsl_vector_set (v, i, sum);
    }

  /* lambda = p . v */

  lambda = 0;

  for (i = 0; i < n; i++)
    {
      lambda += gsl_vector_get (p, i) * gsl_vector_get (v, i);
    }

  if (lambda == 0)
    {
      GSL_ERROR ("approximation to Jacobian has collapsed", GSL_EZERODIV) ;
    }

  /* v' = v + t * p */

  for (i = 0; i < n; i++)
    {
      double vi = gsl_vector_get (v, i) + t * gsl_vector_get (p, i);
      gsl_vector_set (v, i, vi);
    }

  /* w^T = p^T H */

  for (i = 0; i < n; i++)
    {
      double sum = 0;

      for (j = 0; j < n; j++)
	{
	  sum += gsl_matrix_get (H, j, i) * gsl_vector_get (p, j);
	}

      gsl_vector_set (w, i, sum);
    }

  /* Hij -> Hij - (vi wj / lambda) */

  for (i = 0; i < n; i++)
    {
      double vi = gsl_vector_get (v, i);

      for (j = 0; j < n; j++)
	{
	  double wj = gsl_vector_get (w, j);
	  double Hij = gsl_matrix_get (H, i, j) - vi * wj / lambda;
	  gsl_matrix_set (H, i, j, Hij);
	}
    }

  /* copy fnew into f */

  gsl_vector_copy (f, fnew);

  /* copy x_trial into x */

  gsl_vector_copy (x, x_trial);

  for (i = 0; i < n; i++)
    {
      double pi = gsl_vector_get (p, i);
      gsl_vector_set (dx, i, t * pi);
    }

  state->phi = phi1;

  return GSL_SUCCESS;
}


void
broyden_free (void *vstate)
{
  broyden_state_t *state = (broyden_state_t *) vstate;

  gsl_matrix_free (state->H);

  gsl_matrix_free (state->lu);
  gsl_vector_int_free (state->permutation);
  
  gsl_vector_free (state->v);
  gsl_vector_free (state->w);
  gsl_vector_free (state->y);
  gsl_vector_free (state->p);

  gsl_vector_free (state->fnew);
  gsl_vector_free (state->x_trial);
  
}


static const gsl_multiroot_fsolver_type broyden_type =
{"broyden",			/* name */
 sizeof (broyden_state_t),
 &broyden_alloc,
 &broyden_set,
 &broyden_iterate,
 &broyden_free};

const gsl_multiroot_fsolver_type *gsl_multiroot_fsolver_broyden = &broyden_type;
