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
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/SUdpControlChannelF.h>
#include <Ice/SUdpTransceiverF.h>
#include <Ice/CryptorF.h>

namespace IceSecurity
{

namespace SecureUdp
{

class ControlChannel : public virtual IceUtil::Shared
{

protected:
    // ControlChannel(const IceInternal::SUdpTransceiverPtr&, const IceInternal::InstancePtr&);
    ControlChannel(IceInternal::SUdpTransceiver*, const IceInternal::InstancePtr&);
    virtual ~ControlChannel();

    virtual void unsetTransceiver();

    friend IceInternal::SUdpTransceiver;

    // IceInternal::SUdpTransceiverPtr _transceiver;
    IceInternal::SUdpTransceiver* _transceiver;
    IceInternal::InstancePtr _instance;
    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    IceUtil::Mutex _mutex;
    CryptorPtr _cryptor;
};

}

}

#endif

