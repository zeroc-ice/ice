// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_MESSAGE_AUTHENTICATOR_F_H
#define ICE_MESSAGE_AUTHENTICATOR_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace SecureUdp
{

class MessageAuthenticator;
typedef IceInternal::Handle<MessageAuthenticator> MessageAuthenticatorPtr;

}

}

namespace IceInternal
{

void incRef(::IceSecurity::SecureUdp::MessageAuthenticator*);
void decRef(::IceSecurity::SecureUdp::MessageAuthenticator*);

}

#endif

