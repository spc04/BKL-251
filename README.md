# BKL-251

C implementation of parallel, efficient and faster Field Arithmetic operations for the Binary-Kummer-Line-251 over GF(2^256) using VPCLMULQDQ intrinsic instruction set.

## Overview

This project provides optimized implementations of field arithmetic operations for the Binary-Kummer-Line-251 elliptic curve over the finite field GF(2^256). The implementations leverage Intel's VPCLMULQDQ instruction set for high-performance carry-less multiplication, which is essential for efficient elliptic curve cryptography.

## Files

- `vpcl.c`: Basic schoolbook multiplication implementation using AVX intrinsics
- `vpcl_red.c`: Multiplication with field reduction for BKL-251
- `vpcl_red_parallel.c`: Parallel implementation supporting multiple simultaneous operations

## Features

- **VPCLMULQDQ Optimization**: Utilizes Intel's Vector PCLMULQDQ instructions for fast carry-less multiplication
- **AVX Support**: Leverages AVX2 and AVX-512 intrinsics for vectorized operations
- **Parallel Processing**: Parallel version supports concurrent arithmetic operations
- **Field Reduction**: Implements efficient reduction modulo the BKL-251 field polynomial

## Compilation

Compile with GCC using the following flags:

```bash
gcc -o vpcl -mavx2 -msse4.1 -mpclmul -mvpclmulqdq vpcl.c
gcc -o vpcl_red -mavx2 -msse4.1 -mpclmul -mvpclmulqdq vpcl_red.c
gcc -o vpcl_red_parallel -mavx2 -msse4.1 -mpclmul -mvpclmulqdq vpcl_red_parallel.c
```

## Requirements

- GCC compiler with AVX and PCLMUL support
- Intel processor supporting VPCLMULQDQ instructions (Intel Ice Lake or later, or AMD Zen 3 or later)
- Linux/Windows environment

## Usage

Each program includes a main function with example usage. Run the compiled binaries to see sample multiplication and reduction operations.

## Performance

The implementations are optimized for:
- Low latency elliptic curve operations
- High-throughput cryptographic computations
- Efficient use of SIMD instructions
