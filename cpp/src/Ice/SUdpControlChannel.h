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

// using IceInternal::SUdpTransceiverPtr;
using IceInternal::SUdpTransceiver;
using IceInternal::InstancePtr;
using IceUtil::Shared;
using IceUtil::Mutex;
using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

class ControlChannel : public virtual Shared
{

protected:
    // ControlChannel(const SUdpTransceiverPtr&, const InstancePtr&);
    ControlChannel(SUdpTransceiver*, const InstancePtr&);
    virtual ~ControlChannel();

    virtual void unsetTransceiver();

    friend IceInternal::SUdpTransceiver;

    // SUdpTransceiverPtr _transceiver;
    SUdpTransceiver* _transceiver;
    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;
    Mutex _mutex;
    CryptorPtr _cryptor;
};

}

}

#endif

