#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#define ROWS 256
#define COLS 256
#define MAXBUFF 100

typedef unsigned int 	uint_t;
typedef unsigned char 	uchar_t;
typedef unsigned short	ushort_t;

/* function prototypes */
void 	parseheader(FILE *, uint_t []);	//parses header and reads # of vertices and # of faces.
float 	**getvertices(FILE *, uint_t);
uint_t 	**getfaces(FILE *, uint_t);
float 	getboundingbox(float **, uint_t, float [], float [], float []);
float 	*getdiffvector(float [], float []);
float 	*getcamera(float [3], float, float, float, float, float [3]);	//gets the camera position and orientation
void	rotate(float [3], float [3][3]);
void 	getcoordinates(float [3], float [3], float [3], float [3], float [3], float [3], float [3], float [3], float, float); //gets the 3d coordinates bounding the image
float	getplaneequation(float [3], float *, float *, float *);
float 	*disttotriangle(float [3], float [3], float [3], float);
void 	wrtiePPM(uchar_t [ROWS][COLS], ushort_t);
void	cleanup(float **, uint_t **, uint_t, uint_t);


float 	getmax(float *);
float 	*multiplybyscalar(float, float []);
float	*crossprod(float [], float []);
float	dotprod(float [], float []);
float	*addvector(float [3], float [3]);


int main(int argc, char *argv[])
{
	FILE 		*fpt;
	uint_t		**faces, numvertices, numfaces, header[2]; //index 0 has # vertices and index 1 has # faces
	float		**vertices, boundmin[3], boundmax[3], boundcenter[3], E, *camera, up[3], x, y, z;
	float		a, left[3], right[3], top[3], bottom[3], topleft[3], *image, zbuffer;
	float 		*v0, *v1, *v2, plane[3], D, *dist, n, d, *intersect, dot1, dot2, dot3;
	int 		r, c, i;
	uchar_t		finalimage[ROWS][COLS];
	ushort_t	maxcolor, color;
		
	if(argc != 5)
	{
		printf("usage: ./prog [filename] [x] [y] [z]\n");
		exit(0);
	}
		
	/* open file for reading */
	fpt = fopen(argv[1], "rb");
	if(fpt == NULL)
	{
		printf("ERROR! Unable to open %s\n", argv[2]);
		exit(0);
	}
	
	/* parse header to get number of vertices and faces */
	
	parseheader(fpt, header);
	numvertices = header[0];
	numfaces	= header[1];
	
	/* get the individual vertices */
	
	vertices = getvertices(fpt, numvertices);
	
	/* get the individual faces */
	
	faces = getfaces(fpt, numfaces);
	fclose(fpt);
/*	
	printf("vertex %u is: %f, %f, %f\n", numvertices, vertices[numvertices-1][0], vertices[numvertices-1][1], vertices[numvertices-1][2]);
	printf("face %u is: %u, %u, %u\n", numfaces, faces[numfaces-1][0], faces[numfaces-1][1], faces[numfaces-1][2]);
*/	
	
	/* get the bounding box and scalar E */
	E = getboundingbox(vertices, numvertices, boundmin, boundmax, boundcenter);
	
//	printf("E :%f\n", E);

	/* read in x y and z coordinates and convert them to floats */
	x = strtof(argv[2], NULL);
	y = strtof(argv[3], NULL);
	z = strtof(argv[4], NULL);

	/*convert degrees to radians */
	x = (M_PI/180) * x;
	y = (M_PI/180) * y;
	z = (M_PI/180) * z;
	
	/* get camera position and orientation */
	up[0] = up[1] = up[2] = 0;
		
	camera = getcamera(up, x, y, z, E, boundcenter);
	
	/* Determine the 3D coordinates bounding the image */
	a = 0;
	getcoordinates(left, right, top, bottom, topleft, boundcenter, camera, up, a, E);
	

	for(r=0; r<ROWS; r++)
	{
		for(c=0; c<COLS; c++)
		{
			/* reset zbuffer for each pixel */
			zbuffer = 999999;

			/* calculate vector coordinates for image pixel */
			image = addvector(topleft, addvector(multiplybyscalar((c/(COLS-1)), getdiffvector(right, left)), multiplybyscalar((r/(ROWS-1)), getdiffvector(bottom, top))));

			for(i=0; i<numfaces; i++)
			{
				v0 = vertices[faces[i][0]];
				v1 = vertices[faces[i][1]];
				v2 = vertices[faces[i][2]];

				D = getplaneequation(plane, v0, v1, v2);

				dist = disttotriangle(plane, camera, image, D);

				n = dist[0];
				d = dist[1];

				if (floor(d) == 0) //skip this triangle
				{
					continue;
				}

				/* find intersecting coordinates of ray and plane */
				intersect = addvector(camera, multiplybyscalar(n/d, getdiffvector(image, camera)));

				/* Determine if intersection point lies within triangle by calculating the three
dot products */

				dot1 = dotprod(crossprod(getdiffvector(v2, v0), getdiffvector(v1, v0)), crossprod(getdiffvector(intersect, v0), getdiffvector(v1, v0)));
				dot2 = dotprod(crossprod(getdiffvector(v0, v1), getdiffvector(v2, v1)), crossprod(getdiffvector(intersect, v1), getdiffvector(v2, v1)));
				dot3 = dotprod(crossprod(getdiffvector(v1, v2), getdiffvector(v0, v2)), crossprod(getdiffvector(intersect, v2), getdiffvector(v0, v2)));

				if (dot1 < 0 || dot2 < 0 || dot3 < 0) //intersection point lies outside the triangle so skip
				{
					continue;
				}
				else if ((n/d) > zbuffer)//triangle lies behind a closer triangle so skip
				{
					continue;
				}
				else
				{
					zbuffer = n/d;

					color = 155 + (i%100);

					if(color > maxcolor)
					{
						maxcolor = color;
					}

					finalimage[r][c] = color; 
				}
			}
		}
	}

	wrtiePPM(finalimage, maxcolor);

	cleanup(vertices, faces, numvertices, numfaces);

	return 0;	
}


/**********************************************FUNCTIONS*************************************/


/* parses the header and saves the number of vertices and number of faces in a header array */

void parseheader(FILE *fpt, uint_t headerdata[])
{
	char	line[MAXBUFF], *tail;
	
	/* read the first 9 lines of the file -- these are the header lines */
	while(fgets(line, MAXBUFF, fpt) != NULL)
	{		
		if(strstr(line, "element vertex") != NULL)
		{
			/* get the numvertices value from the line at index 15*/
			tail = &line[15];
			headerdata[0] = strtoul(tail, &tail, 10);
		}
		
		else if(strstr(line, "element face"))		
		{
			/* get the numfaces value from the line at index 13*/
			tail = &line[13];
			headerdata[1] = strtoul(tail, &tail, 10);
		}
		else if(strstr(line, "end_header"))
		{
			break;
		}
	}
}

/* reads in the vertices in the file and stores them in an array of pointers for x, y and z */

float **getvertices(FILE *fpt, uint_t numvertices)
{
	float **vertices;
	int i;
	char	line[MAXBUFF], *tail;
	
	vertices = (float **)calloc(numvertices, sizeof(float *));
	for (i = 0; i < numvertices; i++)
	{
		vertices[i] = (float *)calloc(3, sizeof(float)); //calloc space for x, y, z
	}
	
	for(i=0; i<numvertices; i++)
	{
		fgets(line, MAXBUFF, fpt);
		
		vertices[i][0] = strtof(line, &tail);		//x
		vertices[i][1] = strtof(tail, &tail);		//y
		vertices[i][2] = strtof(tail, NULL);		//z
	}
	
	return vertices;
}

uint_t **getfaces(FILE *fpt, uint_t numfaces)
{
	uint_t **faces;
	int i;
	char	line[MAXBUFF], *tail;
	
	faces = (uint_t **)calloc(numfaces, sizeof(uint_t *));
	for (i = 0; i < numfaces; i++)
	{
		faces[i] = (uint_t *)calloc(3, sizeof(uint_t)); //calloc space for x, y, z
	}
	
	for(i=0; i<numfaces; i++)
	{
		fgets(line, MAXBUFF, fpt);
		tail = &line[1]; //discard the first value since it is only describing the number of points that follow in the line
	
		faces[i][0] = strtoul(tail, &tail, 10);		//x
		faces[i][1] = strtoul(tail, &tail, 10);		//y
		faces[i][2] = strtoul(tail, NULL, 10);		//z
	}
	
	return faces;
}

/* 	gets the bounding box which contains: 
	(a) Minimum and maximum X, Y and Z (two vectors denoted hmini and hmaxi).
	(b) Center X, Y and Z (vector denoted hcenteri).
	(c) Maximum extent of bounding box E = scalar that is largest component of hmax−
	mini, i.e. largest extent of the three axes.
*/

float getboundingbox(float **vertices, uint_t numvertices, float min[], float max[], float center[])
{
	float 	E, xmin, ymin, zmin, xmax, ymax, zmax, *diff;
	int		i;
	
	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = FLT_MIN;
	
	for(i=0; i<numvertices; i++)
	{
		/* get min co-ordinates */
		if(vertices[i][0] < xmin)
		{
			xmin = vertices[i][0];
		}

		if(vertices[i][1] < ymin)
		{
			ymin = vertices[i][1];
		}
				
		if(vertices[i][2] < zmin)
		{
			zmin = vertices[i][2];
		}
		
		/* get max co-ordinates */
		if(vertices[i][0] > xmax)
		{
			xmax = vertices[i][0];
		}

		if(vertices[i][1] > ymax)
		{
			ymax = vertices[i][1];
		}
				
		if(vertices[i][2] > zmax)
		{
			zmax = vertices[i][2];
		}
	}

			
	min[0] = xmin;
	min[1] = ymin;
	min[2] = zmin;

	max[0] = xmax;
	max[1] = ymax;
	max[2] = zmax;

	center[0] = xmax - (xmax - xmin) / 2;
	center[1] = ymax - (ymax - ymin) / 2;
	center[2] = zmax - (zmax - zmin) / 2;
	
	/* get the difference vector */
	diff = getdiffvector(max, min);
	
	/* E is the max of the diff vector elements */	
	E = getmax(diff);
	
	return E;
}

/* gets the camera position and orientation */

float *getcamera(float up[3], float x, float y, float z, float E, float center[3])
{
	static float camera[3] = {1, 0, 0};
	
	float Rx[3][3]	= { {1, 0, 0}, {0, cos(x), -sin(x)}, {0, sin(x), cos(x)} };
	float Ry[3][3]	= { {cos(y), 0, sin(y)}, {0, 1, 0}, {-sin(y), 0, cos(y)} };
	float Rz[3][3]	= { {cos(z), -sin(z), 0}, {sin(z), cos(z), 0}, {0, 0, 1} };
	
	rotate(camera, Rx);
	rotate(camera, Ry);
	rotate(camera, Rz);
	
	//move and scale the camera vector
	camera[0] = 1.5*E*camera[0] + center[0];
	camera[1] = 1.5*E*camera[1] + center[1];
	camera[2] = 1.5*E*camera[2] + center[2];
	
	//rotate up vector
	rotate(up, Rx);
	rotate(up, Ry);
	rotate(up, Rz);

	return camera;
}

/* gets the 3d coordinates bounding the image */

void getcoordinates(float left[3], float right[3], float top[3], float bottom[3], float topleft[3], float center[3], float camera[3], float up[3], float a, float E)
{
	left 	= crossprod(up, getdiffvector(center, camera));
	a 		= sqrt(pow(left[0], 2) + pow(left[1], 2) + pow(left[2], 2));
	left	= addvector(multiplybyscalar(E/(2*a), left), center);
	
	right	= crossprod(getdiffvector(center, camera), up);
	right	= addvector(multiplybyscalar(E/(2*a), right), center);
	
	top		= addvector(multiplybyscalar(E/2, up), center);
	
	bottom	= addvector(multiplybyscalar((-1*E)/2, up), center);
	
	topleft	= addvector(multiplybyscalar(E/2, up), left); 	
}

/* gets the plane equation from the triangle vertices */

float getplaneequation(float plane[3], float *v0, float *v1, float *v2)
{
	float *temp, D;

	temp = crossprod(getdiffvector(v1, v0), getdiffvector(v2,v0));

	plane[0] = temp[0];
	plane[1] = temp[1];
	plane[2] = temp[2];

	D = dotprod(multiplybyscalar(-1,temp),v0);

	return D;
}

/* get dist to the triangle. Returns vector with n in index 0 and d in index 1 */

float *disttotriangle(float plane[3], float camera[3], float image[3], float D)
{
	static float result[2] = {0};

	result[0] = dotprod(multiplybyscalar(-1, plane), camera) - D;
	result[1] = dotprod(plane, getdiffvector(image, camera));

	return result;
}

/* writes ppm file */

void wrtiePPM(uchar_t image[ROWS][COLS], ushort_t maxcolor)
{
	FILE *ofpt;

	ofpt = fopen("output.ppm", "wb");
	if(ofpt == NULL)
	{
		printf("ERROR! Unable to open output.ppm for writing\n");
		exit(0);
	}

	//write the header for the ppm
	fprintf(ofpt, "P5 %d %d %d ", COLS, ROWS, maxcolor);

	//write the image data
	fwrite(image, sizeof(uchar_t), ROWS * COLS, ofpt);

	//close the output file
	fclose(ofpt);
}

/* frees everything */

void cleanup(float **vertices, uint_t **faces, uint_t numvertices, uint_t numfaces)
{
	int i;

	for (i = 0; i < numvertices; i++) 
	{
		free(vertices[i]);
	}
	free(vertices);

	for (i = 0; i < numfaces; i++)
	{
		free(faces[i]);
	}
	free(faces);
}


/*****************************UTILITY FUNCTIONS****************************/

/* rotate vector based on the 3x3 matrix for rotation */

void rotate(float camera[3], float R[3][3])
{
	int 	i, j;
	float 	rotatedv[3];
	
	rotatedv[0] = rotatedv[1] = rotatedv[2] = 0;
	
    for (i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
        {
            rotatedv[i] += R[j][i] * camera[j];
        }
    }

    for (i=0;i<3;i++)
    {   
        camera[i] = rotatedv[i];
    }
}


/* gets the difference of two vectors v1 and v2 */

float *getdiffvector(float v1[], float v2[])
{
	static float result[3] = {0,0,0};
	
	result[0] = v1[0] - v2[0];
	result[1] = v1[1] - v2[1];
	result[2] = v1[2] - v2[2];
	
	return result;
}

/* gets the max number in a float array */
float getmax(float *ar)
{
	int 	i, len;
	float	max = ar[0];
	
	len = sizeof(ar)/sizeof(ar[0]);
	
	for(i=1; i<len; i++)
	{
		if(ar[1] > max)
		{
			max = ar[1];
		}
	}
	
	return max;
}

float *multiplybyscalar(float scalar, float v[])
{
	static float result[3] = {0,0,0};
	
	result[0] = scalar * v[0];
	result[1] = scalar * v[1];
	result[2] = scalar * v[2];
	
	return result;
}

/* gets the cross product of two vectors of size 3 */

float *crossprod(float v1[], float v2[])
{
	static float result[3] = {0,0,0};
	
	result[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	result[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	result[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	
	return result;
}

/* gets the dot product of two vectors of size 3 */

float dotprod(float v1[], float v2[])
{
	float 	result;
	
	result = 0;
	
	result = ((v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]));

	return result;
	
}

/* adds two vectors of size 3 */

float *addvector(float v1[3], float v2[3])
{
	static float result[3] = {0};
	
	result[0] = v1[0] + v2[0];
	result[1] = v1[1] + v2[1];
	result[2] = v1[2] + v2[2];

	return result;
}


