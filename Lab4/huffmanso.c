#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int 	uint_t;
typedef unsigned char	uchar_t;

#define MAXCODELEN	sizeof(uint_t) * 8;

typedef struct huffmanNode
{
	uint_t			frequency;
	uchar_t 		symbol;
	struct huffmanNode 	*right;
	struct huffmanNode 	*left;
} node_t;

typedef struct frequencyNode
{
	uint_t	frequency;
	uint_t	numbits;
	uint_t	code;				// given by updateFtableCodes function
} frequencyNode_t;

typedef struct minHeap
{
	int	   size;
	node_t 	**nodes; 			//array of nodes for the heap. Parent = i/2, left = 2*i, right = 2*i+1;
} heap_t;


/* huffman tree function prototypes */

node_t *buildHuffmanTree(heap_t *); //builds a huffman tree from the min heap


/* Frequency table function prototypes */

frequencyNode_t **buildFrequencyTable(uchar_t *, long);
void updateFtableCodes(node_t *, frequencyNode_t **, int, uint_t);
uint_t getNumUniqueSymbolsInFile(frequencyNode_t **);

/* Min heap function prototypes */

node_t *getMinNode(heap_t *);
void insertNode(node_t *, heap_t *);
void reheapify(heap_t *, int);
heap_t *buildMinHeap(frequencyNode_t **, uint_t);

/* Compression and decompression function prototypes */
void compress(uchar_t *,FILE *, long);
void encode(uchar_t *, long, FILE *, frequencyNode_t **, uint_t);
void decompress(uchar_t *, FILE *, long);
void decode(uchar_t *, FILE *, long, frequencyNode_t **);

/* utility functions */
void PrintHuffmanTree(node_t *, int);
void printHeap(heap_t *);
void printFreqTable(frequencyNode_t **);

int main(int argc, char *argv[])
{
	FILE 	*fpt;
	long 	filesize;
	uchar_t	*file;
	
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
	
	// create filesize var
	// read file into memory and close the file
	// array of unsigned char

	fseek(fpt, 0, SEEK_END);
	filesize = ftell(fpt);
	rewind(fpt);

	file = (uchar_t *)calloc(filesize, sizeof(uchar_t));
	fread(file, sizeof(uchar_t), filesize, fpt);
	fclose(fpt);
	
	/* Don't need to create functions for compress and decompress since we are only using this code once */
	if (strcmp(argv[1], "compress") == 0)
	{
		FILE	*compressedfpt;

		
		/* open the new file to write compressed data */
		compressedfpt = fopen("compressed.bin", "wb");
		if(compressedfpt == NULL)
		{
			printf("ERROR! Unable to open compressed.bin\n");
			exit(0);
		}
		
		// call the compress function with the file pointer
		// the compress funciton has a void return type
		// compress() contains encode() which uses fwrite() 
		// to write to a file (compressedfpt)
		
		compress(file, compressedfpt, filesize);
		fclose(compressedfpt);
		free(file);	
	}
	
	else if (strcmp(argv[1], "decompress") == 0)
	{
		FILE	*decompressedfpt;
		

		
		/* open the new file to write compressed data */
		decompressedfpt = fopen("decompressed.bin", "wb");
		if(decompressedfpt == NULL)
		{
			printf("ERROR! Unable to open decompressed.bin\n");
			fclose(fpt);
			exit(0);
		}
		
		//decompress(file, decompressedfpt, filesize);
		fclose(decompressedfpt);
		free(file);	
	}
	
	else
	{
		printf("usage: ./prog [compress/decompress] [filename.ppm]\n");
		exit(0);
	}
	
	return 0;
}

/*********************************************************************************************/

/*
	builds huffman tree from heap
*/
node_t *buildHuffmanTree(heap_t *heap)
{
	node_t *root, *left, *right, *internal;

	//remove two min nodes and combine them into 1 parent node.
	//keep going until only one root node is left.
	while(heap->size > 2) //we do not want to build an internal node with the root.
	{
		left 	= getMinNode(heap);
		right	= getMinNode(heap);

		//we have the two min nodes, now create an internal node with the frequency being
		//the sum of the two frequencies
		internal 			= (node_t *)calloc(1, sizeof(node_t));
		internal->frequency = left->frequency + right->frequency;
		internal->left		= left;
		internal->right		= right;

		//put the internal node back into the heap
		insertNode(internal, heap); 
	}
	
	root = heap->nodes[1]; //return the root to the new heap which is the huffman tree now.

	return root;
}

/*********************************************************************************************/

heap_t *buildMinHeap(frequencyNode_t **ftable, uint_t numsymbols)
{
	node_t 	*node;
	heap_t 	*heap;
	int	i;

	heap = (heap_t *)calloc(1, sizeof(heap_t));

	heap->nodes	= (node_t **)calloc(numsymbols + 1, sizeof(node_t *)); //we are doing size + 1 because root starts at index 1.
	heap->size	= 1; //root starts at 1

	for(i=0; i<256; i++)
	{
		if(ftable[i]->frequency > 0) //only build heap if symbol exists	in file at least once		
		{
			node = (node_t *)calloc(1, sizeof(node_t));

			node->symbol 	= (uchar_t)i;
			node->frequency	= ftable[i]->frequency;
			node->left 		= NULL;
			node->right 	= NULL;

			insertNode(node, heap);
		}
	}

	return heap;
}

/*
	inserts the internal node to the heap
*/

void insertNode(node_t *node, heap_t *heap)
{
	//node_t *parent, *child, *temp;


	int p, i = heap->size;
	node_t *parent, *curr, temp;

	//insert the node into the end of the heap array
	heap->nodes[i] = node;
	heap->size++;
	
	p		= i/2; //this is the parent index
	
	while(i > 1)
	{
		parent 	= heap->nodes[p];
		curr	= heap->nodes[i];
		
		if(curr->frequency < parent->frequency)
		{
			//swap them and update the respective indices
			temp 	= *parent;
			*parent	= *curr;
			*curr	= temp;
			
			i = p; //the current index now becomes parent's index
			p = p/2; //new parent's index
		}
		else
		{
			break;
		}
	}
}

/*
	restructures the heap to maintain its min heap properties ****** base case for the recursion ******
*/

void reheapify(heap_t *heap, int smallest)
{
	int left	 = 2 * smallest;
	int right	 = 2 * smallest + 1;

	node_t *parent, *leftchild, *rightchild, *curr, temp;

	parent 		= heap->nodes[smallest];
	curr		= heap->nodes[smallest];			// smallest is the smallest index
	leftchild	= heap->nodes[left];
	rightchild	= heap->nodes[right];

	if(left < heap->size && leftchild->frequency < curr->frequency)
	{
		curr 		= leftchild;

		smallest 	= left;
	}

	if(right < heap->size && rightchild->frequency < curr->frequency)
	{
		curr 		= rightchild;

		smallest	= right;
	}

	if (curr != parent)
	{
		temp 	= *parent;
		*parent = *curr;
		*curr	= temp;

		reheapify(heap, smallest);		
	}
}

/*
	gets the root node of the min heap
*/

node_t *getMinNode(heap_t * heap)
{
	node_t *root;

	root = heap->nodes[1];

	heap->nodes[1] = heap->nodes[heap->size - 1]; //the last leaf node becomes root
	heap->size--;

	reheapify(heap, 1);

	return root;
}

/*********************************************************************************************/

frequencyNode_t ** buildFrequencyTable( uchar_t *file, long filesize)
{
	frequencyNode_t **ftable;
	int 			i;

	ftable = (frequencyNode_t **)calloc(256, sizeof (frequencyNode_t *));

	for(i=0; i<256; i++)
	{
		ftable[i] = (frequencyNode_t *)calloc(1, sizeof(frequencyNode_t));
	}
	
	//update frequencies for each symbol
	for(i=0; i<filesize; i++)
	{
		ftable[file[i]]->frequency++;
	}
	
	return ftable;
}

/* counts the number of unique symbols in the file by checking if the frequencies of the said symbols in the frequency table is greater than 0 */

uint_t getNumUniqueSymbolsInFile(frequencyNode_t **ftable)
{
	int i;
	uint_t numsymbols = 0;
	
	for(i=0; i<256; i++)
	{
		if(ftable[i]->frequency > 0)
		{
			numsymbols++;
		}
	}
	
	return numsymbols;
}

/*
	gets the varying bits code from the huffman tree and adds it to the frequency table in the code field
*/

// the node that is passed into the get Varying Bits function is the root of the Huffman Tree
void updateFtableCodes(node_t *node, frequencyNode_t **ftable, int numbits, uint_t code)
{	
	// if the node is not a leaf
	// set a 0 for traversing the left nodes	
	
	if (node->left != NULL) 
	{ 
	  
		//increase numbits but don't do anything to code since we already have a 0 at the relevant bit location
		
		updateFtableCodes(node->left, ftable, numbits+1, code); 
	} 
	  
	// agian, the node is not a leaf
    // set 1 for traversing the right node
	if (node->right != NULL)
	 { 
	  
		code = code | (1 << numbits); 
		updateFtableCodes(node->right, ftable, numbits+1, code); 
	} 
	  
	// once the leaf node has been reached, copy the temporary array
	// to the code field of the frequency table
	// the node's character must match the character in the frequency table
	if (node->left == NULL && node->right == NULL) 
	{
		ftable[node->symbol]->code 		= code;
		ftable[node->symbol]->numbits	= numbits;		
	} 
	
}

/*********************************************************************************************/

void compress(uchar_t *file, FILE *compressedfpt, long fsize) {

	frequencyNode_t **ftable;
	heap_t 			*min_heap;
	node_t	 		*root;
	uint_t			code;
	uint_t			numsymbols;		
	
	code = 0;
	// create the frequency table from the input file (need file size)
	ftable = buildFrequencyTable(file, fsize);
	
	numsymbols = getNumUniqueSymbolsInFile(ftable);

	// build min heap (huffman tree uses the min heap)
	// the size is the number of rows in the frequency table
	min_heap = buildMinHeap(ftable, numsymbols);
	
	//printHeap(min_heap);
	
	// build huffman tree (returns the root to be used by next step)
	root = buildHuffmanTree(min_heap);
	
	PrintHuffmanTree(root, 0); 
	
	// use get varying bits to update the codes in the frequency table
	updateFtableCodes(root, ftable, 0, code);
	
	printFreqTable(ftable);
	
	// after updating the frequency table with the codes
	// use the encode function to write out to the file
	
	//--working till here
	
	//encode(file, fsize, compressedfpt,ftable, numsymbols);
}

/*  
 * encode helper function takes an input file, compares each character in the 
 * file to those in a given frequency table
 * the function iterates over the size of the input file, and writes the code 
 * that corresponds to a character to the output file, effectively encoding each character
*/

void encode(uchar_t *file, long filesize, FILE *compressedfpt, frequencyNode_t **ftable, uint_t numsymbols) 
{
	int i, bitpos = 0, codelen;
	uint_t code;
	uchar_t byte, buff = 0;
	
	fwrite(&numsymbols, 1, sizeof(numsymbols), compressedfpt); //write the number of unique symbols in the header
	
	for(i=0; i<256; i++)
	{
		if(ftable[i]->frequency > 0) //we are only writing if the frequency is greater than 0
		{
			byte = (uchar_t)ftable[i]->frequency;
			fwrite(&byte, 1, sizeof(uchar_t), compressedfpt);
		}
	}
	
	for (i = 0; i < filesize; i++)
	{
		byte = file[i];
		
		if(ftable[byte])
		{
			code 	= ftable[byte]->code;
			codelen = ftable[byte]->numbits; 
			while (bitpos < 8)
			{
				if(codelen <= 8)
				{
					buff = buff | (code << (8-codelen));
				}
				 
			}
			fwrite(&buff, 1, sizeof(uchar_t), compressedfpt);
		}
	}
}

/*********************************************************************************************/

void PrintHuffmanTree(node_t *node, int level)
{
	int i;
	if (node == NULL) return;
	PrintHuffmanTree(node->right, level + 1);
	for (i = 0; i < level; i++) printf("   ");
	printf("%3d\n", node->frequency);
	PrintHuffmanTree(node->left, level + 1);
}

void printHeap(heap_t *heap) 
{
	int i;
	for (i = 0; i < heap->size; i++) 
	{
		if (heap->nodes[i] != NULL ) 
		{
			printf(" frequency: %d\n", heap->nodes[i]->frequency);
		}
	}
}

// print frequency table (symbol and the frequency)
void printFreqTable(frequencyNode_t **ftable) 
{
	int i;
	
	for (i = 0; i < 256; i++) 
	{
		if(ftable[i]->frequency > 0)
		{
			printf(" code %u : numbits %u\n", ftable[i]->code, ftable[i]->numbits);
		}
	}
}


























