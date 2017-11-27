#ifndef MEM_FUNC_C
#define MEM_FUNC_C

#include <time.h>
#include <stdlib.h>

int allocateMemory( unsigned int totMem, unsigned int& startMemory, unsigned int blockSize )
{	
	unsigned int start = startMemory;

	if( startMemory > totMem )
	{
		startMemory = 0;
	}

	startMemory += blockSize;

	return start;
}

#endif