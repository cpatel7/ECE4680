#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE 	*fpt;
	
	fpt = fopen("test.txt", "w");
	if(fpt == NULL)
	{
		printf("Error!");   
		exit(1);             
    }
    
	fprintf(fpt, "A RAT SAT ON A CAR");
	
    fclose(fpt);

	return 0;
}
