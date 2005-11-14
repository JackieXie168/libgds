// ==================================================================
// @(#)rand.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 19/08/2003
// @lastdate 20/08/2003
// ==================================================================

#ifndef __GDS_RAND_H__
#define __GDS_RAND_H__

// ----- rand_uniform -----------------------------------------------
extern double rand_uniform(double dRange);
// ----- rand_uniform_range -----------------------------------------
extern double rand_uniform_range(double dMin, double dMax);
// ----- rand_exponential -------------------------------------------
extern double rand_exponential(double dLambda);
// ----- rand_normal ------------------------------------------------
extern double rand_normal(double dMean, double dDeviate);
// ----- rand_pareto ------------------------------------------------
extern double rand_pareto(double dShape, double dScale);
// ----- rand_log_normal --------------------------------------------
extern double rand_log_normal(double dMean, double dDeviate);

#endif
