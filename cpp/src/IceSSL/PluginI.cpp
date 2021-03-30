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
    return info->verified ? NoError : UnknownTrustFailure;
}

std::string
IceSSL::getTrustErrorDescription(TrustError error)
{
    switch(error)
    {
        case NoError:
        {
            return "no error";
        }
        case ExplicitDistrust:
        {
            return "the certificate is explicitly distrusted";
        }
        case HasExcludedNameConstraint:
        {
            return "the X509 chain is invalid because a certificate has excluded a name constraint";
        }
        case HasNotDefinedNameConstraint:
        {
            return "the certificate has an undefined name constraint";
        }
        case HasNotPermittedNameConstraint:
        {
            return "the certificate has an impermissible name constrain";
        }
        case HasNotSupportedCriticalExtension:
        {
            return "the certificate does not support a critical extension";
        }
        case HasNotSupportedNameConstraint:
        {
            return "the certificate does not have a supported name constraint or has a name constraint that "
                   "is unsupported";
        }
        case HostNameMismatch:
        {
            return "host name mismatch has occurred";
        }
        case InvalidBasicConstraints:
        {
            return "the X509 chain is invalid due to invalid basic constraints";
        }
        case InvalidExtension:
        {
            return "the X509 chain is invalid due to an invalid extension";
        }
        case InvalidNameConstraints:
        {
            return "the X509 chain is invalid due to invalid name constraints";
        }
        case InvalidPolicyConstraints:
        {
            return "the X509 chain is invalid due to invalid policy constraints";
        }
        case NotSignatureValid:
        {
            return "the X509 chain is invalid due to an invalid certificate signature";
        }
        case NotTimeValid:
        {
            return "the X509 chain is not valid due to an invalid time value, such as a value that indicates an "
                   "expired certificate";
        }
        case NotValidForUsage:
        {
            return "the key usage is not valid";
        }
        case PartialChain:
        {
            return "the X509 chain could not be built up to the root certificate";
        }
        case RevocationStatusUnknown:
        {
            return "it is not possible to determine whether the certificate has been revoke";
        }
        case Revoked:
        {
            return "the X509 chain is invalid due to a revoked certificate";
        }
        case UntrustedRoot:
        {
            return "the X509 chain is invalid due to an untrusted root certificate";
        }
        case UnknownTrustFailure:
        {
            return "unknown failure";
        }
    }
    assert(false);
    return "unknown failure";
}
