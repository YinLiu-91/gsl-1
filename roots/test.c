/* test -- program to test root finding functions */
/* $Id$ */


/* config headers */
#include <config.h>

/* standard headers */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* gsl headers */
#include <gsl_errno.h>
#include <gsl_roots.h>

/* roots headers */
#include "roots.h"

/* test headers */
#include "test.h"


/* stopping parameters */
double TEST_EPSILON = DBL_EPSILON * GSL_ROOT_EPSILON_BUFFER;
unsigned int TEST_MAX_ITERATIONS = 100;


/* administer all the tests */
int
main(void)
{
  int num_passed = 0, num_failed = 0;

  printf("Testing root finding functions (GSL version %s)...\n", VERSION);

  /* turn off error handling -- we will take care of everything */
  gsl_set_error_handler(gsl_no_error_handler);


  /* test macros */
  printf("Testing macros:\n");
  gsl_test_root_macros(&num_passed, &num_failed);

  /* test bisection */
  printf("Testing `gsl_root_bisection': \n");
  printf("  sin(x) [3, 4] ... ");
  gsl_test_root_bisection(sin, 3.0, 4.0, M_PI, &num_passed, &num_failed); 
  printf("  sin(x) [-4, -3] ... ");
  gsl_test_root_bisection(sin, -4.0, -3.0, -M_PI, &num_passed, &num_failed); 
  printf("  sin(x) [-1/3, 1] ... ");
  gsl_test_root_bisection(sin, -1.0/3.0, 1.0, 0.0, &num_passed, &num_failed); 
  printf("  cos(x) [0, 3] ... ");
  gsl_test_root_bisection(cos, 0.0, 3.0, M_PI/2.0, &num_passed, &num_failed); 
  printf("  cos(x) [-3, 0] ... ");
  gsl_test_root_bisection(cos, -3.0, 0.0, -M_PI/2.0, &num_passed,
                          &num_failed); 
  printf("  x^{20} - 1 [0.9, 1.1] ... ");
  gsl_test_root_bisection(gsl_test_root_hairy_1, 0.9, 1.1, 1.0, &num_passed,
                          &num_failed); 
  printf("  sqrt(abs(x)) * sgn(x) [-1/3, 1] ... ");
  gsl_test_root_bisection(gsl_test_root_hairy_2, -1.0/3.0, 1.0, 0.0,
                          &num_passed, &num_failed);
  printf("  x^2 - 1e-8 [0, 1] ... ");
  gsl_test_root_bisection(gsl_test_root_hairy_3, 0.0, 1.0, sqrt(1e-8),
                          &num_passed, &num_failed);
  printf("  x exp(-x) [-1/3, 2] ... ");
  gsl_test_root_bisection(gsl_test_root_hairy_4, -1.0/3.0, 2.0, 0.0,
                          &num_passed, &num_failed);

  /* now summarize the results */
  printf("--\nTesting complete.\n");
  if (num_failed == 0)
    printf("All %d tests passed.\n", num_passed);
  else {
    printf("%d of %d tests passed, %d failed.\n", num_passed,
           num_passed + num_failed, num_failed);
    printf("The integrity of this package may be suspect!\n");
  }

  return GSL_SUCCESS;
}

void
gsl_test_root_macros(int * num_passed, int * num_failed)
{
  /* GSL_ISREAL */
  /* 1.0 is real */
  printf("  GSL_ISREAL(1.0) ... ");
  if (GSL_ISREAL(1.0)) {
    printf("ok\n");
    (void)(*num_passed)++;
  }
  else {
    printf("failed\n");
    (void)(*num_failed)++;
  }
  /* 1.0/0.0 == Inf is not real */
  printf("  GSL_ISREAL(1.0/0.0) ... ");
  if (!GSL_ISREAL(1.0/0.0)) {
    printf("ok\n");
    (void)(*num_passed)++;
  }
  else {
    printf("failed\n");
    (void)(*num_failed)++;
  }
  /* 0.0/0.0 == NaN is not real */
  printf("  GSL_ISREAL(0.0/0.0) ... ");
  if (!GSL_ISREAL(0.0/0.0)) {
    printf("ok\n");
    (void)(*num_passed)++;
  }
  else {
    printf("failed\n");
    (void)(*num_failed)++;
  }
}

/* Find the root of the function pointed to by f, using the interval
   [lower_bound, upper_bound]. Check if f succeeded and that it was accurate
   enough. */
void
gsl_test_root_bisection(double (* f)(double), double lower_bound,
                        double upper_bound, double cor_root, int * num_passed,
                        int * num_failed)
{
  int err;
  double root;
  
  err = gsl_root_bisection(&root, f, &lower_bound, &upper_bound, TEST_EPSILON,
                           TEST_MAX_ITERATIONS);
  /* if there was an error */
  if (err != GSL_SUCCESS) {
    printf("failed (gsl_errno = %d)\n", gsl_errno);
    (void)(*num_failed)++;
  }
  /* if it wasn't accurate enough */
  else if (_GSL_ROOT_ERR(cor_root, root) > TEST_EPSILON) {
    printf("failed (inaccurate)\n");
    (void)(*num_failed)++;
  }
  /* if the test passed */
  else {
    printf("ok\n");
    (void)(*num_passed)++;
  }
}

/* f(x) = x^{20} - 1 */
/* zero at x = 1 or -1 */
double
gsl_test_root_hairy_1(double x)
{
  return pow(x, 20.0) - 1;
}

/* f(x) = sqrt(abs(x))*sgn(x) */
/* zero at ?? */
double
gsl_test_root_hairy_2(double x)
{
  double delta;

  if (x > 0)
    delta = 1.0;
  else if (x < 0)
    delta = -1.0;
  else
    delta = 0.0;

  return sqrt(fabs(x))*delta;
}

/* f(x) = x^2 - 1e-8 */
/* zero at x = sqrt(1e-8) or -sqrt(1e-8) */
double
gsl_test_root_hairy_3(double x)
{
  return pow(x, 2.0) - 1e-8;
}

/* f(x) = x exp(-x) */
/* zero at x = 0 */
double
gsl_test_root_hairy_4(double x)
{
  return x * exp(-x);
}

/* f(x) = 1/(1+exp(x)) */
/* no roots! */
double
gsl_test_root_hairy_5(double x)
{
  return 1 / (1 + exp(x));
}
