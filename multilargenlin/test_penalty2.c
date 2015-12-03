#define penalty2_N         8 /* 2*p */
#define penalty2_P         4

#define penalty2_NTRIES    3

static double penalty2_x0[penalty2_P] = { 0.5, 0.5, 0.5, 0.5 };
static double penalty2_epsrel = 1.0e-6;

static double penalty2_f[penalty2_N];
static double penalty2_J[penalty2_N * penalty2_P];

static void
penalty2_checksol(const double x[], const double sumsq,
                  const double epsrel, const char *sname,
                  const char *pname)
{
  const double sumsq_exact = 9.37629300735544219e-06;

  gsl_test_rel(sumsq, sumsq_exact, epsrel, "%s/%s sumsq",
               sname, pname);

  (void)x; /* avoid unused parameter warning */
}

static int
penalty2_fdf (const gsl_vector * x, gsl_matrix * JTJ,
              gsl_vector * JTf, double * normf, void *params)
{
  gsl_matrix_view J = gsl_matrix_view_array(penalty2_J, penalty2_N, penalty2_P);
  gsl_vector_view f = gsl_vector_view_array(penalty2_f, penalty2_N);
  const double alpha = 1.0e-5;
  const double sqrt_alpha = sqrt(alpha);
  double x1 = gsl_vector_get(x, 0);
  double sum = penalty2_P * x1 * x1;
  size_t i, j;

  gsl_vector_set(&f.vector, 0, x1 - 0.2);

  /* rows [2:p] */
  for (i = 1; i < penalty2_P; ++i)
    {
      double xi = gsl_vector_get(x, i);
      double xim1 = gsl_vector_get(x, i - 1);
      double yi = exp(0.1*(i + 1.0)) + exp(0.1*i);

      gsl_vector_set(&f.vector, i, sqrt_alpha*(exp(0.1*xi) + exp(0.1*xim1) - yi));

      sum += (penalty2_P - i) * xi * xi;
    }

  /* rows [p+1:2p-1] */
  for (i = penalty2_P; i < penalty2_N - 1; ++i)
    {
      double xi = gsl_vector_get(x, i - penalty2_P + 1);

      gsl_vector_set(&f.vector, i, sqrt_alpha*(exp(0.1*xi) - exp(-0.1)));
    }

  /* row 2p */
  gsl_vector_set(&f.vector, penalty2_N - 1, sum - 1.0);

  if (JTJ)
    {
      for (j = 0; j < penalty2_P; ++j)
        {
          double xj = gsl_vector_get(x, j);
          double delta1j = (j == 0) ? 1.0 : 0.0;

          /* first and last rows */
          gsl_matrix_set(&J.matrix, 0, j, delta1j);
          gsl_matrix_set(&J.matrix, penalty2_N - 1, j, 2.0 * (penalty2_P - j) * xj);

          /* rows [2:p] */
          for (i = 1; i < penalty2_P; ++i)
            {
              double xi = gsl_vector_get(x, i);
              double xim1 = gsl_vector_get(x, i - 1);
              double Jij;

              if (i == j)
                Jij = exp(0.1 * xi);
              else if (i - 1 == j)
                Jij = exp(0.1 * xim1);
              else
                Jij = 0.0;

              Jij *= 0.1 * sqrt_alpha;

              gsl_matrix_set(&J.matrix, i, j, Jij);
            }

          /* rows [p+1:2p-1] */
          for (i = penalty2_P; i < penalty2_N - 1; ++i)
            {
              double xi = gsl_vector_get(x, i - penalty2_P + 1);

              if (i - penalty2_P + 1 == j)
                gsl_matrix_set(&J.matrix, i, j, 0.1 * sqrt_alpha * exp(0.1*xi));
              else
                gsl_matrix_set(&J.matrix, i, j, 0.0);
            }
        }
    }

  *normf = gsl_blas_dnrm2(&f.vector);

  if (JTJ)
    {
      gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, &J.matrix, 0.0, JTJ);
      gsl_blas_dgemv(CblasTrans, 1.0, &J.matrix, &f.vector, 0.0, JTf);
    }

  (void)params; /* avoid unused parameter warning */

  return GSL_SUCCESS;
}

static gsl_multilarge_function_fdf penalty2_func =
{
  &penalty2_fdf,
  penalty2_P,
  NULL,
  0,
  0
};

static test_fdf_problem penalty2_problem =
{
  "penalty2",
  penalty2_x0,
  NULL,
  &penalty2_epsrel,
  penalty2_NTRIES,
  &penalty2_checksol,
  &penalty2_func
};
