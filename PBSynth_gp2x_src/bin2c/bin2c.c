/* bin2c.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "getopts.h"

#define CHUNKSIZE 16

static void strupor ( char * dest, const char * src )
	{
		int l = strlen(src);
		int i;

		for (i=0;i<l;i++)
			dest[i] = toupper(src[i]);
		dest[l] = '\0';
	}

int main ( int argc, char * argv[] )
	{
		FILE * ofile;
		FILE * ifile;
		int gasm;
		unsigned char * mem;
		int sz;
		int filesize;
		char * args;
		char infile[256];
		int infileok = 0;
		char outfile[256];
		int outfileok = 0;
		char label[256];
		int labelok = 0;
		char headerfile[256];
		int headerok = 0;
		int underscore = 0;
		int i;

    struct options opts[] =
      {
        { 1,  "version", "Show version info", "V", 0 },
        { 2,  "infile", "Input file", "i", 1 },
        { 3,  "outfile", "Output file", "o", 1 },
        { 4,  "label", "Label name", "l", 1 },
        { 5,  "asm", "GNU AS data output", "s", 0 },
        { 6,  "ansic", "ANSI C data output", "c", 0 },
        { 7,  "header", "ANSI C header file", "g", 1 },
        { 8,  "underscore", "Add underscore to ASM output label", "u", 0 },
        { 0, NULL, NULL, NULL, 0 }
      };

    memset ( infile, 0, sizeof(infile) );
    memset ( outfile, 0, sizeof(outfile) );
    memset ( label, 0, sizeof(label) );
    memset ( headerfile, 0, sizeof(headerfile) );

    args = calloc ( 1, 128 );
    while ((i=getopts(argc, argv, opts, args)) != 0)
      {
        switch (i)
          {
            case 1:   /* version */
              printf ("bin2c Copyright (c)2003 by Christian Nowak <chnowak@web.de>\n");
              printf ("Project website: http://chn.roarvgm.com/\n");
              exit(0);
              break;
            case 2:   /* infile */
            	strcpy ( infile, args );
            	infileok = 1;
              break;
            case 3:   /* outfile */
            	strcpy ( outfile, args );
            	outfileok = 1;
              break;
            case 4:   /* label */
            	strcpy ( label, args );
            	labelok = 1;
              break;
            case 5:		/* asm */
            	gasm = 1;
            	break;
            case 6:		/* c */
            	gasm = 0;
            	break;
            case 7:		/* header file */
            	strcpy ( headerfile, args );
            	headerok = 1;
            	break;
            case 8:   /* Underscore */
            	underscore = 1;
            	break;
          }
    	}

    if (infileok==0)
    	{
	    	printf("You must specify an input file.\n");
	    	return 1;
    	}

    if (outfileok==0)
    	{
	    	printf("You must specify an output file.\n");
	    	return 1;
    	}

    if (labelok==0)
    	{
	    	printf("You must specify a label name.\n");
	    	return 1;
    	}


    ifile = fopen ( infile, "rb" );
    if (ifile==NULL)
    	{
	    	printf("Couldn't open file '%s'\n", infile);
	    	return 1;
    	}

    ofile = fopen ( outfile, "wb" );
    if (ofile==NULL)
    	{
	    	printf("Couldn't create file '%s'\n", outfile);
	    	fclose ( ifile );
	    	return 1;
    	}

    mem = malloc ( CHUNKSIZE );

    fseek ( ifile, 0, SEEK_END );
    filesize = sz = ftell ( ifile );
    fseek ( ifile, 0, SEEK_SET );

    if (!gasm)
    	{
		    fprintf (ofile, "unsigned char %s[%d] =\n\t{\n", label, sz);
	    } else
	    {
		    if (underscore)
		    	{
						fprintf(ofile, "\t.global _%s\n\t.data\n\t.type _%s,object\n\t.size _%s,%d\n",label,label,label,sz);
						fprintf(ofile, "_%s:\n",label);
		    	} else
					{
						fprintf(ofile, "\t.global %s\n\t.data\n\t.type %s,object\n\t.size %s,%d\n",label,label,label,sz);
						fprintf(ofile, "%s:\n",label);
					}
			}

		if (gasm)
			{
    		while (sz>0)
    			{
	    			int rd,i;
	    			char outline[CHUNKSIZE*10];
	    			char bla[32];

	    			strcpy ( outline, "\t.byte " );
	    			rd = fread ( mem, 1, CHUNKSIZE, ifile );

	    			for (i=0;i<rd;i++)
	    				{
		    				sz--;
		    				if (i==rd-1)
		    					sprintf ( bla, "0x%02x\n", mem[i] );
		    				else
		    					sprintf ( bla, "0x%02x,", mem[i] );

		    				strcat ( outline, bla );
	    				}
	    			fprintf(ofile, outline);
    			}
    		fprintf (ofile, "\n\n");
    	} else
    	{
	    	while (sz>0)
	    		{
		    		int rd,i;
		    		char outline[CHUNKSIZE*10];
		    		char bla[32];

		    		strcpy ( outline, "\t\t" );
		    		rd = fread ( mem, 1, CHUNKSIZE, ifile );

		    		for (i=0;i<rd;i++)
		    			{
			    			sz--;
			    			if (sz==0)
			    				sprintf (bla, "0x%02x", mem[i]);
			    			else
			    				sprintf (bla, "0x%02x,", mem[i]);
			    			strcat ( outline, bla );

			    			if (i==rd-1)
			    				strcat ( outline, "\n" );
		    			}
		    		fprintf ( ofile, outline );
	    		}
	    	fprintf (ofile, "\t};\n\n");
    	}

    free ( mem );
   	fclose ( ifile );
   	fclose ( ofile );

   	if (headerok)
   		{
	   		char labelu[256];

	   		strupor(labelu, label);
	   		ofile = fopen ( headerfile, "wb" );
	   		if (ofile==NULL)
	   			{
		   			printf("Couldn't create file '%s'\n", headerfile);
		   			return 1;
	   			}

	   		fprintf (ofile, "#ifndef __%s_H__\n#define __%s_H__\n\n", labelu, labelu);
	   		fprintf (ofile, "extern unsigned char %s[%d];\n\n", label, filesize);
	   		fprintf (ofile, "#endif\n\n");

	   		fclose ( ofile );
   		}

		return 0;
	}

