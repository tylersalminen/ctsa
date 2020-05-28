/*
 * talg.c
 *
 *  Created on: Jul 31, 2013
 *      Author: USER
 */

#include "talg.h"

void detrend_ma(double *sig,int N, int window,double *oup) {
	// Uses moving average filter to detrend the signal

	mafilter(sig,N,window,oup);

	// other options are
	// 1.  mafilter2(sig,N,window,oup)
	// 2.  expfilter(sig,N,window,oup)
	// 3. mafilter_wt(sig,N,weights,window,oup) [See the code in filter.c]
	// For more information, please consult
	//Time Series: Theory and Methods Second Edition
	// by Peter J. Brockwell ,Richard A. Davis
	// Chapter 1
}

int poly(double *A, double *B, double *C, int lA, int lB) {
	int lC,i,j,k;
	double temp;
	lC = lA + lB - 1;
	
	for(i = 0; i < lC;++i) {
			C[i] = 0.;
	}
	
	for(i = 0; i < lC;++i) {
		temp = 0.0;
		for(j = 0; j < lA;++j) {
			for(k = 0; k < lB;++k) {
				if (j + k == i) {
					temp += A[j] * B[k];
				}
			}
		}
		C[i] = temp;
			
	}
	
		
	return lC;
}

int upsample(double *x,int lenx, int M, double *y) {
	int N,i,j,k;

	if (M < 0) {
		return -1;
	}

	if (M == 0) {
		for (i = 0; i < lenx; ++i) {
			y[i] = x[i];
		}
		return lenx;
	}

	N = M * (lenx-1) + 1;
	j = 1;
	k = 0;

	for(i = 0; i < N;++i) {
		j--;
		y[i] = 0.0;
		if (j == 0) {
			y[i] = x[k];
			k++;
			j = M;
		}
	}

	return N;
}

int downsample(double *x,int lenx, int M, double *y) {
	int N,i;

	if (M < 0) {
		return -1;
	}
	if (M == 0) {
		for (i = 0; i < lenx; ++i) {
			y[i] = x[i];
		}
		return lenx;
	}

	N = (lenx-1)/M + 1;

	for (i = 0;i < N;++i) {
		y[i] = x[i*M];
	}

	return N;
}

void deld(int d, double* C) {
	int i,j;
	double *vec,*oup;
	/*
	 * deld returns coefficients of [1,-1]^d
	 * eg.,
	 * d=2 yields [1,-2,1]
	 * d=3 [1,-3,3,1]
	 */ 
	vec = (double*) malloc(sizeof(double) * 2);
	oup = (double*) malloc(sizeof(double) * (d+1));
	vec[0] = 1.;
	vec[1] = -1.;
	oup[0] = 1.;
	// oup has length (d+1)
	
	for(i = 0; i < d; ++i) {
		poly(oup,vec,C,i+1,2);
		for(j = 0; j < i+2; ++j) {
			oup[j] = C[j];
		}
		
	}
	free(vec);
	free(oup);
}

void delds(int D, int s, double *C) {
	int i,j;
	double *vec,*oup;
	/*
	 * delds returns coefficients of [1,....(s-1) zeroes,.. -1]^D
	 * eg., D =1 s = 4 yields
	 * [1,0,0,0,-1]
	 * D=2 s=4 -> [1,0,0,0,-2,0,0,0,1]
	 */ 
	/*
	 * s - seasonal component
	 * s = 4 (quarterly data), 12 (yearly data) etc.
	 * D - Differencing operator. D >= 1 when seasonal component is present
	 * D is usually 1 when seasonal component is present. D = 0 means no
	 * seasonal component
	 */ 
	vec = (double*) malloc(sizeof(double) * (s+1));
	oup = (double*) malloc(sizeof(double) * (D*s + 1));
	for(i = 0; i < s+1; ++i) {
		vec[i] = 0.;
	}
	vec[0] = 1.0;
	vec[s] = -1.0;
	oup[0] = 1.0;
	
	for(i = 0; i < D; ++i) {
		poly(oup,vec,C,i*s+1,s+1);
		for(j = 0; j < s*(i+1)+1; ++j) {
			oup[j] = C[j];
		}
		
	}
	free(vec);
	free(oup);
}

int diff(double *sig, int N, int d, double *oup) {
	int Noup,i,j;
	double *coeff;
	double sum;
	/*
	 * 
	 * diff output = [ X(t) - X(t-1) ] ^ d where X(t) is the
	 * input timeseries of length N
	 * output is of length N - d
	 * 
	 * diff is used to detrend data
	 * 
	 * d=1 typically takes care of linear trend while
	 * d=2 handles quadratic trend
	 */ 
	
	coeff = (double*) malloc(sizeof(double) * (d+1));
	deld(d,coeff);
	Noup = N - d;
	
	for(i = d; i < N;++i) {
		sum = 0.;
		for(j = 1; j < d+1;++j) {
			sum += sig[i-j]*coeff[j];
		}
		oup[i-d] = sum + coeff[0] * sig[i];
		
	}

	free(coeff);
	
	
	return Noup;
}

int diffs(double *sig, int N, int D,int s, double *oup) {
	int Noup,i,j,d;
	double *coeff;
	double sum;
	/*
	 * diffs output = [ X(t) - X(t-s) ] ^ D where X(t) is the
	 * input timeseries of length N
	 * output is of length N - D*s
	 * 
	 * 
	 * s - seasonal component, D = 1 when seasonal component is present and 0
	 * when it is absent. D >= 2 is rarely needed and in most cases you may want
	 *  to re-analyze data if D >= 2 is needed.
	 * s = 4 (quarterly data), 12 (yearly data) etc.
	 * 
	 * D - Differencing operator. D >= 1 when seasonal component is present
	 * D is usually 1 when you are differencing [ X(t) - X(t-s) ]
	 * D = 2 yields [ X(t) - X(t-s) ] * [ X(t) - X(t-s) ]
	 * see Chapter 9 of Box Jenkins for detrending and deseasoning seasonal data
	 * with trends 
	 */ 
	
	d = D*s;
	coeff = (double*) malloc(sizeof(double) * (d+1));
	delds(D,s,coeff);
	Noup = N - d;
	
	for(i = d; i < N;++i) {
		sum = 0.;
		for(j = s; j < d+1;j+=s) {
			sum += sig[i-j]*coeff[j];
		}
		oup[i-d] = sum + coeff[0] * sig[i];
		
	}
	
	free(coeff);
	
	return Noup;
}

void deseason_ma(double *sig,int N,int s,double *oup) {
	double *mt,*w,*seas;
	int k,window,q,odd,jd,count,per,it;
	double temp,wi;
	
	window = s;
	mt = (double*) malloc(sizeof(double) * N);	
	w = (double*) malloc(sizeof(double) * window);	
	seas = (double*) malloc(sizeof(double) * window);
	
	mafilter2(sig,N,window,mt);
	odd = window - ((window/2) * 2);
	
	if (odd) {
		q = (window - 1 ) / 2; 
	} else {
		q = window / 2;
	}
	
	for(k = 0; k < window;++k) {
		jd = k;
		count = 0;
		temp = 0.0;
		while (jd < N - 2*q) {
			temp = temp + sig[q+jd]-mt[q+jd];
			count++;
			jd+=window;
		}
		per = k + q -((k+q)/window)*window;
		w[per] = temp/count;
	}
	
	temp = 0.0;
	for(k = 0; k < window;++k)  {
		temp += w[k];
	}
	wi = temp/window;
	
	for(k = 0; k < window;++k)  {
		seas[k] = w[k] - wi;
	}
	
	for(k = 0; k < N;++k) {
		it = k;
		while (it >= window) {
			it -= window;
		}
		oup[k] = sig[k] - seas[it];
	}
	
	free(w);
	free(seas);
	free(mt);
}

void psiweight(double *phi,double *theta,double *psi,int p,int q,int j) {
	int i,k;
	double temp;
	double *th;
	psi[0] = 1.0;
	th = (double*) malloc(sizeof(double) * (q+1));	
	th[0] = 1.;
	for(i = 0; i < q;++i) {
		th[i+1] = theta[i];
	}
	
	for(i = 1; i < j;++i) {
		psi[i] = 0.0;
		temp = 0.0;
		if(i <= q) {
			psi[i] = th[i];
		}
		for(k = 1; k < p+1;++k) {
			if((i - k) >= 0) {
				temp+=phi[k-1] * psi[i-k];
			} 
			
		}
		psi[i] += temp;
	}
	
	free(th);
}

void piweight(double *phi,double *theta,double *piw,int p,int q,int j) {
	int i,k;
	double temp;
	double *ph;
	piw[0] = 1.0;
	ph = (double*) malloc(sizeof(double) * (p+1));	
	ph[0] = -1.;
	for(i = 0; i < p;++i) {
		ph[i+1] = phi[i];
	}
	
	for(i = 1; i < j;++i) {
		piw[i] = 0.0;
		temp = 0.0;
		if(i <= p) {
			piw[i] = -ph[i];
		}
		for(k = 1; k < q+1;++k) {
			if((i - k) >= 0) {
				temp+=theta[k-1] * piw[i-k];
			} 
			
		}
		piw[i] -= temp;
	}
	
	free(ph);
	
}

void arma_autocovar(double *phi,double *theta,int p,int q,double var,double* acov, int lag) {
	int i,j,t,m;
	double *tcov,*psi,*A,*b,*ph,*th,*thph;
	int *ipiv;
	int p1;
	double temp;
	
	p1 = p+1;
	tcov = (double*) malloc(sizeof(double) * (p1));
	ipiv = (int*) malloc(sizeof(int) * (p1));
	A = (double*) malloc(sizeof(double) * (p1) * (p1));
	b = (double*) malloc(sizeof(double) * (p1));
	ph = (double*) malloc(sizeof(double) * (p1));
	psi = (double*) malloc(sizeof(double) * (q+1));
	th = (double*) malloc(sizeof(double) * (q+1));

	ph[0] = -1.0;
	th[0] = 1.0;
	
	for(i = 0; i < p;++i) {
		ph[i+1] = phi[i];
	}
	
	for(i = 0; i < q;++i) {
		th[i+1] = theta[i];
	}
	
	if (p >= q+1) {
		m = p;
	} else {
		m = q + 1;
	}
	thph = (double*) malloc(sizeof(double) * m);
	// set A
	for(i = 0; i < p1;++i) {
		for(j=0; j < p1;++j) {
			A[i*p1+j] = 0.;
			if (i == j && i != 0) {
				A[i*p1+j] = 1.;
			}

			t = i - j;
			if (t < 0) {
				t = -t;
			} 
			
			A[i*p1+t] -= ph[j];
			
		}
	}
	
	//set b
	
	psiweight(phi,theta,psi,p,q,q+1);
	for(i=0; i < m;++i) {
		
		temp = 0.;
		for(j = 0; j < q+1;++j) {
			if(i+j < q+1) {
				temp += th[i+j] * psi[j];
			}
		}
		thph[i] = temp*var;
		
	}
	
	for(i=0; i < p1;++i) {
		b[i] = thph[i];
	}
	
	ludecomp(A,p1,ipiv);
	linsolve(A,p1,b,ipiv,tcov);
	
	for(i = 0; i < p1;++i) {
		acov[i] = tcov[i];
	}
	
	for(i = p1; i < lag;++i) {
		temp = 0.0;
		for(j = 1; j < p1; ++j) {
			temp+= phi[j-1] * acov[i - j];
		}
		if (i < m) {
			temp += thph[i];
		}
		acov[i] = temp;
	}
	
	free(ph);
	free(th);
	free(thph);
	free(psi);
	free(tcov);
	free(A);
	free(ipiv);
	free(b);
}

int twacf(double *P, int MP, double *Q, int MQ, double *ACF, int MA, double *CVLI, int MXPQ1, double *ALPHA, int MXPQ) {
	int ifault, i, k, kc, j, jpk, kcp1mj, j1, kp1, kp2mj, miim1p, imj, mikp, kp1mj;
	double epsil2, zero, one, half, two, div;

	ifault = 0;
	epsil2 = 1.0e-10;
	zero = 0.0; half = 0.5, one = 1.0, two = 2.0;

	if (MP < 0 || MQ < 0) {
		ifault = 1;
	}
	if (MXPQ != imax(MP, MQ)) {
		ifault = 2;
	}
	if (MXPQ1 != MXPQ + 1) {
		ifault = 3;
	}
	if (MA < MXPQ1) {
		ifault = 4;
	}

	if (ifault > 0) {
		return ifault;
	}

	// Initialization and return if MP = MQ = 0

	ACF[0] = one;
	CVLI[0] = one;

	if (MA == 1) {
		return ifault;
	}

	for (i = 1; i < MA; ++i) {
		ACF[i] = zero;
	}

	if (MXPQ1 == 1) {
		return ifault;
	}

	for (i = 1; i < MXPQ1; ++i) {
		CVLI[i] = zero;
	}
	for (k = 0; k < MXPQ; ++k) {
		ALPHA[k] = 0.0;
	}

	// Computation of the A.C.F. of the moving average part stored in ACF

	if (MQ != 0) {
		for (k = 1; k <= MQ; ++k) {
			CVLI[k] = -Q[k - 1];
			ACF[k] = -Q[k - 1];
			kc = MQ - k;
			if (kc != 0) {
				for (j = 1; j <= kc; ++j) {
					jpk = j + k;
					ACF[k] += (Q[j - 1] * Q[jpk - 1]);
				}
			}//120
			ACF[0] += (Q[k - 1] * Q[k - 1]);
		}

		//Initialization of CVLI = T.W.-S.PHI -- return if MP = 0
	}//180

	if (MP == 0) {
		return ifault;
	}

	for (k = 0; k < MP; ++k) {
		ALPHA[k] = P[k];
		CVLI[k] = P[k];
	}

	// Computation of T.W.-S ALPHA and DELTA
	// DELTA stored in ACF which is gradually overwritten

	for (k = 1; k <= MXPQ; ++k) {
		kc = MXPQ - k;
		if (kc < MP) {
			div = one - ALPHA[kc] * ALPHA[kc];
			if (div <= epsil2) {
				return 5;
			}
			if (kc == 0) {
				break; //break For loop. Go to 290
			}//290
			for (j = 1; j <= kc; ++j) {
				kcp1mj = kc - j;
				ALPHA[j - 1] = (CVLI[j - 1] + ALPHA[kc] * CVLI[kcp1mj]) / div;
			}
		}//240
		if (kc < MQ) {
			j1 = imax(kc + 1 - MP, 1);
			for (j = j1; j <= kc; ++j) {
				kcp1mj = kc - j;
				ACF[j] += ACF[kc + 1] * ALPHA[kcp1mj];
			}
		}//260
		if (kc < MP) {
			for (j = 1; j <= kc; ++j) {
				CVLI[j - 1] = ALPHA[j - 1];
			}
		}
	}//290

	// Computation of T.W.-S NU
	// NU is stored in CVLI copied into ACF

	ACF[0] *= half;
	for (k = 1; k <= MXPQ; ++k) {
		if (k <= MP) {
			kp1 = k + 1;
			div = one - ALPHA[k - 1] * ALPHA[k - 1];
			for (j = 1; j <= kp1; ++j) {
				kp2mj = k + 2 - j;
				CVLI[j - 1] = (ACF[j - 1] + ALPHA[k - 1] * ACF[kp2mj - 1]) / div;
			}
			for (j = 1; j <= kp1; ++j) {
				ACF[j - 1] = CVLI[j - 1];
			}
		}//330
	}//330

	//Computation of ACF

	for (i = 1; i <= MA; ++i) {
		miim1p = imin(i - 1, MP);
		if (miim1p != 0) {
			for (j = 1; j <= miim1p; ++j) {
				imj = i - j; 
				ACF[i - 1] += P[j - 1] * ACF[imj - 1];
			}
		}//430
	}//430

	ACF[0] *= two;

	//Computation of CVLI

	CVLI[0] = one;
	if (MQ > 0) {
		for (k = 1; k <= MQ; ++k) {
			CVLI[k] = -Q[k - 1];
			if (MP != 0) {
				mikp = imin(k, MP);
				for (j = 1; j <= mikp; ++j) {
					kp1mj = k + 1 - j;
					CVLI[k] += P[j - 1] * CVLI[kp1mj - 1];
				}
			}
		}
	}

	return ifault;
}

void artrans(int p, double *old, double *new1) {
	int j, k;
	double a;
	double *temp;

	temp = (double*)malloc(sizeof(double)* p);

	for (j = 0; j < p; ++j) {
		new1[j] = tanh(old[j]);
		temp[j] = new1[j];
	}

	for (j = 1; j < p; ++j) {
		a = new1[j];
		for (k = 0; k < j; ++k) {
			temp[k] -= a * new1[j - k - 1];
		}
		for (k = 0; k < j; ++k) {
			new1[k] = temp[k];
		}
	}

	free(temp);
}

void arinvtrans(int p, double *old, double *new1) {
	int j, k;
	double a;
	double *temp;

	temp = (double*)malloc(sizeof(double)* p);

	for (j = 0; j < p; ++j) {
		temp[j] = new1[j] = old[j];
	}

	for (j = p - 1; j > 0; --j) {
		a = new1[j];
		for (k = 0; k < j; ++k) {
			temp[k] = (new1[k] + a * new1[j - k - 1]) / (1 - a * a);
		}
		for (k = 0; k < j; ++k) {
			new1[k] = temp[k];
		}
	}

	for (j = 0; j < p; ++j) {
		new1[j] = atanh(new1[j]);
	}
	free(temp);
}

int invertroot(int q, double *ma) {
	int i, index, retval, fail, rcheck, qn, i1, j;
	double *temp, *zeror, *zeroi, *xr, *xi,*yr,*yi;
	int *ind;
	double mod,tempr,tempi;

	temp = (double*)malloc(sizeof(double)* (q + 1));
	zeror = (double*)malloc(sizeof(double)* q);
	zeroi = (double*)malloc(sizeof(double)* q);
	ind = (int*)malloc(sizeof(int)* q);

	retval = 0;
	index = -1;
	for (i = 0; i < q; ++i) {
		if (ma[i] != 0.0) {
			index = i;
		}
	}

	if (index == -1) {
		return retval;
	}

	index++;
	temp[0] = 1.0;
	for (i = 1; i <= index; ++i) {
		temp[i] = ma[i - 1];
	}

	qn = index;

	xr = (double*)malloc(sizeof(double)* (qn + 1));
	xi = (double*)malloc(sizeof(double)* (qn + 1));
	yr = (double*)malloc(sizeof(double)* (qn + 1));
	yi = (double*)malloc(sizeof(double)* (qn + 1));

	fail = polyroot(temp, qn, zeror, zeroi);

	//mdisplay(zeroi, 1, qn);

	if (fail == 1) {
		return retval;
	}

	rcheck = 0;

	for (i = 0; i < qn; ++i) {
		mod = zeror[i] * zeror[i] + zeroi[i] * zeroi[i];
		ind[i] = 0;
		if (mod < 1.0) {
			ind[i] = 1;
			rcheck++;
		}
	}

	if (rcheck == 0) {
		return retval;
	}
	else {
		retval = 1;
	}

	if (index == 1) {
		ma[0] = 1.0 / ma[0];
		for (i = 1; i < q; ++i) {
			ma[i] = 0.0;
		}
		return retval;
	}

	for (i = 0; i < index; ++i) {
		if (ind[i] == 1) {
			mod = zeror[i] * zeror[i] + zeroi[i] * zeroi[i];
			zeror[i] = zeror[i] / mod;
			zeroi[i] = -zeroi[i] / mod;
		}
	}

	//mdisplay(zeroi, 1, qn);

	xr[0] = 1.0; xi[0] = 0.0;
	yr[0] = xr[0];
	yi[0] = xi[0];

	for (i = 0; i < qn; ++i) {
		i1 = i + 1;
		mod = zeror[i] * zeror[i] + zeroi[i] * zeroi[i];
		tempr = zeror[i] / mod;
		tempi = -zeroi[i] / mod;
		xr[i1] = xi[i1] = 0.0;
		for (j = 1; j <= i1; ++j) {
			yr[j] = tempr * xr[j - 1] - tempi * xi[j - 1];
			yi[j] = tempr * xi[j - 1] + tempi * xr[j - 1];
			yr[j] = xr[j] - yr[j];
			yi[j] = xi[j] - yi[j];
		}
		for (j = 1; j <= i1; ++j) {
			xr[j] = yr[j];
			xi[j] = yi[j];
		}
		mdisplay(xr, 1, qn+1);
	}

	for (i = 0; i < qn; ++i) {
		ma[i] = xr[i+1];
	}

	for (i = qn; i < q; ++i) {
		ma[i] = 0.0;
	}

	free(zeror);
	free(zeroi);
	free(temp);
	free(ind);
	free(xr);
	free(xi);
	free(yr);
	free(yi);
	return retval;
}

void transall(int p,int q, int P, int Q, double *old, double *new1) {
	int N;

	N = 0;

	if (p != 0) {
		artrans(p, old, new1);
		N = N + p;
	}

	if (q != 0) {
		artrans(q, old + N, new1 + N);
		N = N + q;
	}

	if(P != 0) {
		artrans(P, old + N, new1 + N);
		N = N + P;
	}

	if (Q != 0) {
		artrans(Q, old + N, new1 + N);
	}
}

void invtransall(int p, int q, int P, int Q, double *old, double *new1) {
	int N;

	N = 0;

	if (p != 0) {
		arinvtrans(p, old, new1);
		N = N + p;
	}

	if (q != 0) {
		arinvtrans(q, old + N, new1 + N);
		N = N + q;
	}

	if (P != 0) {
		arinvtrans(P, old + N, new1 + N);
		N = N + P;
	}

	if (Q != 0) {
		arinvtrans(Q, old + N, new1 + N);
	}
}

double interpolate_linear(double *xin,double *yin, int N, double z) {
	int i,j,k;
	double *x,*y;
	int *pos;
	double out,ylo,yhi;

	x = (double*)malloc(sizeof(double)*N);
	y = (double*)malloc(sizeof(double)*N);
	pos = (int*)malloc(sizeof(int)*N);

	sort1d_ascending(xin,N,pos);

	for(i = 0; i < N;++i) {
		x[i] = xin[pos[i]];
		y[i] = yin[pos[i]];
	}

	ylo= y[0];
	yhi = y[N-1];

	i = 0;
	j = N - 1;

	if (z < x[0]) {
		free(x);
		free(y);
		free(pos);
		return ylo;
	}

	if (z > x[j]) {
		free(x);
		free(y);
		free(pos);
		return yhi;
	}

	while (i < j-1) {
		k = (i + j)/2;
		if (z < x[k]) {
			j = k;
		} else {
			i = k;
		}
	}

	if (z == x[j]) {
		out = y[j];
		free(x);
		free(y);
		free(pos);
		return out;
	}

	if (z == x[i]) {
		out = y[i];
		free(x);
		free(y);
		free(pos);
		return out;
	}

	out = y[i] + (y[j] - y[i])* ((z - x[i])/(x[j] - x[i]));

	free(x);
	free(y);
	free(pos);

	return out;
}

static double interpolate_linear_sorted_ascending(double *x,double *y, int N, double z) {
	/*
		Input x should be sorted in ascending order with all unique values for x. 
		Input y should have the index ordered on sorted x values.
	*/
	int i,j,k;
	double ylo,yhi;

	i = 0;
	j = N - 1;

	ylo= y[0];
	yhi = y[N-1];

	if (z < x[0]) {
		return ylo;
	}

	if (z > x[j]) {
		return yhi;
	}

	while (i < j-1) {
		k = (i + j)/2;
		if (z < x[k]) {
			j = k;
		} else {
			i = k;
		}
	}

	if (z == x[j]) {
		return y[j];
	}

	if (z == x[i]) {
		return y[i];
	}

	return y[i] + (y[j] - y[i])* ((z - x[i])/(x[j] - x[i]));
}

void linspace(double *x, int N,double xlo,double xhi) {
    int i;
    double intv;

    intv = (xhi - xlo)/(double)(N-1);
    x[0] = xlo;
    x[N-1] = xhi;

    for(i = 1; i < N-1;++i) {
        x[i] = xlo + intv * (double) i;
    }
}

void approx(double *x,double *y, int N,double *xout, double *yout,int Nout) {
    int i;
	/* x and y should be sorted in ascending order. x has all unique values
		xout contains equally spaced Nout values from lowest x[0]
		to highest x[N-1].Outputs y[out] are Nout interpolated values.
	*/

    for(i = 0; i < Nout;++i) {
        if (xout[i] == xout[i]) {
            yout[i] = interpolate_linear_sorted_ascending(x,y,N,xout[i]);
        } else {
            yout[i] = xout[i];
        }
    }

}

void arrayminmax(double *x, int N, double *amin,double *amax) {
	int i;
	*amax = - DBL_MAX;
	*amin = DBL_MAX;

	for(i = 0; i < N;++i) {
		*amax = x[i] > *amax ? x[i] : *amax;
		*amin = x[i] < *amin ? x[i] : *amin;
	}

}

void cumsum(double *x, int N, double *csum) {
	int i;

	double sum = 0.0;

	for (i = 0; i < N;++i) {
		sum += x[i];
		csum[i] = sum;
	}
}

void ppsum(double* u, int n, int l, double* sum)
{
	/* Copyright (C) 1997-2000  Adrian Trapletti 
	efficient computation of the sums involved in the Phillips-Perron tests */
  int i, j;
  double tmp1, tmp2;

  printf("%d %d %g \n ",n,l,*sum);
  
  tmp1 = 0.0;
  for (i=1; i<= l; i++)
  {
    tmp2 = 0.0;
    for (j=i; j< n; j++)  
    {
      tmp2 += (u[j]*u[j-i]);
    }
    tmp2 *= 1.0-((double)i/((double) l +1.0));
    tmp1 += tmp2;
  }
  tmp1 /= (double) n;
  tmp1 *= 2.0;
  (*sum) += tmp1;
}

/*

STL routines converted to C using f2c and then modified to integrate with
the rest of the code.

c     
c     from netlib/a/stl: no authorship nor copyright claim in the source;
c     presumably by the authors of 
c     
c     R.B. Cleveland, W.S.Cleveland, J.E. McRae, and I. Terpenning,
c     STL: A Seasonal-Trend Decomposition Procedure Based on Loess, 
c     Statistics Research Report, AT&T Bell Laboratories.
c     
c     Converted to double precision by B.D. Ripley 1999.
c     Indented, goto labels renamed, many goto's replaced by `if then {else}'
c     (using Emacs), many more comments;  by M.Maechler 2001-02.
c     

*/

void stl_(double *y, int *n, int *np, int *ns, int *nt, int *nl, int *isdeg, int *itdeg, int *
	ildeg, int *nsjump, int *ntjump, int *nljump, int *ni, int *no, double *rw, double *season, double *trend, 
	double *work)
{
    /* System generated locals */
    int work_dim1, work_offset, i__1;

    /* Local variables */
    int i__, k, newnl, newnp, newns, newnt;
	int userw;

/* Arg */
/* 	n                   : length(y) */
/* 	ns, nt, nl          : spans        for `s', `t' and `l' smoother */
/* 	isdeg, itdeg, ildeg : local degree for `s', `t' and `l' smoother */
/* 	nsjump,ntjump,nljump: ........     for `s', `t' and `l' smoother */
/*       ni, no              : number of inner and outer (robust) iterations */
/* Var */
    /* Parameter adjustments */
    --trend;
    --season;
    --rw;
    --y;
    work_dim1 = *n + 2 * *np;
    work_offset = 1 + work_dim1;
    work -= work_offset;

    /* Function Body */
    userw = 0;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	trend[i__] = 0.0;
/* L1: */
    }
/* the three spans must be at least three and odd: */
    newns = imax(3,*ns);
    newnt = imax(3,*nt);
    newnl = imax(3,*nl);
    if (newns % 2 == 0) {
	++newns;
    }
    if (newnt % 2 == 0) {
	++newnt;
    }
    if (newnl % 2 == 0) {
	++newnl;
    }
/* periodicity at least 2: */
    newnp = imax(2,*np);
    k = 0;
/* --- outer loop -- robustnes iterations */
L100:
    stlstp_(&y[1], n, &newnp, &newns, &newnt, &newnl, isdeg, itdeg, ildeg, 
	    nsjump, ntjump, nljump, ni, &userw, &rw[1], &season[1], &trend[1],
	     &work[work_offset]);
    ++k;
    if (k > *no) {
	goto L10;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	work[i__ + work_dim1] = trend[i__] + season[i__];
/* L3: */
    }
    stlrwt_(&y[1], n, &work[work_dim1 + 1], &rw[1]);
    userw = 1;
    goto L100;
/* --- end Loop */
L10:
/*     robustness weights when there were no robustness iterations: */
    if (*no <= 0) {
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    rw[i__] = 1.0;
/* L15: */
	}
    }
} 

int stless_(double *y, int *n, int *len, int *ideg, int *njump, int *userw, double *rw, double *ys,
	 double *res)
{
    /* System generated locals */
    int i__1, i__2, i__3;
    double d__1;

    /* Local variables */
    int i__, j, k;
    int ok;
    int nsh;
    double delta;
    int nleft, newnj, nright;

/* Arg */
/* Var */
    /* Parameter adjustments */
    --res;
    --ys;
    --rw;
    --y;

    /* Function Body */
    if (*n < 2) {
	ys[1] = y[1];
	return 0;
    }
/* Computing MIN */
    i__1 = *njump, i__2 = *n - 1;
    newnj = imin(i__1,i__2);
    if (*len >= *n) {
	nleft = 1;
	nright = *n;
	i__1 = *n;
	i__2 = newnj;
	for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	    d__1 = (double) i__;
	    stlest_(&y[1], n, len, ideg, &d__1, &ys[i__], &nleft, &nright, &
		    res[1], userw, &rw[1], &ok);
	    if (! ok) {
		ys[i__] = y[i__];
	    }
/* L20: */
	}
    } else {
	if (newnj == 1) {
	    nsh = (*len + 1) / 2;
	    nleft = 1;
	    nright = *len;
	    i__2 = *n;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		if (i__ > nsh && nright != *n) {
		    ++nleft;
		    ++nright;
		}
		d__1 = (double) i__;
		stlest_(&y[1], n, len, ideg, &d__1, &ys[i__], &nleft, &nright,
			 &res[1], userw, &rw[1], &ok);
		if (! ok) {
		    ys[i__] = y[i__];
		}
/* L30: */
	    }
	} else {
	    nsh = (*len + 1) / 2;
	    i__2 = *n;
	    i__1 = newnj;
	    for (i__ = 1; i__1 < 0 ? i__ >= i__2 : i__ <= i__2; i__ += i__1) {
		if (i__ < nsh) {
		    nleft = 1;
		    nright = *len;
		} else if (i__ >= *n - nsh + 1) {
		    nleft = *n - *len + 1;
		    nright = *n;
		} else {
		    nleft = i__ - nsh + 1;
		    nright = *len + i__ - nsh;
		}
		d__1 = (double) i__;
		stlest_(&y[1], n, len, ideg, &d__1, &ys[i__], &nleft, &nright,
			 &res[1], userw, &rw[1], &ok);
		if (! ok) {
		    ys[i__] = y[i__];
		}
/* L40: */
	    }
	}
    }
    if (newnj != 1) {
	i__1 = *n - newnj;
	i__2 = newnj;
	for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	    delta = (ys[i__ + newnj] - ys[i__]) / (double) newnj;
	    i__3 = i__ + newnj - 1;
	    for (j = i__ + 1; j <= i__3; ++j) {
		ys[j] = ys[i__] + delta * (double) (j - i__);
/* L47: */
	    }
/* L45: */
	}
	k = (*n - 1) / newnj * newnj + 1;
	if (k != *n) {
	    d__1 = (double) (*n);
	    stlest_(&y[1], n, len, ideg, &d__1, &ys[*n], &nleft, &nright, &
		    res[1], userw, &rw[1], &ok);
	    if (! ok) {
		ys[*n] = y[*n];
	    }
	    if (k != *n - 1) {
		delta = (ys[*n] - ys[k]) / (double) (*n - k);
		i__2 = *n - 1;
		for (j = k + 1; j <= i__2; ++j) {
		    ys[j] = ys[k] + delta * (double) (j - k);
/* L55: */
		}
	    }
	}
    }
	return 0;
} /* stless_ */

int stlest_(double *y, int *n, int *len, int *ideg, double *xs, double *ys, int *nleft, int *
	nright, double *w, int *userw, double *rw, int *ok)
{
    /* System generated locals */
    int i__1;
    double d__1, d__2;


    /* Local variables */
    double a, b, c__, h__;
    int j;
    double r__, h1, h9, range;

/* Arg */
/* Var */
    /* Parameter adjustments */
    --rw;
    --w;
    --y;

    /* Function Body */
    range = (double) (*n) - 1.;
/* Computing MAX */
    d__1 = *xs - (double) (*nleft), d__2 = (double) (*nright) - *xs;
    h__ = pmax(d__1,d__2);
    if (*len > *n) {
	h__ += (double) ((*len - *n) / 2);
    }
    h9 = h__ * .999;
    h1 = h__ * .001;
    a = 0.0;
    i__1 = *nright;
    for (j = *nleft; j <= i__1; ++j) {
	r__ = (d__1 = (double) j - *xs, fabs(d__1));
	if (r__ <= h9) {
	    if (r__ <= h1) {
		w[j] = 1.0;
	    } else {
/* Computing 3rd power */
		d__2 = r__ / h__;
/* Computing 3rd power */
		d__1 = 1.f - d__2 * (d__2 * d__2);
		w[j] = d__1 * (d__1 * d__1);
	    }
	    if (*userw) {
		w[j] = rw[j] * w[j];
	    }
	    a += w[j];
	} else {
	    w[j] = 0.0;
	}
/* L60: */
    }
    if (a <= 0.0) {
	*ok = 0;
    } else {
	*ok = 1;
	i__1 = *nright;
	for (j = *nleft; j <= i__1; ++j) {
	    w[j] /= a;
/* L69: */
	}
	if (h__ > 0.f && *ideg > 0) {
	    a = 0.f;
	    i__1 = *nright;
	    for (j = *nleft; j <= i__1; ++j) {
		a += w[j] * (double) j;
/* L73: */
	    }
	    b = *xs - a;
	    c__ = 0.0;
	    i__1 = *nright;
	    for (j = *nleft; j <= i__1; ++j) {
/* Computing 2nd power */
		d__1 = (double) j - a;
		c__ += w[j] * (d__1 * d__1);
/* L75: */
	    }
	    if (sqrt(c__) > range * .001) {
		b /= c__;
		i__1 = *nright;
		for (j = *nleft; j <= i__1; ++j) {
		    w[j] *= b * ((double) j - a) + 1.0;
/* L79: */
		}
	    }
	}
	*ys = 0.0;
	i__1 = *nright;
	for (j = *nleft; j <= i__1; ++j) {
	    *ys += w[j] * y[j];
/* L81: */
	}
    }
    return 0;
} /* stlest_ */

int stlfts_(double *x, int *n, int *np, double *trend, double *work)
{
    /* System generated locals */
    int i__1;
	int c__3 = 3;

    /* Parameter adjustments */
    --work;
    --trend;
    --x;

    /* Function Body */
    stlma_(&x[1], n, np, &trend[1]);
    i__1 = *n - *np + 1;
    stlma_(&trend[1], &i__1, np, &work[1]);
    i__1 = *n - (*np << 1) + 2;
    stlma_(&work[1], &i__1, &c__3, &trend[1]);
    return 0;
} /* stlfts_ */

int stlma_(double *x, int *n, int *len, double *ave)
{
    /* System generated locals */
    int i__1;

    /* Local variables */
    int i__, j, k, m;
    double v, flen;
    int newn;

/* Moving Average (aka "running mean") */
/* ave(i) := mean(x{j}, j = max(1,i-k),..., min(n, i+k)) */
/*           for i = 1,2,..,n */
/* Arg */
/* Var */
    /* Parameter adjustments */
    --ave;
    --x;

    /* Function Body */
    newn = *n - *len + 1;
    flen = (double) (*len);
    v = 0.0;
    i__1 = *len;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v += x[i__];
/* L3: */
    }
    ave[1] = v / flen;
    if (newn > 1) {
	k = *len;
	m = 0;
	i__1 = newn;
	for (j = 2; j <= i__1; ++j) {
	    ++k;
	    ++m;
	    v = v - x[m] + x[k];
	    ave[j] = v / flen;
/* L7: */
	}
    }
    return 0;
} /* stlma_ */

int stlstp_(double *y, int *n, int *np, int *ns, int *nt, int *nl, int *isdeg, int *itdeg, int 
	*ildeg, int *nsjump, int *ntjump, int *nljump, int *ni, int *userw, double *rw, double *season,
	double *trend, double *work)
{
    /* System generated locals */
    int work_dim1, work_offset, i__1, i__2;

    /* Local variables */
    int i__, j;
	int c_false = 0;

/* Arg */
/* Var */
    /* Parameter adjustments */
    --trend;
    --season;
    --rw;
    --y;
    work_dim1 = *n + 2 * *np;
    work_offset = 1 + work_dim1;
    work -= work_offset;

    /* Function Body */
    i__1 = *ni;
    for (j = 1; j <= i__1; ++j) {
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    work[i__ + work_dim1] = y[i__] - trend[i__];
/* L1: */
	}
	stlss_(&work[work_dim1 + 1], n, np, ns, isdeg, nsjump, userw, &rw[1], 
		&work[(work_dim1 << 1) + 1], &work[work_dim1 * 3 + 1], &work[(
		work_dim1 << 2) + 1], &work[work_dim1 * 5 + 1], &season[1]);
	i__2 = *n + (*np << 1);
	stlfts_(&work[(work_dim1 << 1) + 1], &i__2, np, &work[work_dim1 * 3 + 
		1], &work[work_dim1 + 1]);
	stless_(&work[work_dim1 * 3 + 1], n, nl, ildeg, nljump, &c_false, &
		work[(work_dim1 << 2) + 1], &work[work_dim1 + 1], &work[
		work_dim1 * 5 + 1]);
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    season[i__] = work[*np + i__ + (work_dim1 << 1)] - work[i__ + 
		    work_dim1];
/* L3: */
	}
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    work[i__ + work_dim1] = y[i__] - season[i__];
/* L5: */
	}
	stless_(&work[work_dim1 + 1], n, nt, itdeg, ntjump, userw, &rw[1], &
		trend[1], &work[work_dim1 * 3 + 1]);
/* L80: */
    }
    return 0;
} /* stlstp_ */

int stlrwt_(double *y, int *n, double *fit, double *rw)
{
    /* System generated locals */
    int i__1;
    double d__1, d__2;

    /* Local variables */
    int i__;
    double r__, c1, c9;
    int mid[2];
    double cmad;
	int c__2 = 2;

/* Robustness Weights */
/* 	rw_i := B( |y_i - fit_i| / (6 M) ),   i = 1,2,...,n */
/* 		where B(u) = (1 - u^2)^2  * 1[|u| < 1]   {Tukey's biweight} */
/* 		and   M := median{ |y_i - fit_i| } */
/* Arg */
/* Var */
    /* Parameter adjustments */
    --rw;
    --fit;
    --y;

    /* Function Body */
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	rw[i__] = (d__1 = y[i__] - fit[i__], fabs(d__1));
/* L7: */
    }
    mid[0] = *n / 2 + 1;
    mid[1] = *n - mid[0] + 1;
    psort_(&rw[1], *n, mid, c__2);
    cmad = (rw[mid[0]] + rw[mid[1]]) * 3.0;
/*     = 6 * MAD */
    c9 = cmad * .999;
    c1 = cmad * .001;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	r__ = (d__1 = y[i__] - fit[i__], fabs(d__1));
	if (r__ <= c1) {
	    rw[i__] = 1.f;
	} else if (r__ <= c9) {
/* Computing 2nd power */
	    d__2 = r__ / cmad;
/* Computing 2nd power */
	    d__1 = 1.f - d__2 * d__2;
	    rw[i__] = d__1 * d__1;
	} else {
	    rw[i__] = 0.0;
	}
/* L10: */
    }
    return 0;
} /* stlrwt_ */

int stlss_(double *y, int *n, int *np, int *ns, int *isdeg, int *nsjump, int *userw, double *rw, 
	double *season, double *work1, double *work2, double *work3, double *work4)
{
    /* System generated locals */
    int i__1, i__2, i__3;
	int c__1 = 1;

    /* Local variables */
    int i__, j, k, m;
    int ok;
    double xs;
    int nleft, nright;

/* 	called by stlstp() at the beginning of each (inner) iteration */

/* Arg */
/* Var */
    /* Parameter adjustments */
    --work4;
    --work3;
    --work2;
    --work1;
    --rw;
    --y;
    --season;

    /* Function Body */
    if (*np < 1) {
	return 0;
    }
    i__1 = *np;
    for (j = 1; j <= i__1; ++j) {
	k = (*n - j) / *np + 1;
	i__2 = k;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    work1[i__] = y[(i__ - 1) * *np + j];
/* L10: */
	}
	if (*userw) {
	    i__2 = k;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		work3[i__] = rw[(i__ - 1) * *np + j];
/* L12: */
	    }
	}
	stless_(&work1[1], &k, ns, isdeg, nsjump, userw, &work3[1], &work2[2],
		 &work4[1]);
	xs = 0.;
	nright = imin(*ns,k);
	stlest_(&work1[1], &k, ns, isdeg, &xs, &work2[1], &c__1, &nright, &
		work4[1], userw, &work3[1], &ok);
	if (! ok) {
	    work2[1] = work2[2];
	}
	xs = (double) (k + 1);
/* Computing MAX */
	i__2 = 1, i__3 = k - *ns + 1;
	nleft = imax(i__2,i__3);
	stlest_(&work1[1], &k, ns, isdeg, &xs, &work2[k + 2], &nleft, &k, &
		work4[1], userw, &work3[1], &ok);
	if (! ok) {
	    work2[k + 2] = work2[k + 1];
	}
	i__2 = k + 2;
	for (m = 1; m <= i__2; ++m) {
	    season[(m - 1) * *np + j] = work2[m];
/* L18: */
	}
/* L200: */
    }
    return 0;
} /* stlss_ */


void stlez_(double *y, int *n, int *np, int *ns, int *isdeg, int *itdeg,int *robust, int *no, 
	double *rw, double *season, double *trend, double *work)
{
    /* System generated locals */
    int work_dim1, work_offset, i__1, i__2;
    double d__1;

    /* Local variables */
    int i__, j, ni, nl, nt;
    double difs, dift, mins, mint, maxs, maxt;
    int ildeg;
    double maxds, maxdt;
    int newnp, newns, nljump, nsjump, ntjump;
	int c_false = 0;
	int c_true = 1;

		// Robust

/* Arg */
/* Var */
    /* Parameter adjustments */
    --trend;
    --season;
    --rw;
    --y;
    work_dim1 = *n + 2 * *np;
    work_offset = 1 + work_dim1;
    work -= work_offset;

    /* Function Body */
    ildeg = *itdeg;
    newns = imax(3,*ns);
    if (newns % 2 == 0) {
	++newns;
    }
    newnp = imax(2,*np);
    nt = newnp * 1.5 / (1 - 1.5 / newns) + .5;
    nt = imax(3,nt);
    if (nt % 2 == 0) {
	++nt;
    }
    nl = newnp;
    if (nl % 2 == 0) {
	++nl;
    }
    if (*robust) {
	ni = 1;
    } else {
	ni = 2;
    }
/* Computing MAX */
    i__1 = 1, i__2 = (int) ((double) newns / 10 + .9);
    nsjump = imax(i__1,i__2);
/* Computing MAX */
    i__1 = 1, i__2 = (int) ((double) nt / 10 + .9);
    ntjump = imax(i__1,i__2);
/* Computing MAX */
    i__1 = 1, i__2 = (int) ((double) nl / 10 + .9);
    nljump = imax(i__1,i__2);
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	trend[i__] = 0.0;
/* L2: */
    }
    stlstp_(&y[1], n, &newnp, &newns, &nt, &nl, isdeg, itdeg, &ildeg, &nsjump,
	     &ntjump, &nljump, &ni, &c_false, &rw[1], &season[1], &trend[1], &
	    work[work_offset]);
    *no = 0;
    if (*robust) {
	j = 1;
/*        Loop  --- 15 robustness iterations */
L100:
	if (j <= 15) {
	    i__1 = *n;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		work[i__ + work_dim1 * 6] = season[i__];
		work[i__ + work_dim1 * 7] = trend[i__];
		work[i__ + work_dim1] = trend[i__] + season[i__];
/* L35: */
	    }
	    stlrwt_(&y[1], n, &work[work_dim1 + 1], &rw[1]);
	    stlstp_(&y[1], n, &newnp, &newns, &nt, &nl, isdeg, itdeg, &ildeg, 
		    &nsjump, &ntjump, &nljump, &ni, &c_true, &rw[1], &season[
		    1], &trend[1], &work[work_offset]);
	    ++(*no);
	    maxs = work[work_dim1 * 6 + 1];
	    mins = work[work_dim1 * 6 + 1];
	    maxt = work[work_dim1 * 7 + 1];
	    mint = work[work_dim1 * 7 + 1];
	    maxds = (d__1 = work[work_dim1 * 6 + 1] - season[1], fabs(d__1));
	    maxdt = (d__1 = work[work_dim1 * 7 + 1] - trend[1], fabs(d__1));
	    i__1 = *n;
	    for (i__ = 2; i__ <= i__1; ++i__) {
		if (maxs < work[i__ + work_dim1 * 6]) {
		    maxs = work[i__ + work_dim1 * 6];
		}
		if (maxt < work[i__ + work_dim1 * 7]) {
		    maxt = work[i__ + work_dim1 * 7];
		}
		if (mins > work[i__ + work_dim1 * 6]) {
		    mins = work[i__ + work_dim1 * 6];
		}
		if (mint > work[i__ + work_dim1 * 7]) {
		    mint = work[i__ + work_dim1 * 7];
		}
		difs = (d__1 = work[i__ + work_dim1 * 6] - season[i__], fabs(
			d__1));
		dift = (d__1 = work[i__ + work_dim1 * 7] - trend[i__], fabs(
			d__1));
		if (maxds < difs) {
		    maxds = difs;
		}
		if (maxdt < dift) {
		    maxdt = dift;
		}
/* L137: */
	    }
	    if (maxds / (maxs - mins) < .01 && maxdt / (maxt - mint) < .01) 
		    {
		goto L300;
	    }
/* L151: */
	    ++j;
	    goto L100;
	}
/*        end Loop */
L300:
	;
    } else {
/*     	.not. robust */
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    rw[i__] = 1.0;
/* L150: */
	}
    }
} 

int psort_(double *a, int n, int *ind, int ni)
{
    int i__, j, k, l, m, p;
    double t;
    int ij, il[16], jl, iu[16], ju;
    int tt;
    int indl[16], indu[16];


/* Partial Sorting ; used for Median (MAD) computation only */

/* Arg */
/* Var */
    /* Parameter adjustments */
    --a;
    --ind;

    /* Function Body */
    if (n < 0 || ni < 0) {
	return 0;
    }
    if (n < 2 || ni == 0) {
	return 0;
    }
    jl = 1;
    ju = ni;
    indl[0] = 1;
    indu[0] = ni;
    i__ = 1;
    j = n;
    m = 1;
/* Outer Loop */
L161:
    if (i__ < j) {
	goto L10;
    }
/*  _Loop_ */
L166:
    --m;
    if (m == 0) {
	return 0;
    }
    i__ = il[m - 1];
    j = iu[m - 1];
    jl = indl[m - 1];
    ju = indu[m - 1];
    if (! (jl <= ju)) {
	goto L166;
    }
/*     while (j - i > 10) */
L173:
    if (! (j - i__ > 10)) {
	goto L174;
    }
L10:
    k = i__;
    ij = (i__ + j) / 2;
    t = a[ij];
    if (a[i__] > t) {
	a[ij] = a[i__];
	a[i__] = t;
	t = a[ij];
    }
    l = j;
    if (a[j] < t) {
	a[ij] = a[j];
	a[j] = t;
	t = a[ij];
	if (a[i__] > t) {
	    a[ij] = a[i__];
	    a[i__] = t;
	    t = a[ij];
	}
    }
L181:
    --l;
    if (a[l] <= t) {
	tt = a[l];
L186:
	++k;
/* L187: */
	if (! (a[k] >= t)) {
	    goto L186;
	}
	if (k > l) {
	    goto L183;
	}
	a[l] = a[k];
	a[k] = tt;
    }
/* L182: */
    goto L181;
L183:
    indl[m - 1] = jl;
    indu[m - 1] = ju;
    p = m;
    ++m;
    if (l - i__ <= j - k) {
	il[p - 1] = k;
	iu[p - 1] = j;
	j = l;
L193:
	if (jl > ju) {
	    goto L166;
	}
	if (ind[ju] > j) {
	    --ju;
	    goto L193;
	}
	indl[p - 1] = ju + 1;
    } else {
	il[p - 1] = i__;
	iu[p - 1] = l;
	i__ = k;
L200:
	if (jl > ju) {
	    goto L166;
	}
	if (ind[jl] < i__) {
	    ++jl;
	    goto L200;
	}
	indu[p - 1] = jl - 1;
    }
    goto L173;
/*     end while */
L174:
    if (i__ != 1) {
	--i__;
L209:
	++i__;
	if (i__ == j) {
	    goto L166;
	}
	t = a[i__ + 1];
	if (a[i__] > t) {
	    k = i__;
/*           repeat */
L216:
	    a[k + 1] = a[k];
	    --k;
	    if (! (t >= a[k])) {
		goto L216;
	    }
/*           until  t >= a(k) */
	    a[k + 1] = t;
	}
	goto L209;
    }
    goto L161;
/* End Outer Loop */
} /* psort_ */
