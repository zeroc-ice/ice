// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURE_UDP_ICE
#define ICE_SECURE_UDP_ICE

#include <Ice/Stream.ice>

module IceSecurity
{

module SecureUdp
{

class ClientChannel
{
    void serverHello(long clientID, Ice::ByteSeq key);

    void serverKeyChange(Ice::ByteSeq key);

    void serverGoodbye();
};

class ServerChannel
{
    void clientHello(ClientChannel client, Ice::ByteSeq MACkey);

    void clientKeyAcknowledge(long clientID, long msgID);

    void clientKeyRequest(long clientID);

    void clientGoodbye(long clientID);
};

};

};

#endif
