// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Cryptor.h>
#include <Ice/CryptKey.h>
#include <algorithm>

void ::IceInternal::incRef(::IceSecurity::SecureUdp::Cryptor* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::SecureUdp::Cryptor* p) { p->__decRef(); }

using Ice::ByteSeq;
using IceSecurity::SecureUdp::CryptKey;
using IceSecurity::SecureUdp::CryptKeyPtr;

IceSecurity::SecureUdp::Cryptor::Cryptor()
{
}

IceSecurity::SecureUdp::Cryptor::~Cryptor()
{
}

const CryptKeyPtr
IceSecurity::SecureUdp::Cryptor::getNewKey()
{
    // Gotta return a newly generated key here.
    ByteSeq byteSeq;

    // Bogus key - gotta fix this.
    byteSeq.push_back('A');
    byteSeq.push_back('n');
    byteSeq.push_back('t');
    byteSeq.push_back('h');
    byteSeq.push_back('o');
    byteSeq.push_back('n');
    byteSeq.push_back('y');
    byteSeq.push_back('D');
    byteSeq.push_back('a');
    byteSeq.push_back('r');
    byteSeq.push_back('i');
    byteSeq.push_back('u');
    byteSeq.push_back('s');

    CryptKeyPtr cryptKey = new CryptKey(byteSeq);

    _cryptKeys.push_back(cryptKey);

    return cryptKey;
}

const CryptKeyPtr
IceSecurity::SecureUdp::Cryptor::getKey(const ByteSeq& key)
{
    CryptKeyPtr cryptKey = new CryptKey(key);

    CryptKeys::iterator begin = _cryptKeys.begin();
    CryptKeys::iterator end = _cryptKeys.end();
    CryptKeys::iterator pos = std::find(begin, end, cryptKey);

    if (pos == end)
    {
        // TODO: Exception - Trying to use a key that we didn't hand out.
    }

    return *pos;
}

const CryptKeyPtr
IceSecurity::SecureUdp::Cryptor::getOrCreateKey(const ByteSeq& key)
{
    CryptKeyPtr cryptKey;

    cryptKey = getKey(key);

    if (cryptKey == 0)
    {
        cryptKey = new CryptKey(key);

        _cryptKeys.push_back(cryptKey);
    }

    return cryptKey;
}

void
IceSecurity::SecureUdp::Cryptor::encrypt(const CryptKeyPtr& key, const ByteSeq& plainBuffer, ByteSeq& encryptedBuffer)
{
    encryptedBuffer = plainBuffer;
}

void
IceSecurity::SecureUdp::Cryptor::decrypt(const CryptKeyPtr& key, const ByteSeq& encryptedBuffer, ByteSeq& plainBuffer)
{
    plainBuffer = encryptedBuffer;
}
