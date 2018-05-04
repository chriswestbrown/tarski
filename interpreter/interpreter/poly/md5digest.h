#ifndef _MD5_DIGEST_
#define _MD5_DIGEST_
#include <iostream>
#include <string>
#include "gcword.h"
using namespace std;

/***************************************************************
 * md5_digest(in) takes an input stream "in" and returns the 
 * 128-bit md5 digest (or "hash") of the characters in the input
 * stream as a string of 32 hex digits.
 *
 * md5_digest(in,A) returns the 128-bit hash in an array A.
 ***************************************************************/
string md5_digest(istream &in);
void md5_digest(istream &in, unsigned char A[16]);
void md5_digest_saclibObj(Word o, unsigned char *A);

#endif

