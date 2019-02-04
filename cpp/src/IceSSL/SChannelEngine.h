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

#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))

//
// Add some definitions missing from MinGW headers.
//

#   ifndef CERT_TRUST_IS_EXPLICIT_DISTRUST
#      define CERT_TRUST_IS_EXPLICIT_DISTRUST 0x04000000
#   endif

#   ifndef CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT
#      define CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT 0x08000000
#   endif

#   ifndef SECBUFFER_ALERT
#      define SECBUFFER_ALERT 17
#   endif

#   ifndef SCH_SEND_ROOT_CERT
#      define SCH_SEND_ROOT_CERT 0x00040000
#   endif

#   ifndef SP_PROT_TLS1_1_SERVER
#      define SP_PROT_TLS1_1_SERVER 0x00000100
#   endif

#   ifndef SP_PROT_TLS1_1_CLIENT
#      define SP_PROT_TLS1_1_CLIENT 0x00000200
#   endif

#   ifndef SP_PROT_TLS1_2_SERVER
#      define SP_PROT_TLS1_2_SERVER 0x00000400
#   endif

#   ifndef SP_PROT_TLS1_2_CLIENT
#      define SP_PROT_TLS1_2_CLIENT 0x00000800
#   endif

#endif

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

    virtual void verifyPeer(const std::string&, const ConnectionInfoPtr&, const std::string&);

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
