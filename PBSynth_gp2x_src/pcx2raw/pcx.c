#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcx.h"

void * loadFile ( const char * fname, int * filesize )
	{
		FILE * fh;
		void * mem;

		fh = fopen ( fname, "rb" );
		if (!fh)
			return 0;

		fseek ( fh, 0, SEEK_END );
		*filesize = ftell ( fh );
		fseek ( fh, 0, SEEK_SET );
		mem = malloc ( *filesize );
		if (mem==NULL)
			{
				fclose ( fh );
				return NULL;
			}
		fread ( mem, *filesize, 1, fh );
		fclose ( fh );

		return mem;
	}

void freeFile ( void * file )
	{
		if (file!=NULL)
			free ( file );
	}

int PCX_Load ( PCX * pcx, const char * fname )
	{
		unsigned char * file;
		int filesize;
		int s;
		int curofs;
		int srci = 0;
		int bytesperline;
		int compressed;
		int x1,y1,x2,y2;
		int i;
		int datalen;
		unsigned char * data;

		file = loadFile ( fname, &datalen );
		filesize = datalen;
		s = datalen;		
		if ( file==NULL )
			{
				return 1;
			}

		datalen -= 128+769;		
		data = &file[128];

		curofs=0;
		if (file[curofs++]!=0x0a)
			return 1;
		if (file[curofs++]!=0x05)
			return 1;

		if (file[curofs++]==0)
			compressed = 0;
		else
			compressed = 1;

		if (file[curofs++]!=8)
			return 1;
		
		x1  = file[curofs++];
		x1 |= file[curofs++]<<8;
		y1  = file[curofs++];
		y1 |= file[curofs++]<<8;
		x2  = file[curofs++];
		x2 |= file[curofs++]<<8;
		y2  = file[curofs++];
		y2 |= file[curofs++]<<8;
		
		bytesperline = file[66];
		bytesperline |= file[67]<<8;

		pcx->width = (x2-x1)+1;
		pcx->height= (y2-y1)+1;
		
		if ( (pcx->data = malloc ( pcx->width*pcx->height ))==NULL )
			{
				freeFile ( file );
				return 1;
			}

		if (compressed)
			{
				int i=0;
				int s=pcx->width*pcx->height;
				int countlinebytes = pcx->width;
				srci = 0;
				
				while (i<s)
					{
						unsigned char c = data[srci++];

						if ((c&0xc0)==0xc0)
							{
								int l = c&~0xc0;
								int j;
								unsigned char d = data[srci++];
								for (j=0;j<l;j++)
									{
										pcx->data[i++] = d;
										countlinebytes--;
									}
							} else
							{
								pcx->data[i++] = c;
								countlinebytes--;
							}
						
						if (countlinebytes<=0)
							{
								countlinebytes = pcx->width;
								srci+=bytesperline-pcx->width;
							}
					}
			} else
			{
				memcpy ( pcx->data, data, datalen );
			}

/*		curofs = srci + 128;*/
		curofs = filesize-769;
		if (file[curofs++]!=0x0c)
			{
				free ( pcx->data );
				pcx->data = NULL;
				freeFile ( file );
				return 1;
			}

		for (i=0;i<256;i++)
			{
				pcx->pal[i].r = file[curofs++];
				pcx->pal[i].g = file[curofs++];
				pcx->pal[i].b = file[curofs++];
			}
		
		freeFile ( file );

		return 0;
	}

void PCX_Free ( PCX * pcx )
	{
		if (pcx->data!=NULL)
			{
				free (pcx->data);
				pcx->data = NULL;
			}
	}
