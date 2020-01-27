#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <float.h>
#define is_sane(a) (!_isnan((a)) && _finite((a)))


/*
 * Local macros used to verify/sanity check the parameters
 * we receive from Excel. We don't want to call our functions
 * under all circumstances, so this is error checking with excel specific
 * reporting.
 */
#define VOLATILITY_MIN 0.0000001
#define VOLATILITY_MAX 100.99999

/* 
 * There are approx. 253 trading days per year. We allow for less time than one 
 * day to avoid rounding errors. Less than an hour is meaningless.
 * Max time is 20 (trading) years. Remember that t==1.0 == one trading year
 */
#define TIME_MIN (1.0/(253.0 * 24.0)) 
#define TIME_MAX (20.0)

/* 
 * Some sanity checks for interest rate. We cannot have negative rates
 * as that screws up the entire economic system. Not so sure about max values
 * because from time to time one gets insane rates. We set some kind of max anyway
 */
#define INTEREST_RATE_MIN 0.0 /* Zero is OK and happens from time to time, e.g. Japan */
#define INTEREST_RATE_MAX 1000.0 /* 100000% yearly is quite a lot */

/*
 * Strike and price min/max are equal. Nothing is free, so PRICE_MIN > 0.0
 * There's no theoretical upper bound for price, but we want to avoid infinity
 * and set a max price anyway.
 */
#define PRICE_MIN 0.000001
#define PRICE_MAX 1000000000000.0

#define STRIKE_MIN PRICE_MIN
#define STRIKE_MAX PRICE_MAX

/* 
 * Cost of carry is defined here.
 * I guess we could apply rules like COC <= INTEREST_RATE as noone would
 * ever buy something with a cost of carry > risk free rate. 
 * What about negative costs? Happens when underlying pays a yield greater
 * than risk free rate. 
 */
#define COST_OF_CARRY_MIN	-INTEREST_RATE_MAX
#define COST_OF_CARRY_MAX	INTEREST_RATE_MAX

/*
 * Error checking macros, used to aid users of the library.
 * We define our own is_sane() macro which tests a double as much as possible.
 * Some platforms and standards have *very* poor support for this, namely C89.
 * This is the main reason we use our own function.
 */
#define assert_valid_price(S)			assert(is_sane(S) && (S) >= PRICE_MIN && (S) <= PRICE_MAX)
#define assert_valid_strike(X)			assert(is_sane(X) && (X) >= STRIKE_MIN && (X) <= STRIKE_MAX)
#define assert_valid_time(T)			assert(is_sane(T) && (T) >= TIME_MIN && (T) <= TIME_MAX)
#define assert_valid_interest_rate(r)	assert(is_sane(r) && (r) >= INTEREST_RATE_MIN && (r) <= INTEREST_RATE_MAX)
#define assert_valid_cost_of_carry(b)	assert(is_sane(b) && (b) >= COST_OF_CARRY_MIN && (b) <= COST_OF_CARRY_MAX)
#define assert_valid_volatility(v)		assert(is_sane(v) && (v) >= VOLATILITY_MIN && v <= VOLATILITY_MAX)

/**
 * Some constants we use a lot.
 * The M_E and friends from math.h is not a part of the ANSI C standard,
 * so we add them here instead.
 * e, pi and sqrt(2*pi);
 * A common calculation is (1/sqrt2pi) *e, which equals e/sqr2pi.
 *
 * Use the gcc flag -Wshadow to locate scoping problems
 */
static const double e = 2.7182818284590452354;
static const double pi = 3.14159265358979323846;	
static const double sqrt2pi = 2.50662827463100024161;
static const double e_div_sqrt2pi = 1.08443755141922748564;
static const double one_div_sqrt2pi = 0.39894228040143270286;

double pow2(double n) { return n * n; }
double normdist(double x) { return one_div_sqrt2pi * exp(-((x * x)/ 2.0)); }

/*
 * Parameter rules, naming conventions:
 * X - Strike
 * S - Stock price right now
 * T - Time to expiry as fraction of year. 6 months == 0.5, 9 months == 0.75
 * r - Risk free interest rate. 10% == 0.10
 * b - Cost of Carry. 10% == 0.10
 * v - Volatility, 30% == 0.30
 */

/* Cumulative normal distribution */
double cnd(double x)
{
	static const double 
		a1 = +0.31938153,
		a2 = -0.356563782,
		a3 = +1.781477937,
		a4 = -1.821255978,
		a5 = +1.330274429;

	const double L = fabs(x);
	const double K = 1.0 / (1.0 + (0.2316419 * L));
	const double a12345k 
		= (a1 * K)
		+ (a2 * K * K) 
		+ (a3 * K * K * K) 
		+ (a4 * K * K * K * K) 
		+ (a5 * K * K * K * K * K); 

	double result = 1.0 - one_div_sqrt2pi * exp(-pow2(L) / 2.0) * a12345k;
		
	assert(is_sane(x));
	if(x < 0.0) 
		result = 1.0 - result;

	assert(is_sane(x));
	return result;
}

/* European options */
/* Black and Scholes (1973) Stock options */
double blackscholes(int fCall, double S, double X, double T, double r, double v) 
{
	double vst, d1, d2;

	assert_valid_price(S);
	assert_valid_strike(X);
	assert_valid_time(T);
	assert_valid_interest_rate(r);
	assert_valid_volatility(v);

	assert(r >= 0.0);			/* Interest rate >= 0.0 */
	assert(v > 0.0 && v <= 100.0);	/* Volatility between 0 and 100 */
    
	vst = v * sqrt(T);
    d1 = (log(S / X) + (r + pow2(v) / 2.0) * T) / (vst);
    d2 = d1 - vst;
    if(fCall) 
        return S * cnd(d1) - X * exp(-r * T) * cnd(d2);
    else 
        return X * exp(-r * T) * cnd(-d2) - S * cnd(-d1);
}

// GBS 

double gbs(
	int fCall,
	double S,
	double X,
	double T,
	double r,
	double b,
	double v) 
{
	double vst, d1, d2, ebrt, ert, result;

	assert_valid_price(S);
	assert_valid_strike(X);
	assert_valid_time(T);
	assert_valid_interest_rate(r);
	assert_valid_cost_of_carry(b);
	assert_valid_volatility(v);

	vst = v * sqrt(T);
	assert(is_sane(vst));
    d1 = (log(S / X) + (b + pow2(v) / 2.0) * T) / vst;
    d2 = d1 - vst;
	ebrt = exp((b - r) * T);
	ert = exp(-r * T);

    if(fCall)
        result 
			= S * ebrt * cnd(d1) 
			- X * ert  * cnd(d2);
    else 
        result 
			= X * ert  * cnd(-d2) 
			- S * ebrt * cnd(-d1);

	assert(is_sane(result));
	return result;
}



// American Option

static double
phi(double S, double T, double gamma_val, double H, double I, double r, double b, double v) 
{
	double vst, vv, lambda, d, kappa;

	assert_valid_price(S);
	assert_valid_time(T);
	assert_valid_interest_rate(r);
	assert_valid_volatility(v);

	vst = v * sqrt(T);
	vv = v * v;

    lambda = (-r + gamma_val * b + 0.5 * gamma_val * (gamma_val - 1.0) * vv) * T;
    d = -(log(S / H) + (b + (gamma_val - 0.5) * vv) * T) / vst;
    kappa = 2.0 * b / vv + (2.0 * gamma_val - 1.0);

    return exp(lambda) 
		* pow(S, gamma_val)
		* (cnd(d) - pow(I / S, kappa) * cnd(d - 2.0 * log(I / S) / vst));
}

double BSAmericanCallApprox(double S, double X, double T, double r, double b, double v) 
{
	assert_valid_price(S);
	assert_valid_strike(X);
	assert_valid_time(T);
	assert_valid_interest_rate(r);
	assert_valid_volatility(v);

    if(b >= r ) {
		/* Never optimal to exercise before maturity */
		return gbs(1, S, X, T, r, b, v);
	}
    else {
		double vv, Beta, BInfinity, B0, ht, I;
		
		vv = v*v;
		assert(is_sane(vv));

        Beta = (0.5 - b / vv) + sqrt(pow2(b / vv - 0.5) + 2.0 * r / vv);
		assert(is_sane(Beta));

        BInfinity = Beta / (Beta - 1.0) * X;
		assert(is_sane(BInfinity));

        B0 = fmax(X, r / (r - b) * X);
		assert(is_sane(B0));

        ht = -(b * T + 2.0 * v * sqrt(T)) * B0 / (BInfinity - B0);
		assert(is_sane(ht));

        I = B0 + (BInfinity - B0) * (1.0 - exp(ht));
		assert(is_sane(I));

        if(S >= I )
            return S - X;
        else {
			const double alpha = (I - X) * pow(I, -Beta);

            return alpha * pow(S, Beta)
				- alpha * phi(S, T, Beta, I, I, r, b, v) 
				+ phi(S, T, 1.0,  I, I, r, b, v) 
				- phi(S, T, 1.0,  X, I, r, b, v) 
				- X * phi(S, T, 0.0,  I, I, r, b, v) 
				+ X * phi(S, T, 0.0,  X, I, r, b, v);
		}
    }
}

double BSAmericanApprox(int fCall, double S, double X, double T, double r, double b, double v) 
{
	double result;

	assert_valid_price(S);
	assert_valid_strike(X);
	assert_valid_time(T);
	assert_valid_interest_rate(r);
	assert_valid_volatility(v);

    if(fCall)
        result = BSAmericanCallApprox(S, X, T, r, b, v);
    else {
		/* Use the Bjerksund and Stensland put-call transformation */
        result = BSAmericanCallApprox(X, S, T, r - b, -b, v);
	}
    
	assert(is_sane(result));
	return result;
}

