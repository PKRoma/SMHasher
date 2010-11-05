#include "SpeedTest.h"

#include "Random.h"

#include <stdio.h>  // for printf
#include <intrin.h> // for __rdtsc

//-----------------------------------------------------------------------------
// 256k blocks seem to give the best results.

void BulkSpeedTest ( pfHash hash )
{
	const int trials = 9999;
	const int blocksize = 256 * 1024;

	printf("Bulk speed test - %d-byte keys\n",blocksize);

	char * block = new char[blocksize + 16];

	rand_p(block,blocksize+16);

	uint32_t temp[16];

	for(int align = 0; align < 8; align++)
	{
		double bestbpc = 0;

		for(int itrial = 0; itrial < trials; itrial++)
		{
			__int64 begin,end;

			begin = __rdtsc();

			hash(block + align,blocksize,itrial,temp);

			end = __rdtsc();

			blackhole(temp[0]);

			double cycles = double(end-begin);
			double bpc = double(blocksize) / cycles;
			if(bpc > bestbpc) bestbpc = bpc;
		}

		double bestbps = (bestbpc * 3000000000.0 / 1048576.0);
		printf("Alignment %2d - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",align,bestbpc,bestbps);
	}

	delete [] block;
}
