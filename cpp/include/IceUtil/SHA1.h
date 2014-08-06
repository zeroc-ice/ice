// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_SHA1_H
#define ICE_UTIL_SHA1_H

#include <IceUtil/Config.h>

#include <vector>

#ifndef ICE_OS_WINRT
#   if defined(_WIN32)
#      include <Wincrypt.h>
#   elif defined(__APPLE__)
#      include <CommonCrypto/CommonDigest.h>
#   else
#      include <openssl/sha.h>
#   endif
#endif

namespace IceUtil
{

ICE_UTIL_API void 
sha1(const unsigned char*, std::size_t, std::vector<unsigned char>&);
    
#ifndef ICE_OS_WINRT
class ICE_UTIL_API SHA1
{
public:
    SHA1();
    
#   ifdef _WIN32
    ~SHA1();
#   endif
    
    void update(const unsigned char*, std::size_t);
    
    void finalize(std::vector<unsigned char>&);

private:

    // noncopyable
    SHA1(const SHA1&);
    SHA1 operator=(const SHA1&);
    
#   if defined (_WIN32)
    HCRYPTPROV _ctx;
    HCRYPTHASH _hash;
#   elif defined(__APPLE__)
    CC_SHA1_CTX _ctx;
#   else
    SHA_CTX _ctx;
#   endif
};
#endif

}
#endif
