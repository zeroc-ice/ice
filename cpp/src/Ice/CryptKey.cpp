// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/CryptKey.h>

using Ice::ByteSeq;

void ::IceInternal::incRef(::SecureUdp::CryptKey* p) { p->__incRef(); }
void ::IceInternal::decRef(::SecureUdp::CryptKey* p) { p->__decRef(); }

SecureUdp::CryptKey::CryptKey(const ByteSeq& key) :
                                 _keyBytes(key)
{
}

SecureUdp::CryptKey::~CryptKey()
{
}

const ByteSeq&
SecureUdp::CryptKey::toByteSeq() const
{
    return _keyBytes;
}

bool
SecureUdp::CryptKey::operator == (const CryptKey& key) const
{
    return _keyBytes == key._keyBytes;
}

bool
SecureUdp::CryptKey::operator != (const CryptKey& key) const
{
    return !operator==(key);
}

bool
SecureUdp::CryptKey::operator < (const CryptKey& key) const
{
    return _keyBytes < key._keyBytes;
}

