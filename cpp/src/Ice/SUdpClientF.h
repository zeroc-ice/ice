// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_CLIENT_F_H
#define ICE_SUDP_CLIENT_F_H

#include <Ice/Handle.h>

namespace SecureUdp
{

class SUdpClient;
typedef IceInternal::Handle<SUdpClient> SUdpClientPtr;

}

namespace IceInternal
{

void incRef(::SecureUdp::SUdpClient*);
void decRef(::SecureUdp::SUdpClient*);

}

#endif

