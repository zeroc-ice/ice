// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_CONTROL_CHANNEL_F_H
#define ICE_SUDP_CONTROL_CHANNEL_F_H

#include <Ice/Handle.h>

namespace SecureUdp
{

class ControlChannel;
typedef IceInternal::Handle<ControlChannel> ControlChannelPtr;

}

namespace IceInternal
{

void incRef(::SecureUdp::ControlChannel*);
void decRef(::SecureUdp::ControlChannel*);

}

#endif
