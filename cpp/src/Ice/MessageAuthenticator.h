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

namespace SecureUdp
{

class MessageAuthenticator : public IceUtil::Shared
{

public:
    MessageAuthenticator();
    MessageAuthenticator(const Ice::ByteSeq&);
    virtual ~MessageAuthenticator();

    virtual Ice::ByteSeq computeMAC(const Ice::ByteSeq&) const;

    virtual bool authenticate(const Ice::ByteSeq&, const Ice::ByteSeq&);

    virtual const Ice::ByteSeq& getMACKey() const;

protected:
    Ice::ByteSeq _macKeyBytes;

};

}

#endif

