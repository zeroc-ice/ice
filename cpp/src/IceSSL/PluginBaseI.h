// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_I_H
#define ICE_SSL_PLUGIN_I_H

#include <Ice/InstanceF.h>
#include <Ice/Properties.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/PluginBaseIF.h>
#include <IceSSL/SslConnectionF.h>
#include <IceSSL/CertificateVerifierF.h>

namespace IceSSL
{

class PluginBaseI : public IceSSL::Plugin
{
public:

    virtual ConnectionPtr createConnection(ContextType, int) = 0;

    virtual bool isConfigured(ContextType) = 0;

    virtual void configure() = 0;

    virtual void configure(ContextType) = 0;

    virtual void loadConfig(ContextType, const ::std::string&, const ::std::string&) = 0;

    virtual void setCertificateVerifier(ContextType, const CertificateVerifierPtr&) = 0;

    virtual void addTrustedCertificateBase64(ContextType, const std::string&) = 0;

    virtual void addTrustedCertificate(ContextType, const Ice::ByteSeq&) = 0;

    virtual void setRSAKeysBase64(ContextType, const std::string&, const std::string&) = 0;

    virtual void setRSAKeys(ContextType, const ::Ice::ByteSeq&, const ::Ice::ByteSeq&) = 0;

protected:

    PluginBaseI(const IceInternal::InstancePtr&);
    virtual ~PluginBaseI();

    IceInternal::InstancePtr _instance;
    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
};

}

#endif
