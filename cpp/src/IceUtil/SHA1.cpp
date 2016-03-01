// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/SHA1.h>

#ifndef ICE_OS_WINRT
#   if defined(_WIN32)
#      include <Wincrypt.h>
#      include <IceUtil/Exception.h>
#   elif defined(__APPLE__)
#      include <CommonCrypto/CommonDigest.h>
#   else
#      include <openssl/sha.h>
#   endif
#endif

using namespace std;
using namespace IceUtil;

#ifndef ICE_OS_WINRT

namespace IceUtilInternal
{

class SHA1::Hasher
{
public:
    
    Hasher();
#   ifdef _WIN32
    ~Hasher();
#endif
    
    void update(const unsigned char*, std::size_t);
    void finalize(std::vector<unsigned char>&);
    
private:

    // noncopyable
    Hasher(const Hasher&);
    Hasher operator=(const Hasher&);
    
#   if defined (_WIN32)
    HCRYPTPROV _ctx;
    HCRYPTHASH _hash;
#   elif defined(__APPLE__)
    CC_SHA1_CTX _ctx;
#   else
    SHA_CTX _ctx;
#   endif
};

}

#   if defined(_WIN32)

namespace
{
const int SHA_DIGEST_LENGTH = 20;
}

IceUtilInternal::SHA1::Hasher::Hasher() :
    _ctx(0),
    _hash(0)
{
    if(!CryptAcquireContext(&_ctx, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, GetLastError());
    }

    if(!CryptCreateHash(_ctx, CALG_SHA1, 0, 0, &_hash))
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, GetLastError());
    }
}

IceUtilInternal::SHA1::Hasher::~Hasher()
{
    if(_hash)
    {
        CryptDestroyHash(_hash);
    }

    if(_ctx)
    {
        CryptReleaseContext(_ctx, 0);
    }
}
#   elif defined(__APPLE__)
IceUtilInternal::SHA1::Hasher::Hasher()
{
    CC_SHA1_Init(&_ctx);
}
#   else
IceUtilInternal::SHA1::Hasher::Hasher()
{
    SHA1_Init(&_ctx);
}
#   endif

void
IceUtilInternal::SHA1::Hasher::update(const unsigned char* data, size_t length)
{
#   if defined(_WIN32)
    if(!CryptHashData(_hash, data, static_cast<DWORD>(length), 0))
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, GetLastError());
    }
#   elif defined(__APPLE__)
    CC_SHA1_Update(&_ctx, reinterpret_cast<const void*>(data), length);
#   else
    SHA1_Update(&_ctx, reinterpret_cast<const void*>(data), length);
#   endif
}

void
IceUtilInternal::SHA1::Hasher::finalize(vector<unsigned char>& md)
{
#   if defined(_WIN32)
    md.resize(SHA_DIGEST_LENGTH);
    DWORD length = SHA_DIGEST_LENGTH;
    if(!CryptGetHashParam(_hash, HP_HASHVAL, &md[0], &length, 0))
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, GetLastError());
    }
#   elif defined(__APPLE__)
    md.resize(CC_SHA1_DIGEST_LENGTH);
    CC_SHA1_Final(&md[0], &_ctx);
#   else
    md.resize(SHA_DIGEST_LENGTH);
    SHA1_Final(&md[0], &_ctx);
#   endif
}

IceUtilInternal::SHA1::SHA1() :
    _hasher(new Hasher())
{
}

IceUtilInternal::SHA1::~SHA1()
{
}

void
IceUtilInternal::SHA1::update(const unsigned char* data, std::size_t length)
{
    _hasher->update(data, length);
}

void
IceUtilInternal::SHA1::finalize(std::vector<unsigned char>& md)
{
    _hasher->finalize(md);
}
#endif

void
IceUtilInternal::sha1(const unsigned char* data, size_t length, vector<unsigned char>& md)
{
#if defined(ICE_OS_WINRT)
    auto dataA =
        ref new Platform::Array<unsigned char>(const_cast<unsigned char*>(data), static_cast<unsigned int>(length));
    auto hasher = Windows::Security::Cryptography::Core::HashAlgorithmProvider::OpenAlgorithm("SHA1");
    auto hashed = hasher->HashData(Windows::Security::Cryptography::CryptographicBuffer::CreateFromByteArray(dataA));
    auto reader = ::Windows::Storage::Streams::DataReader::FromBuffer(hashed);
    md.resize(reader->UnconsumedBufferLength);
    if(!md.empty())
    {
        reader->ReadBytes(::Platform::ArrayReference<unsigned char>(&md[0], static_cast<unsigned int>(md.size())));
    }
#elif defined(_WIN32)
    SHA1 hasher;
    hasher.update(data, length);
    hasher.finalize(md);
#elif defined(__APPLE__)
    md.resize(CC_SHA1_DIGEST_LENGTH);
    CC_SHA1(&data[0], length, &md[0]);
#else
    md.resize(SHA_DIGEST_LENGTH);
    ::SHA1(&data[0], length, &md[0]);
#endif
}
