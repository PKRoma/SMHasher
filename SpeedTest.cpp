#include "SpeedTest.h"

#include "Random.h"

#include <stdio.h>  // for printf

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
      int64_t begin,end;

      begin = rdtsc();

      hash(block + align,blocksize,itrial,temp);

      end = rdtsc();

      blackhole(temp[0]);

      double cycles = double(end-begin);
      if(cycles > 0)
      {
        double bpc = double(blocksize) / cycles;
        if(bpc > bestbpc) bestbpc = bpc;
      }
    }

    double bestbps = (bestbpc * 3000000000.0 / 1048576.0);
    printf("Alignment %2d - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",align,bestbpc,bestbps);
  }

  delete [] block;
}

//-----------------------------------------------------------------------------

void TinySpeedTest ( pfHash hash, int hashsize, int keysize, bool verbose, double & outCycles )
{
  const int trials = 100000;

  if(verbose) printf("Small key speed test - %4d-byte keys - ",keysize);

  uint8_t * h = new uint8_t[hashsize];
  uint8_t * k = new uint8_t[keysize];

  double bestcycles = 1e9;

  for(int itrial = 0; itrial < trials; itrial++)
  {
    int64_t begin,end;

    rand_p(k,keysize);

    begin = rdtsc();
    
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);

    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);

    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);

    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);
    hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);   hash(k,keysize,itrial,h);

    end = rdtsc();

    //blackhole(*(uint32_t*)(&h));

    double cycles = double(end-begin) / 64;
    if((cycles > 0) && (cycles < bestcycles)) bestcycles = cycles;
  }

  double bestbpc = double(keysize) / bestcycles;
  if(verbose) printf("%8.2f cycles/hash, %8.4f bytes/cycle\n",bestcycles,bestbpc);

  outCycles = bestcycles;
}

//-----------------------------------------------------------------------------
