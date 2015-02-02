#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcx.h"

void changeSuffix(const char *src, char *dest, const char *suffix) {
	
	int lastDot = -1;
	int i = 0;

	while (src[i]) {
		
		if (src[i] == '.')
			lastDot = i;
		
		dest[i] = src[i];
		
		i++;
	}
	
	if (lastDot < 0)
		lastDot = i;

	strcpy(&dest[lastDot], suffix);
}

int main(int argc, char *argv[]) {

	char pcxname[256];
	char palname[256];
	char rawname[256];
	FILE *fh;
	PCX pcx;
	
	if (argc != 2) {
		
		fprintf(stderr, "Usage: %s filename.pcx", argv[0]);
		return 1;
	}
	
	strcpy(pcxname, argv[1]);
	
	changeSuffix(pcxname, palname, ".pal");
	changeSuffix(pcxname, rawname, ".raw");

	if (PCX_Load(&pcx, pcxname) != 0) {
		
		fprintf(stderr, "Couldn't load file '%s'\n", pcxname);
		return 1;
	}
	
	if (!(fh = fopen(rawname, "wb"))) {
		
		fprintf(stderr, "Couldn't write file '%s'\n", rawname);
		PCX_Free(&pcx);
		return 1;
	}
	fwrite(pcx.data, pcx.width * pcx.height, 1, fh);
	fclose(fh);
	
	if (!(fh = fopen(palname, "wb"))) {
		
		fprintf(stderr, "Couldn't write file '%s'\n", palname);
		PCX_Free(&pcx);
		return 1;
	}
	fwrite(pcx.pal, 256 * 3, 1, fh);
	fclose(fh);
	
	PCX_Free(&pcx);
	
	return 0;
}
