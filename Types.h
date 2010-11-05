#pragma once

#include "pstdint.h"
#include "Bitvec.h"

//-----------------------------------------------------------------------------
// If the optimizer detects that a value in a speed test is constant or unused,
// the optimizer may remove references to it or otherwise create code that
// would not occur in a real-world application. To prevent the optimizer from
// doing this we declare two trivial functions that either sink or source data,
// and bar the compiler from optimizing them.

void     blackhole ( uint32_t x );
uint32_t whitehole ( void );

//-----------------------------------------------------------------------------

typedef void (*pfHash) ( const void * blob, const int len, const uint32_t seed, void * out );

template < typename T >
void swap ( T & a, T & b )
{
	T t = a;
	a = b;
	b = t;
}

//-----------------------------------------------------------------------------

template < class T >
class hashfunc
{
public:

	hashfunc ( pfHash h ) : m_hash(h)
	{
	}

	inline void operator () ( const void * key, const int len, const uint32_t seed, uint32_t * out )
	{
		m_hash(key,len,seed,out);
	}

	inline operator pfHash ( void ) const
	{
		return m_hash;
	}

	inline T operator () ( const void * key, const int len, const uint32_t seed ) 
	{
		T result;

		m_hash(key,len,seed,(uint32_t*)&result);

		return result;
	}

	pfHash m_hash;
};

//-----------------------------------------------------------------------------

template < int _bits >
class Blob
{
public:

	Blob()
	{
	}

	Blob ( int x )
	{
		for(int i = 0; i < nbytes; i++)
		{
			bytes[i] = 0;
		}

		*(int*)bytes = x;
	}

	Blob ( const Blob & k )
	{
		for(int i = 0; i < nbytes; i++)
		{
			bytes[i] = k.bytes[i];
		}
	}

	Blob & operator = ( const Blob & k )
	{
		for(int i = 0; i < nbytes; i++)
		{
			bytes[i] = k.bytes[i];
		}

		return *this;
	}

	void set ( const void * blob, int len )
	{
		const uint8_t * k = (const uint8_t*)blob;

		len = len > nbytes ? nbytes : len;

		for(int i = 0; i < len; i++)
		{
			bytes[i] = k[i];
		}

		for(int i = len; i < nbytes; i++)
		{
			bytes[i] = 0;
		}
	}

	uint8_t & operator [] ( int i )
	{
		return bytes[i];
	}

	const uint8_t & operator [] ( int i ) const
	{
		return bytes[i];
	}

	//----------
	// boolean operations
	
	bool operator < ( const Blob & k ) const
	{
		for(int i = 0; i < nbytes; i++)
		{
			if(bytes[i] < k.bytes[i]) return true;
			if(bytes[i] > k.bytes[i]) return false;
		}

		return false;
	}

	bool operator == ( const Blob & k ) const
	{
		for(int i = 0; i < nbytes; i++)
		{
			if(bytes[i] != k.bytes[i]) return false;
		}

		return true;
	}

	bool operator != ( const Blob & k ) const
	{
		return !(*this == k);
	}

	//----------
	// bitwise operations

	Blob operator ^ ( const Blob & k ) const 
	{
		Blob t;

		for(int i = 0; i < nbytes; i++)
		{
			t.bytes[i] = bytes[i] ^ k.bytes[i];
		}

		return t;
	}

	Blob & operator ^= ( const Blob & k )
	{
		for(int i = 0; i < nbytes; i++)
		{
			bytes[i] ^= k.bytes[i];
		}

		return *this;
	}

	int operator & ( int x )
	{
		return (*(int*)bytes) & x;
	}

	Blob & operator &= ( const Blob & k )
	{
		for(int i = 0; i < nbytes; i++)
		{
			bytes[i] &= k.bytes[i];
		}
	}

	Blob operator << ( int c )
	{
		Blob t = *this;

		lshift(&t.bytes[0],nbytes,c);

		return t;
	}

	Blob operator >> ( int c )
	{
		Blob t = *this;

		rshift(&t.bytes[0],nbytes,c);

		return t;
	}

	Blob & operator <<= ( int c )
	{
		lshift(&bytes[0],nbytes,c);

		return *this;
	}

	Blob & operator >>= ( int c )
	{
		rshift(&bytes[0],nbytes,c);

		return *this;
	}

	//----------
	
	enum
	{
		nbits = _bits,
		nbytes = (_bits+7)/8,

		align4  = (nbytes & 2) ? 0 : 1,
		align8  = (nbytes & 3) ? 0 : 1,
		align16 = (nbytes & 4) ? 0 : 1,
	};

private:

	uint8_t bytes[nbytes];
};

typedef Blob<128> uint128_t;

//-----------------------------------------------------------------------------
