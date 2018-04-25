// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_H

#include <IceSSL/SSLEngine.h>
#include <Ice/UniqueRef.h>

#include <Security/Security.h>
#include <Security/SecureTransport.h>

namespace IceSSL
{

namespace SecureTransport
{

class SSLEngine : public IceSSL::SSLEngine
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();
    virtual IceInternal::TransceiverPtr
    createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);

    SSLContextRef newContext(bool);
    CFArrayRef getCertificateAuthorities() const;
    std::string getCipherName(SSLCipherSuite) const;

private:

    void parseCiphers(const std::string&);

    IceInternal::UniqueRef<CFArrayRef> _certificateAuthorities;
    IceInternal::UniqueRef<CFArrayRef> _chain;

    SSLProtocol _protocolVersionMax;
    SSLProtocol _protocolVersionMin;

#if TARGET_OS_IPHONE==0
    std::vector<char> _dhParams;
#endif
    std::vector<SSLCipherSuite> _ciphers;
};

} // SecureTransport namespace end

} // IceSSL namespace end

#endif
