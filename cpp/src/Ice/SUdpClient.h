// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_CLIENT_H
#define ICE_SUDP_CLIENT_H

#include <Ice/SecureUdpF.h>
#include <Ice/Stream.h>
#include <IceUtil/Shared.h>
#include <Ice/SUdpClientF.h>
#include <Ice/MessageAuthenticatorF.h>
#include <Ice/CryptKeyF.h>

namespace SecureUdp
{

class SUdpClient : public IceUtil::Shared
{

public:
    SUdpClient(Ice::Long, const ClientChannelPrx&, const MessageAuthenticatorPtr&);
    virtual ~SUdpClient();

    void serverHello(const CryptKeyPtr&);

    void serverKeyChange(const CryptKeyPtr&);

    void serverGoodbye();

    void setNewCryptKey(Ice::Long, const CryptKeyPtr&);

    Ice::Long getClientID() const;

    const CryptKeyPtr& getCryptKey() const;
    const CryptKeyPtr& getCryptKey(Ice::Long) const;

    const MessageAuthenticatorPtr& getMessageAuthenticator() const;

protected:
    Ice::Long _clientID;
    ClientChannelPrx _clientChannel;
    MessageAuthenticatorPtr _messageAuthenticator;
    CryptKeyPtr _cryptKey;

};

}

#endif


