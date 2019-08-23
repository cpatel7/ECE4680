#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBYTES 255
#define MAXDICTSIZE 65536
#define MAXDICTLEN 1000

/* declare function prototypes */
void InitializeDict(unsigned char *pattern[MAXDICTSIZE], unsigned short *);
void FreeDict(unsigned char **);
int isPbuffinDictionary(unsigned char **, unsigned char *, int);
unsigned short getPCode(unsigned char **, unsigned char *, int);
int isCinDictionary(unsigned char **, unsigned short);
void writeToFile(FILE *, unsigned char *, unsigned short);

int main(int argc, char *argv[])
{
	FILE 	*fpt;
	
	if(argc != 3)
	{
		printf("usage: ./prog [compress/decompress] [filename]\n");
		exit(0);
	}
	
	/* open file for reading */
	fpt = fopen(argv[2], "rb");
	if(fpt == NULL)
	{
		printf("ERROR! Unable to open %s\n", argv[2]);
		exit(0);
	}
	
	/* Don't need to create functions for compress and decompress since we are only using this code once */
	if (strcmp(argv[1], "compress") == 0)
	{
		/* LZW compression algorithm */
	
		unsigned char 	*pattern[MAXDICTSIZE]; 
		unsigned short 	pattern_length[MAXDICTSIZE], code, codeforp, ibuff, ip;
		int 			contains;
		unsigned char 	*p, curr, *pbuff;
		FILE 			*compressedfpt;
		
		code	 	= 256; //since we already have the first 255 characters in the dictionary with their codes
		
		/* open the new file to write compressed data */
		compressedfpt = fopen("compressed.bin", "wb");
		if(compressedfpt == NULL)
		{
			printf("ERROR! Unable to open compressed.bin\n");
			exit(0);
		}
		
		/* intialize p and pbuff */

		p 		= (unsigned char *)calloc(MAXDICTLEN, sizeof(unsigned char));
		pbuff 	= (unsigned char *)calloc(MAXDICTLEN, sizeof(unsigned char));
	
		/* initialize dictionary with all roots */
		
		InitializeDict(pattern, pattern_length);
		
		ibuff	= 0;
		ip		= 0;
				
		while(fread(&curr, sizeof(curr), 1, fpt) == 1)
		{
			/* concatenate p+curr */
	
			memcpy(pbuff, p, ibuff);
			pbuff[ibuff]	= curr;
			ibuff++;
			contains  		= isPbuffinDictionary(pattern, pbuff, ibuff);
			
			if(contains)
			{
				memcpy(p, pbuff, ibuff);
				ip = ibuff; 
			}
			else
			{
				/* get code for p */
				
				codeforp = getPCode(pattern, p, ip);
				fwrite(&codeforp, sizeof(codeforp), 1, compressedfpt);
				memcpy(pattern[code], pbuff, ibuff);
				pattern_length[code]		= ibuff;
				p[0]						= curr;
				ibuff						= 1;
				code++;
			}

		}

		if (feof(fpt))
		{
			/* we hit the end of file */
			ip = 1;
			codeforp = getPCode(pattern, &p[0], ip); 
			fwrite(&codeforp, sizeof(codeforp), 1, compressedfpt);
			fclose(compressedfpt);
		}
		else
		{
			printf("Unknown error while reading the file stream! \n");
			fclose(fpt);
			fclose(compressedfpt);
			exit(0);
		}
		
		free(pbuff);
		free(p);
		//FreeDict(pattern);
	}
	else if (strcmp(argv[1], "decompress") == 0)
	{
		/* LZW Decompression algorithm */

		FILE 			*decompressedfpt;
		unsigned char 	*pattern[MAXDICTSIZE];
		unsigned short 	pattern_length[MAXDICTSIZE], curr, p, code, i;
		int 			contains;
		unsigned char	*xbuff;
		
		code = 256;
		/* intialize currpattern and pbuff */

		xbuff		= (unsigned char *)calloc(MAXDICTLEN, sizeof(unsigned char));


		/* initialize dictionary with all roots */
		
		InitializeDict(pattern, pattern_length);
		
		decompressedfpt = fopen("decompressed.bin", "wb");
		if(decompressedfpt == NULL)
		{
			printf("ERROR! Unable to open decompressed.bin\n");
			exit(0);
		}
		
		fread(&p, sizeof(p), 1, fpt);
		writeToFile(decompressedfpt, pattern[p], pattern_length[p]);
		
		while(fread(&curr, sizeof(curr), 1, fpt) == 1)
		{		
			contains = isCinDictionary(pattern, curr);
		
			if(contains)
			{
				writeToFile(decompressedfpt, pattern[curr], pattern_length[curr]);
				i = pattern_length[p];
				memcpy(xbuff, pattern[p], i);
				
				xbuff[i]		= pattern[curr][0];
				i++;
				memcpy(pattern[code], xbuff, i);
				pattern_length[code] = i;	
				code++;
			}
			else
			{
				i = pattern_length[p];
				memcpy(xbuff, pattern[p], i);
				
				xbuff[i]	= pattern[p][0];
				i++;
				writeToFile(decompressedfpt, xbuff, i);
				
				memcpy(pattern[code], xbuff, i);
				pattern_length[code] = i;
				code++;
			}
			
			p = curr;
		}
		if(feof(fpt))
		{
			fclose(decompressedfpt);
		}
		
		free(xbuff);
		FreeDict(pattern);

	}
	else
	{
		printf("usage: ./prog [compress/decompress] [filename.ppm]\n");
		fclose(fpt);
		exit(0);
	}
	fclose(fpt);
	return 0;
}
	
int isCinDictionary(unsigned char *pattern[], unsigned short code)
{
	if (pattern[code][0] != '\0') 
	{
		return 1;
	}
	return 0;
}

int isPbuffinDictionary(unsigned char *pattern[], unsigned char *pbuff, int ibuff)
{
	int i,j;
	int contains = 1;
	
	for(i=0; i<MAXDICTSIZE; i++)
	{
		for(j=0; j<ibuff; j++)
		{
			if(pbuff[j] != pattern[i][j])
			{
				contains = 0;
				break;
			}
			else
			{
				contains = 1;
			}
		}
		if (contains)
		{
		 	break; //we have found the pattern in dictionary
		}
	}
	
	return contains;
}

unsigned short getPCode(unsigned char *pattern[], unsigned char *p, int ip)
{
	int i,j;
	int contains = 1;
	unsigned short code;
	
	for(i=0; i<MAXDICTSIZE; i++)
	{
		for(j=0; j<ip; j++)
		{
			if(p[j] != pattern[i][j])
			{
				contains = 0;
				break;
			}
			else
			{
				contains = 1;
			}
		}
		if (contains)
		{
			code = i;
		 	break; //we have found the pattern in dictionary
		}
	}
	
	return code;
}

void InitializeDict(unsigned char *pattern[], unsigned short *pattern_length)
{
	int i;
	unsigned char c;
	
	for(i=0; i<MAXDICTSIZE; i++)
	{
		pattern[i] = (unsigned char *)calloc(MAXDICTLEN, sizeof(unsigned char));
	}
	
	c = 0;
	while(1)
	{
		pattern[c][0] 		= c;
		pattern_length[c]	= 1;
		c++;
		
		if (c == 255) break;
	}
	
	for(i=256; i<MAXDICTSIZE; i++)
	{
		pattern_length[i]	= 0;
		pattern[i][0] 		= '\0';
	}
}

void FreeDict(unsigned char *pattern[])
{
	int i;
	
	for(i=0; i<MAXDICTSIZE; i++)
	{
		if(pattern[i] != NULL) free(pattern[i]);
	}
}	

void writeToFile(FILE *fpt, unsigned char *charsequence, unsigned short len)
{
	int i;
	
	for(i=0; i<len; i++)
	{
		fwrite(&charsequence[i], sizeof(unsigned char), 1, fpt);
	}
}
	
	
	
	
	
	
	
	
