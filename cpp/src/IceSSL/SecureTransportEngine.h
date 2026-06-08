//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_H

#ifdef __APPLE__

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
    virtual ~SSLEngine();

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

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    // Path of the temporary directory holding the imported certificate's keychain, removed by the
    // destructor. Empty when IceSSL.Keychain is set or no certificate is configured.
    std::string _temporaryKeychainDir;
#endif

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

#endif
