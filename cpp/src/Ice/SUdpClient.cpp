// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SUdpClient.h>
#include <Ice/CryptKey.h>
#include <Ice/MessageAuthenticator.h>

using Ice::Long;
using IceSecurity::SecureUdp::CryptKeyPtr;
using IceSecurity::SecureUdp::MessageAuthenticatorPtr;

void ::IceInternal::incRef(::IceSecurity::SecureUdp::SUdpClient* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::SecureUdp::SUdpClient* p) { p->__decRef(); }

IceSecurity::SecureUdp::SUdpClient::SUdpClient(Long clientID,
                                               const ClientChannelPtr& clientChannel,
                                               const MessageAuthenticatorPtr& messageAuthenticator) :
                                   _clientID(clientID),
                                   _clientChannel(clientChannel),
                                   _messageAuthenticator(messageAuthenticator)
{
}

IceSecurity::SecureUdp::SUdpClient::~SUdpClient()
{
}

void
IceSecurity::SecureUdp::SUdpClient::serverHello(const CryptKeyPtr& key)
{
    _clientChannel->serverHello(_clientID, key->toByteSeq());
}

void
IceSecurity::SecureUdp::SUdpClient::serverKeyChange(const CryptKeyPtr& key)
{
    _clientChannel->serverKeyChange(key->toByteSeq());
}

void
IceSecurity::SecureUdp::SUdpClient::serverGoodbye()
{
    _clientChannel->serverGoodbye();
}

Long
IceSecurity::SecureUdp::SUdpClient::getClientID() const
{
    return _clientID;
}

const CryptKeyPtr&
IceSecurity::SecureUdp::SUdpClient::getCryptKey() const
{
    return _cryptKey;
}

const CryptKeyPtr&
IceSecurity::SecureUdp::SUdpClient::getCryptKey(Long msgID) const
{
    // TODO: Must be able to return a CryptKey based on a msgID
    return _cryptKey;
}

const MessageAuthenticatorPtr&
IceSecurity::SecureUdp::SUdpClient::getMessageAuthenticator() const
{
    return _messageAuthenticator;
}

void
IceSecurity::SecureUdp::SUdpClient::setNewCryptKey(Long msgID, const CryptKeyPtr& cryptKey)
{
    _cryptKey = cryptKey;
}

