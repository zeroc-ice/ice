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
#include <Ice/SecureUdp.h>

using Ice::Long;
using SecureUdp::CryptKeyPtr;
using SecureUdp::MessageAuthenticatorPtr;

void ::IceInternal::incRef(::SecureUdp::SUdpClient* p) { p->__incRef(); }
void ::IceInternal::decRef(::SecureUdp::SUdpClient* p) { p->__decRef(); }

SecureUdp::SUdpClient::SUdpClient(Long clientID,
                                               const ClientChannelPrx& clientChannel,
                                               const MessageAuthenticatorPtr& messageAuthenticator) :
                                   _clientID(clientID),
                                   _clientChannel(clientChannel),
                                   _messageAuthenticator(messageAuthenticator)
{
    assert(_clientChannel);
    assert(_messageAuthenticator);
}

SecureUdp::SUdpClient::~SUdpClient()
{
}

void
SecureUdp::SUdpClient::serverHello(const CryptKeyPtr& key)
{
    assert(_clientChannel);
    _clientChannel->serverHello(_clientID, key->toByteSeq());
}

void
SecureUdp::SUdpClient::serverKeyChange(const CryptKeyPtr& key)
{
    assert(_clientChannel);
    _clientChannel->serverKeyChange(key->toByteSeq());
}

void
SecureUdp::SUdpClient::serverGoodbye()
{
    assert(_clientChannel);
    _clientChannel->serverGoodbye();
}

Long
SecureUdp::SUdpClient::getClientID() const
{
    return _clientID;
}

const CryptKeyPtr&
SecureUdp::SUdpClient::getCryptKey() const
{
    return _cryptKey;
}

const CryptKeyPtr&
SecureUdp::SUdpClient::getCryptKey(Long msgID) const
{
    // TODO: Must be able to return a CryptKey based on a msgID
    return _cryptKey;
}

const MessageAuthenticatorPtr&
SecureUdp::SUdpClient::getMessageAuthenticator() const
{
    return _messageAuthenticator;
}

void
SecureUdp::SUdpClient::setNewCryptKey(Long msgID, const CryptKeyPtr& cryptKey)
{
    _cryptKey = cryptKey;
}

