#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float xb[4];
float yb[4];

float a0, a1, a2, b1, b2;
float w0, r0,
      wp, rp;

float fs = 44100.0f;

float getFilterGainDC(void) {
	
	return (a0 + a1 + a2) / (1 - (b1 + b2));
}

float getFilterGain(float wp) {

	if (wp >= M_PI)
		return 0.0f;
	else
		return sqrt((pow(a0 + a1 * cos(wp) + a2 * cos(2.0f * wp), 2.0f) +
	  	           pow(a1 * sin(-wp) + a2 * sin(-2.0f * wp), 2.0f)) /
	    	        (pow(1.0f - b1 * cos(wp) - b2 * cos(2.0f * wp), 2.0f) +
	      	       pow(b1 * sin(-wp) + b2 * sin(-2.0f * wp), 2.0f)));
}

void scaleFilter(float amp) {
	
	a0 *= amp;
	a1 *= amp;
	a2 *= amp;
}

void setFilterCoefficients(float w0, float r0, float wp, float rp) {

	a0 = 1.0f,
	a1 = -2.0f * r0 * cos(w0),
	a2 = r0 * r0,
	b1 = 2.0f * rp * cos(wp),
	b2 = -(rp * rp);
}

void normalizeFilterDC(void) {

	scaleFilter(1.0f / getFilterGainDC());
}

void normalizeFilter(float wp) {
	
	scaleFilter(1.0f / getFilterGain(wp));
}

void setFilter(float wpp, float reso) {

	float rpt;
	
	float G;
	float rG;
	
	r0 = 1.0f;
	w0 = M_PI;
	
	wp = wpp;

	rpt = 0.0f;
	
	do {
		
		rpt += 0.0001f;
		rp = reso + ((1.0f - reso) * rpt);
		setFilterCoefficients(w0, r0, wp, rp);
		normalizeFilterDC();

	} while ((getFilterGain(wp * 2.5f) > 0.7f) &&
	         ((getFilterGain(wp) < 0.8f) || wp < 0.5f * M_PI) &&
	         (rpt < 0.99f));

	scaleFilter(1.0f / (getFilterGainDC() * getFilterGain(wp)));

	printf("wp = %f*PI, rp = %f, gain(dc) = %f, gain(wp) = %f\n", wp / M_PI, rp, getFilterGainDC(), getFilterGain(wp));
}

void calcFilter(float *dest, float *src, int iter, int N) {

	int n;
	int j;

	for (n = 0; n < N; n++) {
		
		int i;

		for (i = 3; i > 0; i--) {
		
			xb[i] = xb[i - 1];
			yb[i] = yb[i - 1];
		}
		
		xb[0] = src[n];

		dest[n] = (a0 * xb[0]) + (a1 * xb[1]) + (a2 * xb[2]) +
	  		                     (b1 * yb[1]) + (b2 * yb[2]);

		yb[0] = dest[n];
	}
}

int main(void) {

	FILE *fh;
	signed short x[65536];
	signed short y[65536];
	float xf[65536];
	float yf[65536];
	int n, i, j;
	int iter;
	float m;
	float ad;
	float f = 20000.0f;
	

	if (!(fh = fopen("noise.pcm", "rb"))) {
		
		printf("Couldn't open 'noise.pcm'\n");
		return 1;
	}
	
	fread(x, 65536, 2, fh);
	fclose(fh);
	
	for (n = 0; n < 65536; n++)
		xf[n] = ((float)x[n]) / 32768.0f;


	for (j = 0; j < 2; j++) {
		
		n = 0;
		i = 0;
		f = 20000.0f;
		ad = -7.0f;
		
		for (i = 0; i < 65536/16; i++) {
	
			setFilter((2.0f * M_PI * f) / fs, 0.0f);

			calcFilter(&yf[n], &xf[n], 1, 16);

			n += 16;
			f += ad;
			
			if (f < 20.0f)
				ad = -ad;
		}
	
		for (n = 0; n < 65536; n++)
			xf[n] = yf[n];
	}
	
/*
	for (iter = 0; iter < 1; iter++) {
		
		 calcFilter(yf, xf, 1, 65536);

		for (n = 0; n < 65536; n++)
			xf[n] = yf[n];
	}
*/
	
	for (n = 0; n < 65536; n++)
		y[n] = yf[n] * 32768;
		
	fh = fopen("noiseout.pcm", "wb");
	fwrite(y, 65536, 2, fh);
	fclose(fh);
	
	printf("sizeof(float) = %d, sizeof(double) = %d\n", sizeof(float), sizeof(double));

	return 0;
}
