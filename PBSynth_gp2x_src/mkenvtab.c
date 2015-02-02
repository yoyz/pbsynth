#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ENV_MAX	2.0f
#define ENV_MIN (2.0f / (44100.0f * 5.0f))
#define ENV_M (((1.0f / ENV_MIN) - (1.0f / ENV_MAX)) / 2.0f)
#define ENV_B (((1.0f / ENV_MIN) + (1.0f / ENV_MAX)) / 2.0f)

float param2IncVal(float x) {
	
	float ret;
	
	if (x < -1)
		x = -1;
	else
	if (x > 1)
		x = 1;
	
	ret = (ENV_M * x) + ENV_B;
	x = 1;
	ret = x / ret;

	return ret;
}

int main(int argc, char *argv[]) {

	int i;
	
	printf("float envtab[128] = {\n");
	for (i = 0; i < 128; i++) {
		
		float t;
		float r1, r2;
		
		t = i;
		t = t / 127.0f;
		r1 = param2IncVal((t * 2.0f) - 1.0f);
		t = ((1.0f - cos(M_PI * t * 0.5f)) * 2.0f) - 1.0f;
		r2 = param2IncVal(t);
		printf("\t%ff", r2);
		if (i != 127)
			printf(",");
		printf("\n");
	}
	
	printf("};\n");
		
	return 0;
}
