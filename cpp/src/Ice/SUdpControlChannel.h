// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_CONTROL_CHANNEL_H
#define ICE_SUDP_CONTROL_CHANNEL_H

#include <Ice/InstanceF.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/SUdpControlChannelF.h>
#include <Ice/SUdpTransceiverF.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceInternal::SUdpTransceiverPtr;
using IceInternal::InstancePtr;
using IceUtil::Shared;
using IceUtil::Mutex;

class ControlChannel : public virtual Shared
{

protected:

    ControlChannel(const SUdpTransceiverPtr&, const InstancePtr&);
    virtual ~ControlChannel();

    virtual void unsetTransceiver();

    friend IceInternal::SUdpTransceiver;

    SUdpTransceiverPtr _transceiver;
    InstancePtr _instance;
    Mutex _mutex;
};

}

}

#endif
