// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_I_H
#define ICE_SSL_PLUGIN_I_H

#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/PluginBaseIF.h>
#include <IceSSL/CertificateVerifierF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/SslTransceiverF.h>

namespace IceSSL
{

class PluginBaseI : public Plugin
{
public:

    virtual SslTransceiverPtr createTransceiver(ContextType, int) = 0;

    virtual bool isConfigured(ContextType) = 0;

    virtual void configure() = 0;

    virtual void configure(ContextType) = 0;

    virtual void loadConfig(ContextType, const ::std::string&, const ::std::string&) = 0;

    virtual void setCertificateVerifier(ContextType, const CertificateVerifierPtr&) = 0;

    virtual void addTrustedCertificateBase64(ContextType, const std::string&) = 0;

    virtual void addTrustedCertificate(ContextType, const Ice::ByteSeq&) = 0;

    virtual void setRSAKeysBase64(ContextType, const std::string&, const std::string&) = 0;

    virtual void setRSAKeys(ContextType, const ::Ice::ByteSeq&, const ::Ice::ByteSeq&) = 0;

    TraceLevelsPtr getTraceLevels() const;

    Ice::LoggerPtr getLogger() const;

    Ice::PropertiesPtr getProperties() const;

    IceInternal::ProtocolPluginFacadePtr getProtocolPluginFacade() const;

protected:

    PluginBaseI(const IceInternal::ProtocolPluginFacadePtr&);
    virtual ~PluginBaseI();

    IceInternal::ProtocolPluginFacadePtr _protocolPluginFacade;
    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
};

}

#endif
