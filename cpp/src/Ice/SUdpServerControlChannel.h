// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_SERVER_CONTROL_CHANNEL_H
#define ICE_SUDP_SERVER_CONTROL_CHANNEL_H

#include <Ice/SUdpControlChannel.h>
#include <Ice/SecureUdp.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceInternal::SUdpTransceiver;

class ServerControlChannel : public ControlChannel, public ServerChannel
{

public:
    virtual void clientHello(const ClientChannelPtr&, const ::Ice::ByteSeq&, const ::Ice::Current&);

    virtual void clientKeyAcknowledge(::Ice::Long, ::Ice::Long, const ::Ice::Current&);

    virtual void clientKeyRequest(::Ice::Long, const ::Ice::Current&);

    virtual void clientGoodbye(::Ice::Long, const ::Ice::Current&);

protected:

    ServerControlChannel(const SUdpTransceiverPtr&, const InstancePtr&, int);

    virtual ~ServerControlChannel();

    friend IceInternal::SUdpTransceiver;

    int _port;
    Ice::ObjectAdapterPtr _adapter;
};

}

}

#endif
