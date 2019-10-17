//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ENGINE_H
#define ICESSL_ENGINE_H

#include <IceSSL/Plugin.h>
#include <IceSSL/Util.h>
#include <IceSSL/SSLEngineF.h>
#include <IceSSL/TrustManagerF.h>
#include <IceSSL/InstanceF.h>

#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Network.h>
#include <Ice/TransceiverF.h>

namespace IceSSL
{

class ICESSL_API SSLEngine : public IceUtil::Shared
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);

    Ice::CommunicatorPtr communicator() const { return _communicator; }
    Ice::LoggerPtr getLogger() const { return _logger; };

    void setCertificateVerifier(const CertificateVerifierPtr&);
    void setPasswordPrompt(const PasswordPromptPtr&);
    std::string password(bool);

    //
    // Setup the engine.
    //
    virtual void initialize() = 0;

    virtual bool initialized() const;

    //
    // Destroy the engine.
    //
    virtual void destroy() = 0;

    //
    // Create a transceiver using the engine specific implementation
    //
    virtual IceInternal::TransceiverPtr
    createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool) = 0;

    //
    // Verify peer certificate
    //
    virtual void verifyPeer(const std::string&, const ConnectionInfoPtr&, const std::string&);
    void verifyPeerCertName(const std::string&, const ConnectionInfoPtr&);

    CertificateVerifierPtr getCertificateVerifier() const;
    PasswordPromptPtr getPasswordPrompt() const;

    std::string getPassword() const;
    void setPassword(const std::string& password);

    bool getCheckCertName() const;
    bool getServerNameIndication() const;
    int getVerifyPeer() const;
    int securityTraceLevel() const;
    std::string securityTraceCategory() const;

protected:

    bool _initialized;
    IceUtil::Mutex _mutex;

private:

    const Ice::CommunicatorPtr _communicator;
    const Ice::LoggerPtr _logger;
    const TrustManagerPtr _trustManager;

    std::string _password;
    CertificateVerifierPtr _verifier;
    PasswordPromptPtr _prompt;

    bool _checkCertName;
    bool _serverNameIndication;
    int _verifyDepthMax;
    int _verifyPeer;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
};

}

#endif
