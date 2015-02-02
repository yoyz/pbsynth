#ifndef __PCX_H__
#define __PCX_H__

typedef struct PCX
	{
		unsigned char * data;
		int width;
		int height;
		struct
			{
				unsigned char r,g,b;
			} pal[256];
	} PCX;

int PCX_Load ( PCX *, const char * );
void PCX_Free ( PCX * );

#endif
