// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CRYPT_KEY_H
#define ICE_CRYPT_KEY_H

#include <IceUtil/Shared.h>
#include <Ice/Stream.h>
#include <Ice/CryptKeyF.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceUtil::Shared;
using Ice::ByteSeq;

class CryptKey : public Shared
{

public:
    CryptKey(const ByteSeq&);
    virtual ~CryptKey();

    virtual const ByteSeq& toByteSeq() const;

    //
    // Compare CryptKeys for sorting purposes
    //
    virtual bool operator==(const CryptKey&) const;
    virtual bool operator!=(const CryptKey&) const;
    virtual bool operator<(const CryptKey&) const;

protected:
    ByteSeq _keyBytes;

};

inline bool operator==(const CryptKey& cryptKey, const CryptKeyPtr& cryptKeyPtr)
{
    return (cryptKey == *(cryptKeyPtr.get()));
}

inline bool operator==(const CryptKeyPtr& cryptKeyPtr, const CryptKey& cryptKey)
{
    return (cryptKey == *(cryptKeyPtr.get()));
}

}

}

#endif

