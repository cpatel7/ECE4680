#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	unsigned char	byte;
	FILE 			*fpt;
	
	fpt = fopen(argv[1], "rb");
	if(fpt == NULL)
	{
		printf("Error!");   
		exit(1);             
    }
    
    
    while(fread(&byte, sizeof(byte), 1, fpt) == 1)
    {
    	printf("%c, ", byte);
    }
    printf("\n");
    fclose(fpt);

	return 0;
}
