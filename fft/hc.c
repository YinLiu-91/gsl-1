#include <config.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <gsl_errno.h>
#include <gsl_complex.h>

#include <gsl_fft_halfcomplex.h>

#define BASE_DOUBLE
#include "templates_on.h"
#include "hc_pass.h"
#include "hc_init.c"
#include "hc_main.c"
#include "hc_pass_2.c"
#include "hc_pass_3.c"
#include "hc_pass_4.c"
#include "hc_pass_5.c"
#include "hc_pass_n.c"

#include "bitreverse.c"
#include "hc_radix2.c"

#include "templates_off.h"
#undef  BASE_DOUBLE
