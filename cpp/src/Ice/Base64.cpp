// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Base64.h>
#include <Ice/LocalException.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/UniqueRef.h>
#include <iterator>

#if defined(ICE_OS_UWP)
using namespace Platform;
using namespace Windows::Security::Cryptography;
using namespace Windows::Storage::Streams;
#elif defined(_WIN32)
#   include <Wincrypt.h>
#elif defined(__APPLE__)
#   include <Security/Security.h>
#else
#   include <openssl/bio.h>
#   include <openssl/evp.h>
#endif

using namespace IceUtil;
using namespace std;

string
IceInternal::Base64::encode(const vector<unsigned char>& decoded)
{
#if defined(ICE_OS_UWP)
    try
    {
        ArrayReference<unsigned char> data(const_cast<unsigned char*>(&decoded[0]),
                                           static_cast<unsigned int>(decoded.size()));
        auto writer = ref new DataWriter();
        writer->WriteBytes(data);
        return wstringToString(CryptographicBuffer::EncodeToBase64String(writer->DetachBuffer())->Data());
    }
    catch(Platform::Exception^ ex)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, wstringToString(ex->Message->Data()));
    }
#elif defined(_WIN32)
    DWORD sz = 0;
    const DWORD flags = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;
    if(!CryptBinaryToString(&decoded[0], static_cast<DWORD>(decoded.size()), flags, 0, &sz))
    {
        throw IllegalArgumentException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }
    std::string encoded;
    encoded.resize(sz - 1);
    if(!CryptBinaryToString(&decoded[0], static_cast<DWORD>(decoded.size()), flags, &encoded[0], &sz))
    {
        throw IllegalArgumentException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }
    return encoded;
#elif defined(__APPLE__)
    CFErrorRef err = 0;
    UniqueRef<SecTransformRef> encoder(SecEncodeTransformCreate(kSecBase64Encoding, &err));
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    CFDataRef in = CFDataCreateWithBytesNoCopy(0, &decoded[0], decoded.size(), kCFAllocatorNull);
    SecTransformSetAttribute(encoder.get(), kSecTransformInputAttributeName, in, &err);
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    UniqueRef<CFDataRef> data(static_cast<CFDataRef>(SecTransformExecute(encoder.get(), &err)));
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    return string(reinterpret_cast<const char*>(CFDataGetBytePtr(data.get())), CFDataGetLength(data.get()));
#else
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_new(BIO_s_mem());
    BIO_push(b64, bio);
    if(BIO_write(b64, &buffer[0], buffer.size()) <= 0)
    {
        BIO_free_all(b64);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    BIO_flush(b64);

    char* data;
    long size = BIO_get_mem_data(bio, &data);
    string encoded(data, size);
    BIO_free_all(b64);
    return encoded;
#endif
}

vector<unsigned char>
IceInternal::Base64::decode(const string& encoded)
{
    vector<unsigned char> decoded;
#if defined(ICE_OS_UWP)
    try
    {
        auto reader = DataReader::FromBuffer(
            CryptographicBuffer::DecodeFromBase64String(ref new String(stringToWstring(encoded).c_str())));
        decoded.resize(reader->UnconsumedBufferLength);
        if(!decoded.empty())
        {
            reader->ReadBytes(ArrayReference<unsigned char>(&decoded[0], reader->UnconsumedBufferLength));
        }
    }
    catch(Platform::Exception^ ex)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, wstringToString(ex->Message->Data()));
    }
#elif defined(_WIN32)
    DWORD sz = static_cast<DWORD>(encoded.size());
    decoded.resize(sz);
    if(!CryptStringToBinary(encoded.c_str(), sz, CRYPT_STRING_BASE64, &decoded[0], &sz, 0, 0))
    {
        throw IllegalArgumentException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }
    decoded.resize(sz);
#elif defined(__APPLE__)
    CFErrorRef err = 0;
    UniqueRef<SecTransformRef> decoder(SecDecodeTransformCreate(kSecBase64Encoding, &err));
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    CFDataRef input = CFDataCreateWithBytesNoCopy(0, reinterpret_cast<const unsigned char*>(&encoded[0]), 
                                                  encoded.size(), kCFAllocatorNull);
    SecTransformSetAttribute(decoder.get(), kSecTransformInputAttributeName, input, &err);
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    UniqueRef<CFDataRef> data(static_cast<CFDataRef>(SecTransformExecute(decoder.get(), &err)));
    if(err)
    {
        CFRelease(err);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    vector<unsigned char> out;
    decoded.resize(CFDataGetLength(data.get()));
    memcpy(&decoded[0], CFDataGetBytePtr(data.get()), decoded.size());
    return out;
#else
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_new_mem_buf(&encoded[0], static_cast<int>(encoded.size()));
    BIO_push(b64, bio);
    decoded.resize(encoded.size());
    int sz = BIO_read(b64, &decoded[0], static_cast<int>(decoded.size()));
    if(sz <= 0)
    {
        BIO_free_all(b64);
        throw IllegalArgumentException(__FILE__, __LINE__, "Base64 bad data");
    }
    decoded.resize(sz);
    BIO_free_all(b64);
#endif
    return decoded;
}
