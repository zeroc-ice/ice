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
#include <IceUtil/Mutex.h>
#include <Ice/Buffer.h>
#include <Ice/SUdpClientF.h>
#include <map>

namespace SecureUdp
{

// typedef pair<Ice::Long, CryptKeyPtr> MsgKeyPair;
// typedef vector<MsgKeyPair> MsgKeyVector;

// Encryption key and historical list
// MsgKeyVector _msgEncryptionKeys;
// MsgKeyPair _currentEncryptionInfo;

typedef std::map<Ice::Long, SUdpClientPtr> SUdpClientMap;

class ServerControlChannel : public ControlChannel, public ServerChannel
{

public:

    // Messages received from Client
    virtual void clientHello(const ClientChannelPrx&, const Ice::ByteSeq&, const Ice::Current&);
    virtual void clientKeyAcknowledge(Ice::Long, Ice::Long, const Ice::ByteSeq&, const Ice::Current&);
    virtual void clientKeyRequest(Ice::Long, const Ice::Current&);
    virtual void clientGoodbye(Ice::Long, const Ice::Current&);

protected:

    // ServerControlChannel(const IceInternal::SUdpTransceiverPtr&, const InstancePtr&, int);
    ServerControlChannel(IceInternal::SUdpTransceiver*, const IceInternal::InstancePtr&, int);

    virtual ~ServerControlChannel();

    friend IceInternal::SUdpTransceiver;

    void decryptPacket(IceInternal::Buffer&, IceInternal::Buffer&);
    Ice::Long getNewClientID();

    SUdpClientPtr& getSUdpClient(Ice::Long);
    void newSUdpClient(const SUdpClientPtr&);
    void deleteSUdpClient(Ice::Long);

    Ice::ObjectAdapterPtr _adapter;

    // Keep a listing of all clients connected to us.
    SUdpClientMap _clientMap;
    IceUtil::Mutex _clientMapMutex;

    // Generate unique Client ID numbers
    Ice::Long _clientIDGenerator;
    IceUtil::Mutex _clientIDMutex;
};

}

#endif

