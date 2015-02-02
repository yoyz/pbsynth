#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LFO_MAX	(2.0f / (44100.0f * 0.02f))
#define LFO_MIN (2.0f / (44100.0f * 1.0f))
#define LFO_M (((1.0f / LFO_MIN) - (1.0f / LFO_MAX)) / 2.0f)
#define LFO_B (((1.0f / LFO_MIN) + (1.0f / LFO_MAX)) / 2.0f)

float param2IncVal(float x) {
	
	float ret;
	
	if (x < -1.0f)
		x = -1.0f;
	else
	if (x > 1.0f)
		x = 1.0f;
	
	ret = (LFO_M * x) + LFO_B;
	x = 1;
	ret = x / ret;

	return ret;
}

int main(int argc, char *argv) {

	int i;
	
	printf("float lfotab[128] = {\n\n");
	
	for (i = 0; i < 128; i++) {

		float t;
		float r1;
		
		t = i;
		t = t / 127.0f;
		r1 = param2IncVal((t * 2.0f) - 1.0f);

		printf("\t%ff", r1);
		if (i != 127)
			printf(",");
		printf("\n");
	}
	
	printf("};\n");
	
	return 0;
}
