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

class ClientControlChannel : public ControlChannel, public ClientChannel
{

public:

    // Messages received from the Server
    virtual void serverHello(Ice::Long, const Ice::ByteSeq&, const Ice::Current&);
    virtual void serverKeyChange(const Ice::ByteSeq&, const Ice::Current&);
    virtual void serverGoodbye(const Ice::Current&);


protected:

    // ClientControlChannel(const IceInternal::SUdpTransceiverPtr&, const Ice::InstancePtr&, const std::string&, int);
    ClientControlChannel(IceInternal::SUdpTransceiver*, const IceInternal::InstancePtr&, const std::string&, int);

    virtual ~ClientControlChannel();

    void serverKeyChangeMessage(const Ice::ByteSeq&);
    void clientHello();


    friend IceInternal::SUdpTransceiver;

    // Called from the IceInternal::SUdpTransceiver
    void encryptPacket(IceInternal::Buffer&, IceInternal::Buffer&);
    void clientKeyRequest();

    Ice::ObjectAdapterPtr _adapter;
    ClientChannelPrx _clientProxy;
    ServerChannelPrx _serverChannel;

    Ice::Long _msgID;
    Ice::Long _clientID;
    CryptKeyPtr _encryptionKey;

    MessageAuthenticatorPtr _messageAuthenticator;
};

}

}

#endif

