#include <stdio.h>
#include <stdlib.h>

#define MAXBYTES 255

int main(int argc, char *argv[])
{
	int 	i;
	FILE 	*fpt;
	
	if(argc != 3)
	{
		printf("usage: ./prog [compress/decompress] [filename]");
		exit(0);
	}
	
	/* open file for reading */
	fpt = fopen(argv[2], "rb");
	if(fpt == NULL)
	{
		printf("ERROR! Unable to open %s", argv[2]);
		exit(0);
	}
	
	/* Don't need to create functions for compress and decompress since we are only using this code once */
	if (strcmp(argv[1], "compress") == 0)
	{
		/* RLE compression algorithm */
	
		int 			runCount = 0; 
		unsigned char 	valA, valB;
		FILE 			*compressedfpt;
		
		/* open the new file to write compressed data */
		compressedfpt = fopen("compressed.bin", "wb");
		if(compressedfpt == NULL)
		{
			printf("ERROR! Unable to open compressed.bin");
			exit(0);
		}
		
		/* read the first byte into valA for comparison in the loop */
		fread(&valA, sizeof(valA), 1, fpt);
		runCount++;
		
		/* read from file one byte at a time */
		while(fread(&valB, sizeof(valB), 1, fpt) == 1)
		{
			if(valA == valB)
			{
				if (runCount == MAXBYTES)
				{
					fwrite(&runCount, sizeof(unsigned char), 1, compressedfpt);
					fwrite(&valA, sizeof(valA), 1, compressedfpt);
					valA = valB;
					runCount = 1; //reset to one to account for valB which has already been read in
				}
				else
				{
					runCount++;
				}
			}
			else
			{
				fwrite(&runCount, sizeof(unsigned char), 1, compressedfpt);
				fwrite(&valA, sizeof(valA), 1, compressedfpt);
				valA = valB;
				runCount = 1; //reset to one to account for valB which has already been read in
			}
		}
		if (feof(fpt))
		{
			/* we hit the end of file */
			fwrite(&runCount, sizeof(unsigned char), 1, compressedfpt);
			fwrite(&valA, sizeof(valA), 1, compressedfpt);
			fclose(compressedfpt);
		}
		else
		{
			printf("Unknown error while reading the file stream! \n");
			fclose(fpt);
			exit(0);
		}
	}
	else if (strcmp(argv[1], "decompress") == 0)
	{
		/* RLE Decompression algorithm */
		int 			runCount = 0; 
		unsigned char 	valuedata[2], val;
		FILE 			*decompressedfpt;
		
		decompressedfpt = fopen("decompressed.bin", "wb");
		if(decompressedfpt == NULL)
		{
			printf("ERROR! Unable to open decompressed.bin");
			exit(0);
		}
		
		/* read the the file data in pairs of runCounts and Values */
		while(fread(&valuedata, sizeof(valuedata), 1, fpt) == 1)
		{
			runCount 	= (int)valuedata[0]; //the count of similar bytes
			val 		= valuedata[1]; //the byte value
			
			for(i=0; i<runCount; i++)
			{
				fwrite(&val, sizeof(unsigned char), 1, decompressedfpt);			
			}
		}
		if (feof(fpt))
		{
			/* we hit the end of file */
			fclose(decompressedfpt);
		}
		else
		{
			printf("Unknown error while reading the file stream! \n");
			fclose(fpt);
			exit(0);
		}
	}
	else
	{
		printf("usage: ./prog [compress/decompress] [filename.ppm]");
		fclose(fpt);
		exit(0);
	}
	fclose(fpt);
	return 0;
}
	
	
	
	
	
	
	
	
	
	
