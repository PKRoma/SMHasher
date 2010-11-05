#include <stdio.h>

#include "hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"

#include <time.h>
#include <intrin.h>
#include <windows.h>

#pragma warning(disable : 4127) // "conditional expression is constant" in the if()s for avalanchetest

bool g_testAll = false;

/*
bool g_testSanity      = true;
bool g_testSpeed       = true;
bool g_testDiff        = true;
bool g_testAvalanche   = true;
bool g_testCyclic      = true;
bool g_testSparse      = true;
bool g_testPermutation = true;
bool g_testWindow      = true;
bool g_testText        = true;
bool g_testZeroes      = true;
bool g_testSeed        = true;
*/

//*
bool g_testSanity      = false;
bool g_testSpeed       = false;
bool g_testDiff        = false;
bool g_testAvalanche   = false;
bool g_testCyclic      = false;
bool g_testSparse      = false;
bool g_testPermutation = false;
bool g_testWindow      = false;
bool g_testText        = false;
bool g_testZeroes      = false;
bool g_testSeed        = false;
//*/

//-----------------------------------------------------------------------------

struct HashInfo
{
	pfHash hash;
	int hashbits;
	const char * name;
	const char * desc;
};

HashInfo g_hashes[] = 
{
	{ randhash_32,          32, "rand32",      "Random number generator, 32-bit" },
	{ randhash_64,          64, "rand64",      "Random number generator, 64-bit" },
	{ randhash_128,        128, "rand128",     "Random number generator, 128-bit" },

	{ crc32,                32, "crc32",       "CRC-32" },
	{ DoNothingHash,        32, "donothing32", "Do-Nothing Function (only valid for speed test comparison)" },

	{ md5_32,               32, "md5_32a",     "MD5, first 32 bits of result" },
	{ sha1_32a,             32, "sha1_32a",    "SHA1, first 32 bits of result" },

	{ FNV,                  32, "FNV",         "Fowler-Noll-Vo hash, 32-bit" },
	{ lookup3_test,         32, "lookup3",     "Bob Jenkins' lookup3" },
	{ SuperFastHash,        32, "superfast",   "Paul Hsieh's SuperFastHash" },
	
	// MurmurHash2

	{ MurmurHash2_test,     32, "Murmur2",     "MurmurHash2 for x86, 32-bit" },
	{ MurmurHash2A_test,    32, "Murmur2A",    "MurmurHash2A for x86, 32-bit" },
	{ MurmurHash64A_test,   64, "Murmur2B",    "MurmurHash2 for x64, 64-bit" },
	{ MurmurHash64B_test,   64, "Murmur2C",    "MurmurHash2 for x86, 64-bit" },

	// MurmurHash3

	{ MurmurHash3_x86_32,   32, "Murmur3A",    "MurmurHash3 for x86, 32-bit" },
	{ MurmurHash3_x86_64,   64, "Murmur3B",    "MurmurHash3 for x86, 64-bit" },
	{ MurmurHash3_x86_128, 128, "Murmur3C",    "MurmurHash3 for x86, 128-bit" },

	{ MurmurHash3_x64_32,   32, "Murmur3D",    "MurmurHash3 for x64, 32-bit" },
	{ MurmurHash3_x64_64,   64, "Murmur3E",    "MurmurHash3 for x64, 64-bit" },
	{ MurmurHash3_x64_128, 128, "Murmur3F",    "MurmurHash3 for x64, 128-bit" },

};

HashInfo * findHash ( const char * name ) 
{
	for(int i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
	{
		if(_stricmp(name,g_hashes[i].name) == 0) return &g_hashes[i];
	}

	return NULL;
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, const char * hashname )
{
	const int hashbits = sizeof(hashtype) * 8;

	printf("-------------------------------------------------------------------------------\n");
	printf("--- Testing %s\n\n",hashname);

	//-----------------------------------------------------------------------------
	// Sanity tests

	if(g_testSanity || g_testAll)
	{
		printf("[[[ Sanity Tests ]]]\n\n");

		QuickBrownFox(hash,hashbits);
		SanityTest(hash,hashbits);
		AlignmentTest(hash,hashbits);
		AppendedZeroesTest(hash,hashbits);
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Speed tests

	if(g_testSpeed || g_testAll)
	{
		printf("[[[ Speed Tests ]]]\n\n");

		BulkSpeedTest(hash);
		printf("\n");

		TinySpeedTest<hashtype,4>(hash);
		TinySpeedTest<hashtype,8>(hash);
		TinySpeedTest<hashtype,16>(hash);
		TinySpeedTest<hashtype,32>(hash);
		TinySpeedTest<hashtype,64>(hash);
		TinySpeedTest<hashtype,128>(hash);
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Differential tests

	if(g_testDiff || g_testAll)
	{
		printf("[[[ Differential Tests ]]]\n\n");

		bool result = true;
		bool dumpCollisions = false;

		result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions);
		result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions);
		result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions);

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Avalanche tests.
	
	// 2 million reps is enough to measure bias down to ~0.25%
	
	if(g_testAvalanche || g_testAll)
	{
		printf("[[[ Avalanche Tests ]]]\n\n");

		const int hashbits = sizeof(hashtype) * 8;
		bool result = true;

		result &= AvalancheTest< Blob<hashbits * 2>, hashtype > (hash,2000000);

		// The bit independence test is slow and not particularly useful...
		//result &= BicTest < Blob<hashbits * 2>, hashtype > ( hash, 1000000 );

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Cyclic'

	if(g_testCyclic || g_testAll)
	{
		printf("[[[ Keyset 'Cyclic' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,10000000,drawDiagram);
		result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,10000000,drawDiagram);
		result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,10000000,drawDiagram);
		result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,10000000,drawDiagram);
		result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,10000000,drawDiagram);
		
		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Sparse'

	if(g_testSparse || g_testAll)
	{
		printf("[[[ Keyset 'Sparse' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		result &= SparseKeyTest<  32,hashtype>(hash,6,true,true,true,drawDiagram);
		result &= SparseKeyTest<  40,hashtype>(hash,6,true,true,true,drawDiagram);
		result &= SparseKeyTest<  48,hashtype>(hash,5,true,true,true,drawDiagram);
		result &= SparseKeyTest<  56,hashtype>(hash,5,true,true,true,drawDiagram);
		result &= SparseKeyTest<  64,hashtype>(hash,5,true,true,true,drawDiagram);
		result &= SparseKeyTest<  96,hashtype>(hash,4,true,true,true,drawDiagram); 
		result &= SparseKeyTest< 256,hashtype>(hash,3,true,true,true,drawDiagram);
		result &= SparseKeyTest<2048,hashtype>(hash,2,true,true,true,drawDiagram);

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Permutation'

	if(g_testPermutation || g_testAll)
	{
		printf("[[[ Keyset 'Permutation' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		// This very sparse set of blocks is particularly hard for SuperFastHash

		uint32_t blocks[] =
		{
			0x00000000,
			0x00000001,
			0x00000002,
			
			0x00000400,
			0x00008000,
			
			0x00080000,
			0x00200000,

			0x20000000,
			0x40000000,
			0x80000000,
		};

		result &= PermutationKeyTest<hashtype>(hash,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Window'

	// Skip distribution test for these - they're too easy to distribute well,
	// and it generates a _lot_ of testing

	if(g_testWindow || g_testAll)
	{
		printf("[[[ Keyset 'Window' Tests ]]]\n\n");

		bool result = true;
		bool testCollision = true;
		bool testDistribution = false;
		bool drawDiagram = false;

		result &= WindowedKeyTest< Blob<hashbits*2>, hashtype > ( hash, 20, testCollision, testDistribution, drawDiagram );

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Text'

	if(g_testText || g_testAll)
	{
		printf("[[[ Keyset 'Text' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

		result &= TextKeyTest( hash, "Foo",    alnum,4, "Bar",    drawDiagram );
		result &= TextKeyTest( hash, "FooBar", alnum,4, "",       drawDiagram );
		result &= TextKeyTest( hash, "",       alnum,4, "FooBar", drawDiagram );

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Zeroes'

	if(g_testZeroes || g_testAll)
	{
		printf("[[[ Keyset 'Zeroes' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		result &= ZeroKeyTest<hashtype>( hash, drawDiagram );

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}

	//-----------------------------------------------------------------------------
	// Keyset 'Seed'

	if(g_testSeed || g_testAll)
	{
		printf("[[[ Keyset 'Seed' Tests ]]]\n\n");

		bool result = true;
		bool drawDiagram = false;

		result &= SeedTest<hashtype>( hash, 1000000, drawDiagram );

		if(!result) printf("*********FAIL*********\n");
		printf("\n");
	}
}

//-----------------------------------------------------------------------------

void testHash ( const char * name )
{
	HashInfo * pInfo = findHash(name);

	if(pInfo == NULL)
	{
		printf("Invalid hash '%s' specified\n",name);
		return;
	}
	else
	{
		if(pInfo->hashbits == 32)
		{
			test<uint32_t>( pInfo->hash, pInfo->desc );
		}
		else if(pInfo->hashbits == 64)
		{
			test<uint64_t>( pInfo->hash, pInfo->desc );
		}
		else if(pInfo->hashbits == 128)
		{
			test<uint128_t>( pInfo->hash, pInfo->desc );
		}
		else
		{
			printf("Invalid hash bit width %d for hash '%s'",pInfo->hashbits,pInfo->name);
		}
	}
}
//-----------------------------------------------------------------------------

#pragma warning(disable : 4100)
#pragma warning(disable : 4702)

int main ( int argc, char ** argv )
{
	SetProcessAffinityMask(GetCurrentProcess(),2);

	int a = clock();

	g_testAll = true;

	//g_testWindow = true;
	//g_testSanity = true;
	//g_testSpeed = true;
	//g_testAvalanche = true;
	//g_testDiff = true;
	//g_testSparse = true;
	//g_testPermutation = true;

	//testHash("rand32");
	//testHash("rand64");
	//testHash("rand128");

	//testHash("fnv");
	//testHash("superfast");
	//testHash("lookup3");

	//testHash("murmur2");
	//testHash("murmur2B");
	//testHash("murmur2C");

	testHash("murmur3a");
	testHash("murmur3b");
	testHash("murmur3c");

	testHash("murmur3d");
	testHash("murmur3e");
	testHash("murmur3f");

	//----------

	int b = clock();

	printf("time %d\n",b-a);

	return 0;
}
