// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include <InstanceF.h>
#include <Context.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/Plugin.h>

namespace IceSSL
{

class Instance : public IceUtil::Shared
{
public:

    Instance(const Ice::CommunicatorPtr&);

    void initialize(SSL_CTX*);
    void setCertificateVerifier(const CertificateVerifierPtr&);
    void setPasswordPrompt(const PasswordPromptPtr&);

    Ice::CommunicatorPtr communicator() const;
    std::string defaultHost() const;
    int networkTraceLevel() const;
    std::string networkTraceCategory() const;
    int securityTraceLevel() const;
    std::string securityTraceCategory() const;

    ContextPtr context() const;

    CertificateVerifierPtr certificateVerifier() const;
    PasswordPromptPtr passwordPrompt() const;

    std::string sslErrors() const;

    void destroy();

private:

    IceInternal::ProtocolPluginFacadePtr _facade;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
    ContextPtr _context;
    CertificateVerifierPtr _verifier;
    PasswordPromptPtr _prompt;
};

}

#endif
