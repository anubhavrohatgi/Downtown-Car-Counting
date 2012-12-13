#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

#include "cvblob.h"
#include "CarCounter.h"
#include "CarCountManager.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/flann/flann_base.hpp"

/* time example */
#include <stdio.h>
#include <time.h>

#include <stdio.h>

using namespace cv;

using namespace std;
using namespace cvb;

#include <gsl/gsl_multifit.h>
//#include "polifitgsl.h"

bool polynomialfit(int obs, int degree,
           double *dx, double *dy, double *dz, double *store) /* n, p */
{
  gsl_multifit_linear_workspace *ws;
  gsl_matrix *cov, *X;
  gsl_vector *y, *c, *r;
  double chisq;

  int i, j;

  X = gsl_matrix_alloc(obs, degree);
  y = gsl_vector_alloc(obs);
  r = gsl_vector_alloc(obs);
  c = gsl_vector_alloc(degree);
  cov = gsl_matrix_alloc(degree, degree);


  for (int i = 0; i < obs; i++) {
      gsl_matrix_set(X, i, 0, dx[i] * 50);
      gsl_matrix_set(X, i, 1, dy[i]);
      gsl_vector_set(y, i, dz[i]);
  }

  ws = gsl_multifit_linear_alloc(obs, degree);
  gsl_multifit_linear(X, y, c, cov, &chisq, ws);

  printf("chisq %f\n", chisq);
  /* store result ... */
  for(i=0; i < degree; i++)
  {
    store[i] = gsl_vector_get(c, i);
  }

  for (i = 0; i < degree; i++) {
      for (j = 0; j < degree; j++) {
          printf("%f ", gsl_matrix_get(cov, i, j));
      }
      printf("\n");
  }

  printf("RES\n");
  //gsl_multi
  //gsl_multifit_linear_residuals(X, y, c, r);

  for(i=0; i < obs; i++)
  {
    printf("%lf\n", gsl_vector_get(r, i));
  }



  gsl_multifit_linear_free(ws);
  gsl_matrix_free(X);
  gsl_matrix_free(cov);
  gsl_vector_free(y);
  gsl_vector_free(c);
  return true; /* we do not "analyse" the result (cov matrix mainly)
          to know if the fit is "good" */
}
#if 0
431,330.593645,359.183946,598,2
432,336.231481,360.867284,324,2
432,379.949398,373.438554,415,2
433,390.692045,372.298864,880,2
434,402.229122,369.972163,467,2
434,431.929185,392.457082,466,2
435,427.905128,378.479487,390,2
435,470.024000,396.364000,250,2
435,434.385246,405.418033,122,2
436,500.928433,400.235977,517,2
437,523.901208,401.950249,1407,2
439,578.373541,410.184436,1285,2
439,572.797508,435.386293,321,2
440,598.539474,413.000000,380,2
#endif
int mai333n(int argc, char* argv[])
{
#if 0
    #define NP 14
    double x[] = {330,  336,  379,  390,  402,  431,  427,   470,   434,   500,   523, 578, 572, 598};
    double y[] = {359,  360,  373,  372,  369,  392,  378,   396,   405,   400,   401, 410, 435, 413};
    double t[] = {431,  432,  432,  433,  434,  434,  435,   435,   435,   436,   437, 439, 439, 440};
#endif
#define NP 10
double x[] = {1,  2,  3,  4,  5,  6,   7,   8,   9,   10};
double y[] = {1,  2,  3,  4,  5,  6,   7,   8,   9,   10};
double t[] = {1,  2,  3,  4,  5,  6,   7,   8,   9,   10};

for (int i = 0; i < NP; i++) {
    t[i] = x[i] + y[i];
}

    #define DEGREE 2
    double coeff[DEGREE];
    double err[DEGREE];

  int i;

  polynomialfit(NP, DEGREE, x, y, t, coeff);
  printf("\nCOEFF\n");
  for(i=0; i < DEGREE; i++) {
    printf("%lf\n", coeff[i]);
  }
  return 0;
}

int few()
{
    int i, n, p;
    double xi, yi,ei, chisq;

    gsl_matrix *X, *cov;
    gsl_vector *y, *w, *c;

    p = 3; //we just know this based on the example above
    n = 5; //we just know this based on the example above

    // allocate space for the matrices and vectors
    X = gsl_matrix_alloc(n, p); // this is an input
    y = gsl_vector_alloc(n); //this is an input

    c = gsl_vector_alloc(p); //this is an output
    cov = gsl_matrix_alloc(p, p); //this is an output

    //now put the data into the X matrix, row by row
    gsl_matrix_set(X, 0, 0, 1);
    gsl_matrix_set(X, 0, 1, 25);
    gsl_matrix_set(X, 0, 2, 72); //first row, i.e., Joe, done

    gsl_matrix_set(X, 1, 0, 0);
    gsl_matrix_set(X, 1, 1, 32);
    gsl_matrix_set(X, 1, 2, 68); // Jill, done

    gsl_matrix_set(X, 2, 0, 1);
    gsl_matrix_set(X, 2, 1, 27);
    gsl_matrix_set(X, 2, 2, 69); // Jack done

    gsl_matrix_set(X, 3, 0, 1);
    gsl_matrix_set(X, 3, 1, 45);
    gsl_matrix_set(X, 3, 2, 67); // John done

    gsl_matrix_set(X, 4, 0, 0);
    gsl_matrix_set(X, 4, 1, 38);
    gsl_matrix_set(X, 4, 2, 62); // Jane done

    //now enter the dependent data, y, representing weight

    gsl_vector_set(y, 0, 178); // Joe's weight
    gsl_vector_set(y, 1, 122); // Jill's weight
    gsl_vector_set(y, 2, 167); // Jack
    gsl_vector_set(y, 3, 210); // John
    gsl_vector_set(y, 4, 108); // Jane

    //now make some calls to gsl

    // allocate temporary work space for gsl
    gsl_multifit_linear_workspace * work;
    work = gsl_multifit_linear_alloc(n, p);
int j;
    // now do the fit
    gsl_multifit_linear (X, y, c, cov, &chisq, work);

    printf("FATSO chisq %f\n", chisq);
    /* store result ... */
    for(i=0; i < 3; i++)
    {
      printf("%lf\n", gsl_vector_get(c, i));
    }
printf("\n\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("%f ", gsl_matrix_get(cov, i, j));
        }
        printf("\n");
    }

    /* some notes are in order here:
    X and y are inputs, c, cov, and chisq are outputs
    c is a p-length vector with the parameters that minimized the prediction error
    cov is a pxp matrix with a covariance matrix for the independent variable
    of particular note in cov is the diagonal element (i,i), which is the variance of
      the ith independent variable. In our example, the cov(1,1) is the variance of the ages
    chisq is the sum-of-squared error for this model (lower is better)
    work is merely a workspace that we provide to gsl
    */

    //now do what ever we want with the results
    //most likely we will dump results to a text file

    //************************************
    //very important stuff at the end here
    //************************************
    // don't forget to deallocate - there is no garbage collector in C!

    gsl_matrix_free(X);
    gsl_matrix_free(cov);
    gsl_vector_free(y);
    gsl_vector_free(c);
}
