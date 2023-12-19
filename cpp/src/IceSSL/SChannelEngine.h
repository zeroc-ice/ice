//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SCHANNEL_ENGINE_H
#define ICESSL_SCHANNEL_ENGINE_H

#ifdef _WIN32

#include <IceSSL/SSLEngine.h>
#include <IceSSL/SChannelEngineF.h>

//
// SECURITY_WIN32 or SECURITY_KERNEL, must be defined before including security.h
// indicating who is compiling the code.
//
#  ifdef SECURITY_WIN32
#    undef SECURITY_WIN32
#  endif
#  ifdef SECURITY_KERNEL
#    undef SECURITY_KERNEL
#  endif
#  define SECURITY_WIN32 1
#  include <security.h>
#  include <sspi.h>
#  include <schannel.h>
#  undef SECURITY_WIN32

namespace IceSSL
{

namespace SChannel
{

class SSLEngine : public IceSSL::SSLEngine
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);

    //
    // Setup the engine.
    //
    virtual void initialize();

    virtual IceInternal::TransceiverPtr
    createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);

    //
    // Destroy the engine.
    //
    virtual void destroy();

    std::string getCipherName(ALG_ID) const;

    CredHandle newCredentialsHandle(bool);

    HCERTCHAINENGINE chainEngine() const;

private:

    void parseCiphers(const std::string&);

    std::vector<PCCERT_CONTEXT> _allCerts;
    std::vector<PCCERT_CONTEXT> _importedCerts;
    DWORD _protocols;

    std::vector<HCERTSTORE> _stores;
    HCERTSTORE _rootStore;

    HCERTCHAINENGINE _chainEngine;
    std::vector<ALG_ID> _ciphers;

    const bool _strongCrypto;
};

}

}

#endif

#endif
