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

void ::IceInternal::incRef(::IceSecurity::SecureUdp::CryptKey* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::SecureUdp::CryptKey* p) { p->__decRef(); }

IceSecurity::SecureUdp::CryptKey::CryptKey(const ByteSeq& key) :
                                 _keyBytes(key)
{
}

IceSecurity::SecureUdp::CryptKey::~CryptKey()
{
}

const ByteSeq&
IceSecurity::SecureUdp::CryptKey::toByteSeq() const
{
    return _keyBytes;
}

bool
IceSecurity::SecureUdp::CryptKey::operator == (const CryptKey& key) const
{
    return _keyBytes == key._keyBytes;
}

bool
IceSecurity::SecureUdp::CryptKey::operator != (const CryptKey& key) const
{
    return !operator==(key);
}

bool
IceSecurity::SecureUdp::CryptKey::operator < (const CryptKey& key) const
{
    return _keyBytes < key._keyBytes;
}

