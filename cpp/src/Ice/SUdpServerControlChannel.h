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

namespace IceSecurity
{

namespace SecureUdp
{

// typedef pair<Long, CryptKeyPtr> MsgKeyPair;
// typedef vector<MsgKeyPair> MsgKeyVector;

// Encryption key and historical list
// MsgKeyVector _msgEncryptionKeys;
// MsgKeyPair _currentEncryptionInfo;

using IceInternal::Buffer;
using IceInternal::SUdpTransceiver;
using Ice::ObjectAdapterPtr;
using Ice::Long;
using Ice::ByteSeq;
using Ice::Current;

typedef std::map<Long, SUdpClientPtr> SUdpClientMap;

class ServerControlChannel : public ControlChannel, public ServerChannel
{

public:

    // Messages received from Client
    virtual void clientHello(const ClientChannelPrx&, const ByteSeq&, const Current&);
    virtual void clientKeyAcknowledge(Long, Long, const ByteSeq&, const Current&);
    virtual void clientKeyRequest(Long, const Current&);
    virtual void clientGoodbye(Long, const Current&);

protected:

    // ServerControlChannel(const SUdpTransceiverPtr&, const InstancePtr&, int);
    ServerControlChannel(SUdpTransceiver*, const InstancePtr&, int);

    virtual ~ServerControlChannel();

    friend IceInternal::SUdpTransceiver;

    void decryptPacket(Buffer&, Buffer&);
    Long getNewClientID();

    SUdpClientPtr& getSUdpClient(Long);
    void newSUdpClient(const SUdpClientPtr&);
    void deleteSUdpClient(Long);

    ObjectAdapterPtr _adapter;

    // Keep a listing of all clients connected to us.
    SUdpClientMap _clientMap;
    Mutex _clientMapMutex;

    // Generate unique Client ID numbers
    Long _clientIDGenerator;
    Mutex _clientIDMutex;
};

}

}

#endif

