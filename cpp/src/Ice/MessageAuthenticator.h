// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_MESSAGE_AUTHENTICATOR_H
#define ICE_MESSAGE_AUTHENTICATOR_H

#include <IceUtil/Shared.h>
#include <Ice/Stream.h>
#include <Ice/MessageAuthenticatorF.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceUtil::Shared;
using Ice::ByteSeq;

class MessageAuthenticator : public Shared
{

public:
    MessageAuthenticator();
    MessageAuthenticator(const ByteSeq&);
    virtual ~MessageAuthenticator();

    virtual ByteSeq computeMAC(const ByteSeq&) const;

    virtual bool authenticate(const ByteSeq&, const ByteSeq&);

    virtual const ByteSeq& getMACKey() const;

protected:
    ByteSeq _macKeyBytes;

};

}

}

#endif

