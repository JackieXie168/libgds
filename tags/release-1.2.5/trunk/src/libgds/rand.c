// ==================================================================
// @(#)rand.c
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 19/08/2003
// @lastdate 20/08/2003
// ==================================================================
// Sources: Numerical Recipes in FORTRAN

#include <assert.h>
#include <math.h>
#include <libgds/rand.h>
#include <stdlib.h>
#include <stdio.h>

// ----- rand_uniform -----------------------------------------------
/**
 * range is in [0, dRange]
 */
double rand_uniform(double dRange)
{
  return dRange*rand()/RAND_MAX;
}

// ----- rand_uniform_range -----------------------------------------
/**
 * range is in [dMin, dMax]
 */
double rand_uniform_range(double dMin, double dMax)
{
  return dMin+rand_uniform(dMax-dMin);
}

// ----- rand_exponential -------------------------------------------
/**
 *
 */
double rand_exponential(double dLambda)
{
  double dRandom;
  assert(dLambda > 0);
  while ((dRandom= rand_uniform(1.0)) == 0.0);
  return -log(dRandom)/dLambda;
}

// ----- rand_normal ------------------------------------------------
/**
 *
 */
double rand_normal(double dMean, double dDeviate)
{
  static int iParity= 0;
  static double dNextResult;
  double dSample1, dSample2, dRadius;

  if (dDeviate == 0.0)
    return dMean;
  if (iParity == 0) {
    iParity= 1;
    // Pick two numbers in the square (-1,-1)-(1,1),
    // check that they are in the unit circle (try again until they are)
    do {
      dSample1= 2.0 * rand_uniform(1.0) - 1;
      dSample2= 2.0 * rand_uniform(1.0) - 1;
      dRadius= dSample1 * dSample1 + dSample2 * dSample2;
    } while ((dRadius >= 1.0) || (dRadius == 0.0));
    dRadius= sqrt((-2*log(dRadius))/dRadius);
    dNextResult= dSample2 * dRadius;
    return (dSample1 * dRadius * dDeviate + dMean);
  } else {
    iParity= 0;
    return (dNextResult * dDeviate + dMean);
  }
}

// ----- rand_pareto ------------------------------------------------
/**
 *
 */
double rand_pareto(double dShape, double dScale)
{
  double dRandom;
  assert(dShape > 0);
  dRandom= rand_uniform(1.0);
  return 1.0/(pow(1.0-dRandom+dRandom*pow(1.0/dScale, dShape), 1.0/dShape));
}

// ----- rand_log_normal --------------------------------------------
/**
 *
 */
double rand_log_normal(double dMean, double dDeviate)
{
  return exp(rand_normal(dMean, dDeviate));
}

