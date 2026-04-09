#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

#define align __attribute__ ((aligned (32)))

#if !defined (ALIGN32)
        # if defined (__GNUC__)
                # define ALIGN32 __attribute__ ( (aligned (32)))
        # else
                # define ALIGN32 __declspec (align (32))
        # endif
#endif

void printBytes(unsigned char *data, int num){
       int i;

        for(i=num-1; i>=0; i--)
                printf("%02x ", data[i]);
        printf("\n");
}

/*
__m256i LSB(__m256i a) {

	x = _mm256_set_m128i(0x0000000000000000, 0xffffffffffffffff);
	x = _mm256_and_si256(a, x);
	
	return x;
	}


__m256i MSB(__m256i a) {

	x = _mm256_set_m128i(0xffffffffffffffff, 0x0000000000000000);
	x = _mm256_and_si256(a, x);
	
	return x;
	}
	
*/	
	
__m128i left_shift_128(__m128i a, int n) {
	
	return _mm_or_si128(_mm_slli_epi64(a, n),_mm_srli_epi64(_mm_slli_si128(a, 8), (64 - n))); 
	
	}


__m128i right_shift_128(__m128i a, int n) {
	
	return _mm_or_si128(_mm_srli_epi64(a, n),_mm_slli_epi64(_mm_srli_si128(a, 8), (64 - n))); 
	
	}


void expandM (__m256i x[7], __m128i y[8]) {
	
	for (int i=0; i<7; i++)	  y[i] = _mm256_extract128_si256(x[i], 0);
	
	for (int i=0; i<7; i++) {
	
		y[i+1] = _mm_xor_si128(y[i+1], _mm_srli_si128(y[i], 8));
		y[i] = _mm_slli_si128(y[i]); y[i] = _mm_srli_si128(y[i]);
		 
		}
	
	}

void fold () {

	
	}

void reduce(__m128i u[3], __m128i v[3]) {

	__m128i w[4];
	__m128i r = ;
	v[0] = u[0];
	for (int i=0; i<3; i++) {
		
		w[i] = _mm_srli_si128(v[i], 8);
		v[i] = _mm_slli_si128(v[i], 8); v[i] = _mm_srli_si128(v[i], 8);
		v[i+1] = _mm_xor_si128(u[i+1], w[i]); 
		
		}
	w[3] = right_shift_128(v[3], 59);
	v[3] = _mm_slli_si128(v[3], 8); v[3] = right_shift_128(v[3], 59); v[3] = _mm_srli_epi64(v[3], 5
);
	w[3] = _mm_clmulepi64_si128(w[3], r, 0x00);
	v[0] = _mm_xor_si128(v[0], w[3]);
	w[0] = _mm_srli_si128(v[0], 8);
	v[0] = _mm_slli_si128(v[0], 8); v[0] = _mm_srli_si128(v[0], 8);
	v[1] = _mm_xor_si128(v[1], w[0]); 	
	
	} 



void schoolBookMul256( unsigned char *a, unsigned char *b, unsigned char *c) {

	__m256i T, H, T1, x00, x01, x10, x11, y00, y01, y10, y11, x1, x2, x21, x3, x4, x41, x5, x6, y2, final0, final1;
	__m128i hi, lo, hi1, lo1;
	
	
	T = _mm256_loadu_si256((__m256i*)a); // a11||a10||a01||a00
	H = _mm256_loadu_si256((__m256i*)b); // b11||b10||b01||b00
	
	x00 = _mm256_clmulepi64_epi128(T, H, 0x00); // a10.b10 || a00.b00
	x01 = _mm256_clmulepi64_epi128(T, H, 0x01); // a10.b11 || a00.b01
	x10 = _mm256_clmulepi64_epi128(T, H, 0x10); // a11.b10 || a01.b00
	x11 = _mm256_clmulepi64_epi128(T, H, 0x11); // a11.b11 || a01.b01
	
	//T1 = _mm256_srli_si256(T, 16);
	//T2 = _mm256_slli_si256(T, 16);
	lo = _mm256_extracti128_si256(T, 0);
	hi = _mm256_extracti128_si256(T, 1);
	T1 = _mm256_set_m128i(lo, hi);
	
	y00 = _mm256_clmulepi64_epi128(T1, H, 0x00); // a00.b10 || a10.b00 
	y01 = _mm256_clmulepi64_epi128(T1, H, 0x01); // a00.b11 || a10.b01
	y10 = _mm256_clmulepi64_epi128(T1, H, 0x10); // a01.b10 || a11.b00
	y11 = _mm256_clmulepi64_epi128(T1, H, 0x11); // a01.b11 || a11.b01
	
	
	//x1 = _mm256_srli_si256(x11, 16); x1 = _mm256_slli_si256(x1, 16);
	hi = _mm256_extracti128_si256(x11, 1); x1 = _mm256_set_m128i(_mm_setzero_si128(), hi);
	//x6 = _mm256_slli_si256(x00, 16); x6 = _mm256_srli_si256(x6, 16);
	lo = _mm256_extracti128_si256(x00, 0); x6 = _mm256_set_m128i(_mm_setzero_si128(), lo);
	
	
	x2 = _mm256_xor_si256(x01, x10);
	//x21 = _mm256_srli_si256(x2, 16); 
	hi = _mm256_extracti128_si256(x2, 1);
	x21 = _mm256_set_m128i(_mm_setzero_si128(), hi);// msb 128 bits of x2 becoming the lsb 128 bits of x21
	//x2 = _mm256_slli_si256(x2, 16); x2 = _mm256_srli_si256(x2, 16); 
	lo = _mm256_extracti128_si256(x2, 0); x2 = _mm256_set_m128i(_mm_setzero_si128(), lo);// msb 128 bits of x2 becomes 0
	
	//x21 = _mm256_slli_si256(x21, 8);
	lo = _mm256_extracti128_si256(x21, 0);
	hi = _mm256_extracti128_si256(x21, 1);
	lo1 = _mm_srli_si128(lo, 8);
	hi = _mm_xor_si128(hi, lo1); lo = _mm_slli_si128(lo, 8);
	x21 = _mm256_set_m128i(hi, lo);
	//x2 = _mm256_slli_si256(x2, 8); 
	lo = _mm256_extracti128_si256(x2, 0);
	hi = _mm256_extracti128_si256(x2, 1);
	lo1 = _mm_srli_si128(lo, 8);
	hi = _mm_xor_si128(hi, lo1); lo = _mm_slli_si128(lo, 8);
	x2 = _mm256_set_m128i(hi, lo);
	
	//x00 = _mm256_srli_si256(x00, 16);
	hi = _mm256_extracti128_si256(x00, 1);
	x00 = _mm256_set_m128i(_mm_setzero_si128(), hi);
	//y2 = _mm256_srli_si256(y11, 16);
	hi = _mm256_extracti128_si256(y11, 1);
	y2 = _mm256_set_m128i(_mm_setzero_si128(), hi);
	//y11 = _mm256_slli_si256(y11, 16); y11 = _mm256_srli_si256(y11, 16);
	hi = _mm256_extracti128_si256(y11, 1);
	y11 = _mm256_set_m128i(_mm_setzero_si128(), hi);
	
	x3 = _mm256_xor_si256(x00, y2);
	x3 = _mm256_xor_si256(x3, y11);
	
	//x4 = _mm256_srli_si256(y01, 16);
	hi = _mm256_extracti128_si256(y01, 1);
	hi1 = _mm256_extracti128_si256(y10, 1);
	hi = _mm_xor_si128(hi, hi1);
	x4 = _mm256_set_m128i(_mm_setzero_si128(), hi);
	lo = _mm256_extracti128_si256(y01, 0); y01 = _mm256_set_m128i(_mm_setzero_si128(), lo);
	lo1 = _mm256_extracti128_si256(y10, 0); y10 = _mm256_set_m128i(_mm_setzero_si128(), lo1);
	
	x4 = _mm256_xor_si256(x4, y01); 
	x4 = _mm256_xor_si256(x4, y10); 
	
	x41 = _mm256_srli_si256(x4, 8); // 3rd most significant packed 64 bits of x4 becoming the lsb of x41
	//x4 = _mm256_slli_si256(x4, 8); 
	lo = _mm256_extracti128_si256(x4, 0);
	lo = _mm_srli_si128(lo, 8);
	x4 = _mm256_set_m128i(lo, _mm_setzero_si128());// lsb 64 bits of x4 becoming the msb 64 bits of x4
	
	
	//x11 = _mm256_slli_si256(x11, 16); x11 = _mm256_srli_si256(x11, 16);
	lo = _mm256_extracti128_si256(x11, 0); x11 = _mm256_set_m128i(_mm_setzero_si128(), lo);
	//y2 = _mm256_srli_si256(y00, 16);
	//y00 = _mm256_slli_si256(y00, 16); y00 = _mm256_srli_si256(y00, 16);
	hi = _mm256_extracti128_si256(y00, 1); lo = _mm256_extracti128_si256(y00, 0);
	y2 = _mm256_set_m128i(_mm_setzero_si128(), hi); y00 = _mm256_set_m128i(_mm_setzero_si128(), lo);
	x5 = _mm256_xor_si256(x11, y2);
	x5 = _mm256_xor_si256(x5, y00);
	
	//x5 = _mm256_slli_si256(x5, 16); 
	lo = _mm256_extracti128_si256(x5, 0);
	x5 = _mm256_set_m128i(lo, _mm_setzero_si128());// making the lsb 128 bits of x5 the msb 128 bits of x5
	
	
	final0 = _mm256_xor_si256(x4, x5); final0 = _mm256_xor_si256(final0, x2); final0 = _mm256_xor_si256(final0, x6);
	final1 = _mm256_xor_si256(x41, x3); final1 = _mm256_xor_si256(final1, x21); final1 = _mm256_xor_si256(final1, x1);
	
	_mm256_storeu_si256(((__m256i*)c), final0); 
        _mm256_storeu_si256(((__m256i*)c+1), final1); 
	printBytes(c, 32);
	
	
	}


void main(){
 	/*ALIGN16 unsigned char a[16]={0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 	ALIGN16 unsigned char b[16]={0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; */
	ALIGN32 unsigned char a[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
	ALIGN32 unsigned char b[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
 	ALIGN32 unsigned char c[64], d[64], x[32];

	schoolBookMul256(a,b,c);
	//Karatsuba(a, b, d);
	//Reduction(c, x);
	
	printBytes(a,32);
	printBytes(b,32);
	
	printf("\nThe SchoolBook Multiplication :-\n");
	printBytes(c,64);

	//printf("\nThe Karatsuba Multiplication :-\n");
	//printBytes(d, 32);
	
	//printf("\nThe Reduced form of the answer is (maybe...?) :-\n");
	//printBytes(x, 16);
	
}
	
	
	
	
	
	
	
	
	
