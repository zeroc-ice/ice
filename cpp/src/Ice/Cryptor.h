// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CRYPTOR_H
#define ICE_CRYPTOR_H

#include <IceUtil/Shared.h>
#include <Ice/Stream.h>
#include <Ice/CryptKeyF.h>
#include <Ice/CryptorF.h>
#include <vector>

namespace IceSecurity
{

namespace SecureUdp
{

using IceUtil::Shared;
using Ice::ByteSeq;

typedef std::vector<CryptKeyPtr> CryptKeys;

class Cryptor : public Shared
{

public:
    Cryptor();
    virtual ~Cryptor();

    virtual const CryptKeyPtr getNewKey();

    virtual const CryptKeyPtr getKey(const ByteSeq&);

    virtual const CryptKeyPtr getOrCreateKey(const ByteSeq&);

    virtual void encrypt(const CryptKeyPtr&, const ByteSeq&, ByteSeq&);

    virtual void decrypt(const CryptKeyPtr&, const ByteSeq&, ByteSeq&);

protected:
    CryptKeys _cryptKeys;

};

}

}

#endif

