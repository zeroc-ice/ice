// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_CLIENT_CONTROL_CHANNEL_H
#define ICE_SUDP_CLIENT_CONTROL_CHANNEL_H

#include <Ice/SUdpControlChannel.h>
#include <Ice/SecureUdp.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceInternal::SUdpTransceiver;
using Ice::Long;
using Ice::ByteSeq;
using Ice::Current;

class ClientControlChannel : public ControlChannel, public ClientChannel
{

public:

    virtual void serverHello(Long, const ByteSeq&, const Current&);

    virtual void serverKeyChange(const ByteSeq&, const Current&);

    virtual void serverGoodbye(const Current&);

protected:

    ClientControlChannel(const SUdpTransceiverPtr&, const InstancePtr&, const std::string&, int);

    virtual ~ClientControlChannel();

    friend IceInternal::SUdpTransceiver;

    std::string _host;
    int _port;
    Ice::ObjectAdapterPtr _adapter;
};

}

}

#endif
