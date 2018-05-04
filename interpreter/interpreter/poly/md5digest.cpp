#include "md5digest.h"
// Openssl seems to be intent on deprecating MD5.  So ... I needed to 
// put #undef OPENSSL_NO_MD5 prior to #including md5.h.
#undef OPENSSL_NO_MD5
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

/********************************************************************************
 * MD5 DIGEST IMPLEMENTATIONS
 ********************************************************************************/
//--START BLOCK TO MAKE LINUX md5 VERSION LOOK LIKE SUN>>>>>>
/*
typedef md5_state_t MD5_CTX;
#define MD5Init md5_init
#define MD5Update md5_append
inline void MD5Final(unsigned char *A, MD5_CTX* B) { md5_finish(B,A); } 
*/
//--END BLOCK TO MAKE LINUX VERSION LOOK LIKE SUN <<<<<<< 

//--START BLOCK TO MAKE LINUX openssl/md5 VERSION LOOK LIKE SUN>>>>>>
#define MD5Init MD5_Init
#define MD5Update MD5_Update
#define MD5Final MD5_Final
//--END BLOCK TO MAKE LINUX VERSION LOOK LIKE SUN <<<<<<< 

void md5_digest(istream &in, unsigned char A[16])
{
  // Create MD5 Context and read each character of "in" into it.
  MD5_CTX md5_context;
  unsigned char c;
  MD5Init(&md5_context);
  while((c = in.get()) && in) //c != EOF)
    MD5Update(&md5_context, &c,1);

  // Retrieve the 128-bit "digest" from the MD5 Context  
  MD5Final(A, &md5_context);
}

#include <vector>

void md5_digest_saclibObj(Word o, unsigned char *A)
{
  MD5_CTX md5_context;
  unsigned char c;
  MD5Init(&md5_context);
  std::vector<Word> S;
  S.push_back(o);
  while(!S.empty())
  {
    Word o = S.back(),a,L; S.pop_back();
    if (o == NIL || ISATOM(o)) { MD5Update(&md5_context,(unsigned char*)&o,4); }
    else { ADV(o,&a,&L); S.push_back(L); S.push_back(a); }
  }
  // Retrieve the 128-bit "digest" from the MD5 Context  
  MD5Final(A, &md5_context);
}

string md5_digest(istream &in)
{
  // Retrieve the 128-bit "digest" from the MD5 Context
  unsigned char A[16];
  md5_digest(in,A);

  // Create & return string representation of the digest in hex
  ostringstream s;
  for(int i = 0; i < 16; ++i)
    s << setw(2) << setfill('0') << hex << (unsigned int)(A[i]);
  return s.str();
}
