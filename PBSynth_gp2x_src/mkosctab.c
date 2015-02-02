#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NFINE 128

float getIncVal(int note, int fine) {
	
	float f;

	note = note - 9 - 24 - 24;
	note = note * NFINE;

	f = 440.0f * pow(2.0f, ((float)(note + fine)) / (12.0f * NFINE));

	f = 44100.0f / f;

	return f;	
}


int main(int argc, char *argv[]) {

	int n, f;
	
	printf("float osctab[2 * 128 * %d] = {\n\n", NFINE);
	for (n = 0; n < 128; n++) {
		
		printf("\t");
		for (f = 0; f < NFINE; f++) {
			
			printf("%ff,", getIncVal(n, f));
			printf("%ff", 2.0f / getIncVal(n, f));
			if (!(f == (NFINE - 1) && n == 127))
				printf(",");
		}
		printf("\t/* Note: %d */\n", n);
	}
	printf("};\n");
	
	return 0;
}
