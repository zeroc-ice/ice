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
#include <Ice/Buffer.h>
#include <Ice/CryptKeyF.h>
#include <Ice/MessageAuthenticatorF.h>

namespace IceSecurity
{

namespace SecureUdp
{

using IceInternal::SUdpTransceiver;
using IceInternal::Buffer;
using Ice::Long;
using Ice::ByteSeq;
using Ice::Current;


class ClientControlChannel : public ControlChannel, public ClientChannel
{

public:

    // Messages received from the Server
    virtual void serverHello(Long, const ByteSeq&, const Current&);
    virtual void serverKeyChange(const ByteSeq&, const Current&);
    virtual void serverGoodbye(const Current&);


protected:

    // ClientControlChannel(const SUdpTransceiverPtr&, const InstancePtr&, const std::string&, int);
    ClientControlChannel(SUdpTransceiver*, const InstancePtr&, const std::string&, int);

    virtual ~ClientControlChannel();

    void serverKeyChangeMessage(const ByteSeq&);
    void clientHello();


    friend IceInternal::SUdpTransceiver;

    // Called from the SUdpTransceiver
    void encryptPacket(Buffer&, Buffer&);
    void clientKeyRequest();


    Ice::ObjectAdapterPtr _adapter;
    ClientChannelPrx _clientProxy;
    ServerChannelPrx _serverChannel;

    Long _msgID;
    Long _clientID;
    CryptKeyPtr _encryptionKey;

    MessageAuthenticatorPtr _messageAuthenticator;
};

}

}

#endif

