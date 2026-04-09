// Compilation instruction :- gcc -o vpcl_red -mavx2 -msse4.1 -mpclmul -mvpclmulqdq vpcl_red.c

#include <immintrin.h>
#include<wmmintrin.h>
#include<xmmintrin.h>
#include<smmintrin.h>
#include<emmintrin.h>
#include<tmmintrin.h>
#include<malloc.h>
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
	
__m256i left_shift_128_in256(__m256i a, int n) {
	
	return _mm256_or_si256(_mm256_slli_epi64(a, n),_mm256_srli_epi64(_mm256_slli_si256(a, 8), (64 - n))); 
	
	}


__m256i right_shift_128_in256(__m256i a, int n) {
	
	return _mm256_or_si256(_mm256_srli_epi64(a, n),_mm256_slli_epi64(_mm256_srli_si256(a, 8), (64 - n))); 
	
	}


void expandM_Par (__m256i x[7], __m256i y[8]) {
	
	for (int i=0; i<7; i++)	  y[i] = x[i];
	
	y[7] = _mm256_setzero_si256();
	
	
	for (int i=0; i<7; i++) {
	
		y[i+1] = _mm256_xor_si256(y[i+1], _mm256_srli_si256(y[i], 8));
		y[i] = _mm256_slli_si256(y[i], 8); y[i] = _mm256_srli_si256(y[i], 8);

		}
	
	}

void fold_Par (__m256i y[8], __m256i x[4]) {

	__m128i rt5 = _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x12, 0xa0);
	__m256i rt5New = _mm256_set_m128i(rt5, rt5);
	__m256i temp;
	for (int i=0; i<4; i++) {
		
		temp = _mm256_setzero_si256();
		temp = _mm256_clmulepi64_epi128(y[i+4], rt5New, 0x00);
		x[i] = _mm256_xor_si256(y[i], temp);
		}
	
	}

void reduce_Par(__m256i u[4], __m256i v[4], unsigned char *c, unsigned char *d) {

	__m256i w[4];
	__m128i r = _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0x95);
	__m256i rNew = _mm256_set_m128i(r, r);
	v[0] = u[0];
	for (int i=0; i<3; i++) {
		
		w[i] = _mm256_srli_si256(v[i], 8);
		v[i] = _mm256_slli_si256(v[i], 8); v[i] = _mm256_srli_si256(v[i], 8);
		v[i+1] = _mm256_xor_si256(u[i+1], w[i]); 
		
		}
	w[3] = right_shift_128_in256(v[3], 59);
	v[3] = _mm256_slli_si256(v[3], 8); v[3] = right_shift_128_in256(v[3], 59); /* v[3] = _mm_srli_si128(v[3], 8); v[3] = _mm_slli_epi64(v[3], 5); */  v[3] = _mm256_srli_epi64(v[3], 5);
	
	//temp = _mm256_setzero_si256();
	w[3] = _mm256_clmulepi64_epi128(w[3], rNew, 0x00);
	//w[3] = tmp;
	v[0] = _mm256_xor_si256(v[0], w[3]);
	w[0] = _mm256_srli_si256(v[0], 8);
	v[0] = _mm256_slli_si256(v[0], 8); v[0] = _mm256_srli_si256(v[0], 8);
	v[1] = _mm256_xor_si256(v[1], w[0]); 	
	
	//for (int i=0; i<4; i++) {
	
	v[0] = _mm256_xor_si256(v[0], _mm256_slli_si256(v[1], 8));
	v[2] = _mm256_xor_si256(v[2], _mm256_slli_si256(v[3], 8));
	
	_mm256_storeu_si256(((__m256i*)c), _mm256_set_m128i(_mm256_extracti128_si256(v[2], 0), _mm256_extracti128_si256(v[0], 0)));
	_mm256_storeu_si256(((__m256i*)d), _mm256_set_m128i(_mm256_extracti128_si256(v[2], 1), _mm256_extracti128_si256(v[0], 1)));
	
		}



void schoolBookMul256_Par( unsigned char *u, unsigned char *v, unsigned char *w, unsigned char *m, unsigned char *n, unsigned char *o, __m256i polyMult[7]) {

	__m256i U, V, M, N, MUl, NVl, MUh, NVh, x00, x01, x10, x11, y00, y01, y10, y11, z00, z01, z10, z11, r00, r01, r10, r11, x0, x1, x2, x3, x4, x5, x6, temp0, temp1, temp2, temp3, final_uv0, final_uv1, final_mn0, final_mn1;
	
	
	// Will Multiply U by V and M by N.
	
	U = _mm256_loadu_si256((__m256i*)u); // u11||u10||u01||u00
	V = _mm256_loadu_si256((__m256i*)v); // v11||v10||v01||v00
	M = _mm256_loadu_si256((__m256i*)m); // m11||m10||m01||m00
	N = _mm256_loadu_si256((__m256i*)n); // n11||n10||n01||n00
	
//	UMl = _mm256_inserti128_si256(U, M, );
	MUl = _mm256_set_m128i(_mm256_extracti128_si256(M, 0), _mm256_extracti128_si256(U, 0)); 
	NVl = _mm256_set_m128i(_mm256_extracti128_si256(N, 0), _mm256_extracti128_si256(V, 0));
	MUh = _mm256_set_m128i(_mm256_extracti128_si256(M, 1), _mm256_extracti128_si256(U, 1));
	NVh = _mm256_set_m128i(_mm256_extracti128_si256(N, 1), _mm256_extracti128_si256(V, 1));
	
	
	x00 = _mm256_clmulepi64_epi128(MUl, NVl, 0x00); // m00.n00 || u00.v00
	x01 = _mm256_clmulepi64_epi128(MUl, NVl, 0x01); // m00.n01 || u00.v01
	x10 = _mm256_clmulepi64_epi128(MUl, NVl, 0x10); // m01.n00 || u01.v00
	x11 = _mm256_clmulepi64_epi128(MUl, NVl, 0x11); // m01.n01 || u01.v01
	

	y00 = _mm256_clmulepi64_epi128(MUh, NVh, 0x00); // m10.n10 || u10.v10 
	y01 = _mm256_clmulepi64_epi128(MUh, NVh, 0x01); // m10.n11 || u10.v11
	y10 = _mm256_clmulepi64_epi128(MUh, NVh, 0x10); // m11.n10 || u11.n10
	y11 = _mm256_clmulepi64_epi128(MUh, NVh, 0x11); // m11.n11 || u11.v11
	
	
	z00 = _mm256_clmulepi64_epi128(MUl, NVh, 0x00); // m00.n10 || u00.v10 
	z01 = _mm256_clmulepi64_epi128(MUl, NVh, 0x01); // m00.n11 || u00.v11
	z10 = _mm256_clmulepi64_epi128(MUl, NVh, 0x10); // m01.n10 || u01.n10
	z11 = _mm256_clmulepi64_epi128(MUl, NVh, 0x11); // m01.n11 || u01.v11
	
	
	r00 = _mm256_clmulepi64_epi128(MUh, NVl, 0x00); // m10.n00 || u10.v00 
	r01 = _mm256_clmulepi64_epi128(MUh, NVl, 0x01); // m10.n01 || u10.v01
	r10 = _mm256_clmulepi64_epi128(MUh, NVl, 0x10); // m11.n00 || u11.n00
	r11 = _mm256_clmulepi64_epi128(MUh, NVl, 0x11); // m11.n01 || u11.v01
	
	
	x0 = x00;
	polyMult[0] = x0;
	
	
	x1 = _mm256_xor_si256(x01, x10);
	polyMult[1] = x1;
	
	
	x2 = _mm256_xor_si256(z00, r00);
	x2 = _mm256_xor_si256(x2, x11);
	polyMult[2] = x2;
	
	
	x3 = _mm256_xor_si256(r10, r01);
	x3 = _mm256_xor_si256(x3, z10);
	x3 = _mm256_xor_si256(x3, z01);
	polyMult[3] = x3;
	
	
	x4 = _mm256_xor_si256(r11, z11);
	x4 = _mm256_xor_si256(x4, y00);
	polyMult[4] = x4;
	
	
	x5 = _mm256_xor_si256(y10, y01);
	polyMult[5] = x5;
	
	
	x6 = y11;
	polyMult[6] = x6;
	
	
	// Now, the bit adjustment part :----
	
	
	temp0 = _mm256_xor_si256(x0, _mm256_slli_si256(x1, 8));
	
	temp1 = _mm256_xor_si256(x2, _mm256_srli_si256(x1, 8));
	temp1 = _mm256_xor_si256(temp1, _mm256_slli_si256(x3, 8));
	
	temp2 = _mm256_xor_si256(x4, _mm256_srli_si256(x3, 8));
	temp2 = _mm256_xor_si256(temp2, _mm256_slli_si256(x5, 8));
	
	temp3 = _mm256_xor_si256(x6, _mm256_srli_si256(x5, 8));
	
	
	final_uv0 = _mm256_set_m128i(_mm256_extracti128_si256(temp1, 0), _mm256_extracti128_si256(temp0, 0));
	final_uv1 = _mm256_set_m128i(_mm256_extracti128_si256(temp3, 0), _mm256_extracti128_si256(temp2, 0));
	
	final_mn0 = _mm256_set_m128i(_mm256_extracti128_si256(temp1, 1), _mm256_extracti128_si256(temp0, 1));
	final_mn1 = _mm256_set_m128i(_mm256_extracti128_si256(temp3, 1), _mm256_extracti128_si256(temp2, 1));
	
	
	_mm256_storeu_si256(((__m256i*)w), final_uv0); 
        _mm256_storeu_si256(((__m256i*)w+1), final_uv1);
        _mm256_storeu_si256(((__m256i*)o), final_mn0); 
        _mm256_storeu_si256(((__m256i*)o+1), final_mn1);
         
	//printBytes(c, 32);
	

	
	}

void AddSub (unsigned char *a, unsigned char *b, unsigned char *c) {
	
	__m256i T, H, R;
	
	T = _mm256_loadu_si256((__m256i*)a);
	H = _mm256_loadu_si256((__m256i*)b);
	
	R = _mm256_xor_si256(T, H);
	
	_mm256_storeu_si256(((__m256i*)c), R);
	
	}



void packed64_read( unsigned char *a, unsigned char *b, __m128i u[4], __m128i v[4]) {

	__m256i T, H;
	__m128i hi, lo;
	unsigned char ch[16];
	
	T = _mm256_loadu_si256((__m256i*)a);
	H = _mm256_loadu_si256((__m256i*)b);
	
	lo = _mm256_extracti128_si256(T, 0);
	u[1] = _mm_srli_si128(lo, 8);
	//_mm_storeu_si128(((__m128i*)ch), u[1]);
	//printBytes(ch, 16);
	u[0] = _mm_slli_si128(lo, 8); u[0] = _mm_srli_si128(u[0], 8);
	hi = _mm256_extracti128_si256(T, 1);
	u[3] = _mm_srli_si128(hi, 8);
	u[2] = _mm_slli_si128(hi, 8); u[2] = _mm_srli_si128(u[2], 8);
	
	lo = _mm256_extracti128_si256(H, 0);
	v[1] = _mm_srli_si128(lo, 8);
	v[0] = _mm_slli_si128(lo, 8); v[0] = _mm_srli_si128(v[0], 8);
	hi = _mm256_extracti128_si256(H, 1);
	v[3] = _mm_srli_si128(hi, 8);
	v[2] = _mm_slli_si128(hi, 8); v[2] = _mm_srli_si128(v[2], 8);
	}


void packed64_read_Par( unsigned char *a, unsigned char *b, unsigned char *x, unsigned char *y, __m256i u[4], __m256i v[4]) {

	__m256i A, B, X, Y, temp;
	
	
	A = _mm256_loadu_si256((__m256i*)a);
	B = _mm256_loadu_si256((__m256i*)b);
	X = _mm256_loadu_si256((__m256i*)x);
	Y = _mm256_loadu_si256((__m256i*)y);
	
	
	temp = _mm256_set_m128i(_mm256_extracti128_si256(X, 0), _mm256_extracti128_si256(A, 0));
	u[1] = _mm256_srli_si256(temp, 8);
	//_mm_storeu_si128(((__m128i*)ch), u[1]);
	//printBytes(ch, 16);
	u[0] = _mm256_slli_si256(temp, 8); u[0] = _mm256_srli_si256(u[0], 8);
	temp = _mm256_set_m128i(_mm256_extracti128_si256(X, 1), _mm256_extracti128_si256(A, 1));
	u[3] = _mm256_srli_si256(temp, 8);
	u[2] = _mm256_slli_si256(temp, 8); u[2] = _mm256_srli_si256(u[2], 8);
	
	temp = _mm256_set_m128i(_mm256_extracti128_si256(Y, 0), _mm256_extracti128_si256(B, 0));
	v[1] = _mm256_srli_si256(temp, 8);
	v[0] = _mm256_slli_si256(temp, 8); v[0] = _mm256_srli_si256(v[0], 8);
	temp = _mm256_set_m128i(_mm256_extracti128_si256(Y, 1), _mm256_extracti128_si256(B, 1));
	v[3] = _mm256_srli_si256(temp, 8);
	v[2] = _mm256_slli_si256(temp, 8); v[2] = _mm256_srli_si256(v[2], 8);
	}



void main(){
 	/*ALIGN16 unsigned char a[16]={0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 	ALIGN16 unsigned char b[16]={0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; */
	ALIGN32 unsigned char a[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,0};
	ALIGN32 unsigned char b[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,0};
 	/*ALIGN32 unsigned char a[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,07};
 	ALIGN32 unsigned char b[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,07}; */
 	
 	ALIGN32 unsigned char p[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,7};
	ALIGN32 unsigned char q[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,7};
 	
 	ALIGN32 unsigned char c[64], r[64], z[32], t[32];
 	/*ALIGN32*/ 
	/*ALIGN16*/ __m256i pm[7], y[8], u[4], v[4], m[4], n[4];


	schoolBookMul256_Par(a,b,c,p,q,r,pm);
	//Karatsuba(a, b, d);
	//Reduction(c, x);
	
	printBytes(a,32);
	printBytes(b,32);
	printf("\n");
	printBytes(p,32);
	printBytes(q,32);


	printf("\nThe SchoolBook Multiplications :-\n");
	printBytes(c,64);
	printf("\n");
	printBytes(r,64);

	//packed64_read(a,b,u,v);
	//printBytes(u[0],);
	//polyMult(u,v,x);
	expandM_Par(pm, y);
	fold_Par(y, m);
	reduce_Par(m, n, z, t);
	
	
	
	//printf("\nThe Karatsuba Multiplication :-\n");
	//printBytes(d, 32);
	
	printf("\nThe Reduced forms of the answers are (maybe...?) :-\n");
	printBytes(z, 32);
	printf("\n");
	printBytes(t, 32);

}
	
	
	
	
	
	
	
	
	
