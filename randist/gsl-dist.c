#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <gsl_randist.h>
#include <gsl_rng.h>
#include <gsl_test.h>

enum { EXPONENTIAL, CAUCHY, GAUSS } type;

int
main (int argc, char *argv[])
{
  size_t i;
  size_t n = 0;
  double mu = 1;
  double a = 1;
  const char * name ;
  double x;
  gsl_rng * r ;

  gsl_rng_env_setup() ;

  r = gsl_rng_alloc(gsl_rng_default) ;

  if (argc == 3) 
    {
      argc = 3; 
    }

  name = argv[1] ;
  n = strtol (argv[2], NULL, 0);

#define NAME(x) !strcmp(name,(x))
    
  if (NAME("exponential"))
    {
      for (i = 0; i < n; i++)
	printf("%g\n",gsl_ran_exponential (r, mu));
    }
  else if (NAME("levy"))
    {
      for (i = 0; i < n; i++)
	printf("%g\n",gsl_ran_levy (r, mu, a));
    }

  return 0 ;
}


