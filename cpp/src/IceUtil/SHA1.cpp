// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/SHA1.h>

#if defined(_WIN32) && !defined(ICE_OS_WINRT)
#   include <IceUtil/Exception.h>
#endif

using namespace std;
using namespace IceUtil;

#ifndef ICE_OS_WINRT
#   if defined(_WIN32)

namespace
{
const int SHA_DIGEST_LENGTH = 20;
}

IceUtil::SHA1::SHA1() :
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

IceUtil::SHA1::~SHA1()
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
IceUtil::SHA1::SHA1()
{
    CC_SHA1_Init(&_ctx);
}
#   else
IceUtil::SHA1::SHA1()
{
    SHA1_Init(&_ctx);
}
#   endif

void
IceUtil::SHA1::update(const unsigned char* data, size_t length)
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
IceUtil::SHA1::finalize(vector<unsigned char>& md)
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
#endif

void
IceUtil::sha1(const unsigned char* data, size_t length, vector<unsigned char>& md)
{
#if defined(ICE_OS_WINRT)
    auto dataA = ref new Platform::Array<unsigned char>(const_cast<unsigned char*>(&data[0]), 
                                                        static_cast<unsigned int>(data.size()));
    auto hasher = Windows::Security::Cryptography::Core::HashAlgorithmProvider::OpenAlgorithm("SHA1");
    auto hashed = hasher->HashData(Windows::Security::Cryptography::CryptographicBuffer::CreateFromByteArray(dataA));
    auto reader = ::Windows::Storage::Streams::DataReader::FromBuffer(hashed);
    md.resize(reader->UnconsumedBufferLength);
    if(!md.empty())
    {
        reader->ReadBytes(::Platform::ArrayReference<unsigned char>(&result[0],
                                                                    static_cast<unsigned int>(result.size())));
    }
    return md;
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
