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

typedef std::vector<CryptKeyPtr> CryptKeys;

class Cryptor : public IceUtil::Shared
{

public:
    Cryptor();
    virtual ~Cryptor();

    virtual const CryptKeyPtr getNewKey();

    virtual const CryptKeyPtr getKey(const Ice::ByteSeq&);

    virtual const CryptKeyPtr getOrCreateKey(const Ice::ByteSeq&);

    virtual void encrypt(const CryptKeyPtr&, const Ice::ByteSeq&, Ice::ByteSeq&);

    virtual void decrypt(const CryptKeyPtr&, const Ice::ByteSeq&, Ice::ByteSeq&);

protected:
    CryptKeys _cryptKeys;

};

}

}

#endif

