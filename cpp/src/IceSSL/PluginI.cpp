//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/EndpointI.h>

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LocalException.h>
#include <Ice/RegisterPlugins.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

#ifndef ICE_CPP11_MAPPING
CertificateVerifier::~CertificateVerifier()
{
    // Out of line to avoid weak vtable
}

PasswordPrompt::~PasswordPrompt()
{
    // Out of line to avoid weak vtable
}
#endif

IceSSL::Plugin::~Plugin()
{
    // Out of line to avoid weak vtable
}

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com, const SSLEnginePtr& engine) :
    _engine(engine)
{
    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    InstancePtr instance = new Instance(_engine, SSLEndpointType, "ssl"); // SSL based on TCP
    IceInternal::getProtocolPluginFacade(com)->addEndpointFactory(new EndpointFactoryI(instance, TCPEndpointType));
}

void
PluginI::initialize()
{
    _engine->initialize();
}

void
PluginI::destroy()
{
    _engine->destroy();
    _engine = 0;
}

#ifdef ICE_CPP11_MAPPING
void
PluginI::setCertificateVerifier(std::function<bool(const std::shared_ptr<IceSSL::ConnectionInfo>&)> verifier)
{
    if(verifier)
    {
        _engine->setCertificateVerifier(make_shared<CertificateVerifier>(std::move(verifier)));
    }
    else
    {
        _engine->setCertificateVerifier(nullptr);
    }
}
#else
void
PluginI::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _engine->setCertificateVerifier(verifier);
}
#endif

#ifdef ICE_CPP11_MAPPING
void
PluginI::setPasswordPrompt(std::function<std::string()> prompt)
{
     if(prompt)
     {
         _engine->setPasswordPrompt(make_shared<PasswordPrompt>(std::move(prompt)));
     }
     else
     {
         _engine->setPasswordPrompt(nullptr);
     }
}
#else
void
PluginI::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _engine->setPasswordPrompt(prompt);
}
#endif

extern "C"
{

ICESSL_API Ice::Plugin*
createIceSSL(const CommunicatorPtr&, const string&, const StringSeq&);

}

namespace Ice
{

ICESSL_API void
registerIceSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSL, loadOnInitialize);
}

}

//
// Objective-C function to allow Objective-C programs to register plugin.
//
extern "C" ICESSL_API void
ICEregisterIceSSL(bool loadOnInitialize)
{
    Ice::registerIceSSL(loadOnInitialize);
}

IceSSL::TrustError
IceSSL::getTrustError(const IceSSL::ConnectionInfoPtr& info)
{
    ExtendedConnectionInfoPtr extendedInfo = ICE_DYNAMIC_CAST(ExtendedConnectionInfo, info);
    if (extendedInfo)
    {
        return extendedInfo->errorCode;
    }
    return info->verified ? IceSSL::ICE_ENUM(TrustError, NoError) : IceSSL::ICE_ENUM(TrustError, UnknownTrustFailure);
}

std::string
IceSSL::getTrustErrorDescription(TrustError error)
{
    switch(error)
    {
        case  IceSSL::ICE_ENUM(TrustError, NoError):
        {
            return "no error";
        }
        case IceSSL::ICE_ENUM(TrustError, ChainTooLong):
        {
            return "the certificate chain length is greater than the specified maximum depth";
        }
        case IceSSL::ICE_ENUM(TrustError, HasExcludedNameConstraint):
        {
            return "the X509 chain is invalid because a certificate has excluded a name constraint";
        }
        case IceSSL::ICE_ENUM(TrustError, HasNonDefinedNameConstraint):
        {
            return "the certificate has an undefined name constraint";
        }
        case IceSSL::ICE_ENUM(TrustError, HasNonPermittedNameConstraint):
        {
            return "the certificate has a non permitted name constrain";
        }
        case IceSSL::ICE_ENUM(TrustError, HasNonSupportedCriticalExtension):
        {
            return "the certificate does not support a critical extension";
        }
        case IceSSL::ICE_ENUM(TrustError, HasNonSupportedNameConstraint):
        {
            return "the certificate does not have a supported name constraint or has a name constraint that "
                   "is unsupported";
        }
        case IceSSL::ICE_ENUM(TrustError, HostNameMismatch):
        {
            return "a host name mismatch has occurred";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidBasicConstraints):
        {
            return "the X509 chain is invalid due to invalid basic constraints";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidExtension):
        {
            return "the X509 chain is invalid due to an invalid extension";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidNameConstraints):
        {
            return "the X509 chain is invalid due to invalid name constraints";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidPolicyConstraints):
        {
            return "the X509 chain is invalid due to invalid policy constraints";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidPurpose):
        {
            return "the supplied certificate cannot be used for the specified purpose";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidSignature):
        {
            return "the X509 chain is invalid due to an invalid certificate signature";
        }
        case IceSSL::ICE_ENUM(TrustError, InvalidTime):
        {
            return "the X509 chain is not valid due to an invalid time value, such as a value that indicates an "
                   "expired certificate";
        }
        case IceSSL::ICE_ENUM(TrustError, NotTrusted):
        {
            return "the certificate is explicitly distrusted";
        }
        case IceSSL::ICE_ENUM(TrustError, PartialChain):
        {
            return "the X509 chain could not be built up to the root certificate";
        }
        case IceSSL::ICE_ENUM(TrustError, RevocationStatusUnknown):
        {
            return "it is not possible to determine whether the certificate has been revoked";
        }
        case IceSSL::ICE_ENUM(TrustError, Revoked):
        {
            return "the X509 chain is invalid due to a revoked certificate";
        }
        case IceSSL::ICE_ENUM(TrustError, UntrustedRoot):
        {
            return "the X509 chain is invalid due to an untrusted root certificate";
        }
        case IceSSL::ICE_ENUM(TrustError, UnknownTrustFailure):
        {
            return "unknown failure";
        }
    }
    assert(false);
    return "unknown failure";
}

std::string
IceSSL::getHost(const IceSSL::ConnectionInfoPtr& info)
{
    ExtendedConnectionInfoPtr extendedInfo = ICE_DYNAMIC_CAST(ExtendedConnectionInfo, info);
    return extendedInfo ? extendedInfo->host : "";
}
