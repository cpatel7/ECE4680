#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int 	uint_t;
typedef unsigned char	uchar_t;

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


/* function prototypes for the huffman tree */

node_t *buildHuffmanTree(heap_t *);
void updateFtableCodes(node_t *, frequencyNode_t **, int, uint_t);
void updateCode(node_t *, frequencyNode_t **, uint_t, int);

/* function prototypes for the min heap and frequency table */

node_t *getMinNode(heap_t *);
void insertNode(node_t *, heap_t *);
void reheapify(heap_t *, int);
heap_t *buildMinHeap(int, frequencyNode_t **);
frequencyNode_t **buildFrequencyTable(uchar_t *, long);

/* function prototypes for compression and decompression */

void compress(uchar_t *,FILE *, long);
void encode(uchar_t *,FILE *, long, frequencyNode_t **);
void decompress(uchar_t *, FILE *, long);
void decode(uchar_t *, FILE *, long, node_t *);

frequencyNode_t ** rebuildFrequencyTable(uchar_t *, uint_t *);

/* utility funcions */
void printFreqTable(frequencyNode_t **);
void printHeap(heap_t *); 


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

/*
	builds huffman tree from heap
*/

node_t *buildHuffmanTree(heap_t *heap)
{
	node_t *root, *left, *right, *internal;

	//remove two min nodes and combine them into 1 parent node.
	//keep going until only one root node is left.
	while(heap->size > 1)
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
	
	root = getMinNode(heap);

	return root;
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

/*
	inserts the internal node to the heap
*/

void insertNode(node_t *node, heap_t *heap)
{
	node_t *parent, *child, *temp;
	heap->size++;

	int p = heap->size/2;		// this is the location of a parent node in the heap

	//insert node to the rightmost position in heap.
	heap->nodes[heap->size] = node;					
	
	parent 	= heap->nodes[p];
	child	= heap->nodes[heap->size-1];
	//filter up the heap based on frequency value
	while(p > 0)
	{
		if(parent->frequency > child->frequency)
		{
			//swap parent and child
			temp 	= child;
			child 	= parent;
			parent 	= temp;

			p = p/2;
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

	node_t *parent, *leftchild, *rightchild, *curr, *temp;

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
		temp 	= parent;
		parent 	= curr;
		curr	= temp;

		reheapify(heap, smallest);		
	}
}

/*
	gets the varying bits code from the huffman tree and adds it to the frequency table in the code field
*/

// the node that is passed into the get Varying Bits function is the root of the Huffman Tree
void updateFtableCodes(node_t *node, frequencyNode_t **freqTable, int numbits, uint_t code)
{	

	/* 	-go throught the frequency table, for each symbol--traverse the tree and find the varying bit pattern. 
	*  	-Save this pattern in the code column for the corresponding symbol.
	*	-we will find the pattern by doing a bottom up approach through the tree.	
	*	-use bit masks to change bits in desired position
	*	-increase numbits as we continue traversing up until we reach root.
	*/
	
	// if the node is not a leaf
	// set a 0 for traversing the left nodes
	
	/* set code to 0 first so that its easier to change bits to 1 where needed */
	
	
	if (node->left != NULL) { 
	  
		//increase numbits but don't do anything to code since we already have a 0 at the relevant bit location
		
		updateFtableCodes(node->left, freqTable, numbits+1, code); 
	} 
	  
	// agian, the node is not a leaf
        // set 1 for traversing the right node
	if (node->right != NULL) { 
	  
		code = code | (1 << numbits); 
		updateFtableCodes(node->right, freqTable, numbits+1, code); 
	} 
	  
	// once the leaf node has been reached, copy the temporary array
	// to the code field of the frequency table
	// the node's character must match the character in the frequency table
	if (node->left == NULL && node->right == NULL) {
		// the temporary array stores the code for the corresponding character
		// or the symbol in the node struct
	  	updateCode(node,freqTable, code, numbits);
		
	} 
	
}

void updateCode(node_t *node, frequencyNode_t **freqTable, uint_t code, int numbits) {
	
	freqTable[node->symbol]->code 		= code;
	freqTable[node->symbol]->numbits 	= numbits;	

}


/* Builds the frequency table that contains the symbols and their corresponding frequencies
 * as well as the codes for the varying bits for each symbol.
 */

frequencyNode_t ** buildFrequencyTable( uchar_t *file, long filesize)
{
	frequencyNode_t	**ftable;
	uchar_t			i;

	ftable = (frequencyNode_t **)calloc(1, sizeof(frequencyNode_t *));
	
	/* calloc a frequency node for each byte value from 0-255 */
	for(i=0; i<256; i++)
	{
		ftable[i] 	= (frequencyNode_t *)calloc(1, sizeof(frequencyNode_t));
	}

	/* update the frequencies of each symbol that appears in the file. */

	for(i=0; i<filesize; i++)
	{
		ftable[file[i]]->frequency++;
	}

	return ftable;
}

/* rebuilds the frequency table that contains the symbols and their corresponding frequencies
 * as well as the codes for the varying bits for each symbol.
 * this function takes the header from the compressed file with the frequency counts
 * and recreates the frequency table, where the indexes correspond to each character
 */

frequencyNode_t ** rebuildFrequencyTable(uchar_t *file)
{
	frequencyNode_t	**ftable;
	uchar_t			i;

	ftable = (frequencyNode_t **)calloc(1, sizeof(frequencyNode_t *));
	
	/* calloc a frequency node for each byte value from 0-255 */
	for(i=0; i<256; i++)
	{
		ftable[i] 	= (frequencyNode_t *)calloc(1, sizeof(frequencyNode_t));
	}

	/* update the frequencies of each symbol that appears in the file. */

	for(i=0; i<256; i++)
	{
		ftable[i]->frequency = (uint_t)file[i];
	}

	return ftable;
}

/* builds a minimum heap of the given size from the frequency table */

heap_t *buildMinHeap(int size, frequencyNode_t **ftable)
{
	node_t 	*node;
	heap_t 	*heap;
	int	i;

	heap = (heap_t *)calloc(1, sizeof(heap_t));

	heap->nodes	= (node_t **)calloc(size, sizeof(node_t *));
	heap->size	= 0;

	for(i=0; i<256; i++)
	{
		if(ftable[i] != NULL)			
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
		printf(" index %d : frequency %d\n", index, ftable[i]->frequency);
	}

}

/*  
 * encode helper function takes an input file, compares each character in the 
 * file to those in a given frequency table
 * the function iterates over the size of the input file, and writes the code 
 * that corresponds to a character to the output file, effectively encoding each character
*/

void encode(uchar_t *in, FILE *out, long fsize, frequencyNode_t **frequencyTable)
{

	// for iterating through each character in the input file
	int i;
	uchar_t bit;	// single character to be written to the file 
	uchar_t symbol;
	
	for(i=0; i<256; i++)
	{
		bit = (uchar_t)frequencyTable[i]->frequency;
		fwrite(&bit, 1, sizeof(uchar_t), out);
	}
	
	for (i = 0; i < fsize; i++)
	{
		symbol = in[i];
		bit = (uchar_t)frequencyTable[symbol]->code;
		fwrite(&bit, 1, sizeof(uchar_t), out);	
		
	}

}


void compress(uchar_t *in, FILE *out, long fsize) 
{

	frequencyNode_t 	**ftable;
	heap_t 			*min_heap;
	node_t	 		*root;
	uint_t			code;
	
	code = 0;
	// create the frequency table from the input file (need file size)
	ftable = buildFrequencyTable(in, fsize);
	

	// build min heap (huffman tree uses the min heap)
	// the size is the number of rows in the frequency table
	min_heap = buildMinHeap(fsize, ftable);
	

	// build huffman tree (returns the root to be used by next step)
	root = buildHuffmanTree(min_heap);
	
	// use get varying bits to update the codes in the frequency table
	updateFtableCodes(root, ftable, 0, code);
	
	// after updating the frequency table with the codes
	// use the encode function to write out to the file
	
	encode(out,fsize,ftable);
}

// the decompression algorithm takes the input file saved in memory and reads the header
// the header contains all of the frequencies of the characters, with the index
// corresponding to the ASCII value of the character
// from the frequencies, the frequency table is rebuilt


// decode takes the Huffman tree and the input file to look for codes 
// read the bits in order that they appear in the compressed file
// move through the huffman tree, left if the bit is a 0, right if it is a 1
// stop once a leaf has been reached in the tree, at which point the character has been found
// take the symbol value from that leaf and write it to the output file

// byte anded with 1 shifted left by 7 - byte_count
// if and value is 0 go left
// if 1 go right in tree
void decode(uchar_t *in, FILE *out, long fsize, node_t *root)
{
	uchar_t symbol;	// symbol to be written
	uchar_t byte;	// byte read from the file
	node_t *temp;
	int i;
	uint_t bit;
	
	temp = root;
	
	// read the information past the header in the compressed file
	
	for (i = 256; i < fsize; i++) 
	{
		byte = in[i];	// read byte by byte through the input file
		
		
		// check each bit in the byte, and move through the tree accordingly
		while (bit < 8) 
		{
			// if 0, move left in tree
			if (byte && (1 << (7 - bit)) == 0)
			{
				temp = temp->left;
			} 
			// otherwise, move right in the tree
			else 
			{
				temp = temp->right;
			}
			
			// when a leaf node has been reached
			if (temp->left == NULL && temp-> right == NULL)
			{
				symbol = temp->symbol;
				fwrite(&symbol,sizeof(uchar_t,1,out);
				
				// reset to the root of the tree
				temp = root;
			}
			
			bit ++;
		}
		
		bit = 0;
	}	
	
	
}

void decompress(uchar_t *in, FILE *out, long fsize) 
{

	frequencyNode_t		 **ftable;
	heap_t 			*min_heap;
	node_t	 		*root;
	uint_t			code;
	
	code = 0;
	// create the frequency table from the compressed file (need file size)
	ftable = rebuildFrequencyTable(in);

	// build min heap (huffman tree uses the min heap)
	// the size is the number of rows in the frequency table
	min_heap = buildMinHeap(fsize, ftable);
	

	// build huffman tree (returns the root to be used by next step)
	root = buildHuffmanTree(min_heap);
	
	// use get varying bits to update the codes in the frequency table
	updateFtableCodes(root, ftable, 0, code);
	// decode the commpressed file
	// write the corresponding symbols into the decompressed file
	decode(in,out,fsize,root);
	
}

