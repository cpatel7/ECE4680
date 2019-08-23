#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	unsigned short	twobyte;
	FILE 			*fpt;
	
	fpt = fopen(argv[1], "rb");
	if(fpt == NULL)
	{
		printf("Error!");   
		exit(1);             
    }
    
    
    while(fread(&twobyte, sizeof(twobyte), 1, fpt) == 1)
    {
    	printf("%u, ", twobyte);
    }
    printf("\n");
    fclose(fpt);

	return 0;
}
