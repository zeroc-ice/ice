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

// TODO: Add slice2docbook comments for this file.
// TODO: Add SecureUdpF.ice for forward declarations?

module IceSecurity
{

module SecureUdp
{

interface ClientChannel
{
    void serverHello(long clientID, Ice::ByteSeq key);

    void serverKeyChange(Ice::ByteSeq key);

    void serverGoodbye();
};

interface ServerChannel
{
    void clientHello(ClientChannel client, Ice::ByteSeq MACkey);

    void clientKeyAcknowledge(long clientID, long msgID, Ice::ByteSeq key);

    void clientKeyRequest(long clientID);

    void clientGoodbye(long clientID);
};

};

};

#endif
