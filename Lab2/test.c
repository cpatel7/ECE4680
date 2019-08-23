#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int 	i;
	FILE 	*fpt;
	
	fpt = fopen("test.txt", "w");
	if(fpt == NULL)
	{
		printf("Error!");   
		exit(1);             
    }
    
    for(i=0; i<257; i++)
    {
    	fprintf(fpt, "A");
    }
    fclose(fpt);

	return 0;
}
