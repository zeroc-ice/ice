// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_SYSTEM_H
#define ICE_SSL_SYSTEM_H

#include <string>
#include <IceUtil/Shared.h>
#include <Ice/SslConnectionF.h>
#include <Ice/Properties.h>
#include <Ice/SslSystemF.h>
#include <Ice/SslCertificateVerifierF.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerF.h>

namespace IceSecurity
{

namespace Ssl
{

class Factory;

class System : public IceUtil::Shared
{
public:

    virtual bool isConfigLoaded() = 0;
    virtual void loadConfig() = 0;
    virtual void shutdown() = 0;

    virtual Connection* createServerConnection(int) = 0;
    virtual Connection* createClientConnection(int) = 0;

    virtual void setServerCertificateVerifier(const CertificateVerifierPtr&) = 0;
    virtual void setClientCertificateVerifier(const CertificateVerifierPtr&) = 0;

    virtual void setServerCertAuthorityCertificate(const std::string&) = 0;
    virtual void setClientCertAuthorityCertificate(const std::string&) = 0;

    virtual void setServerRSAKeysBase64(const std::string&, const std::string&) = 0;
    virtual void setClientRSAKeysBase64(const std::string&, const std::string&) = 0;

    virtual void setTrace(const IceInternal::TraceLevelsPtr&);
    bool isTraceSet() const;

    virtual void setLogger(const Ice::LoggerPtr&);
    bool isLoggerSet() const;

    void setProperties(const Ice::PropertiesPtr&);
    bool isPropertiesSet() const;

protected:

    System();
    virtual ~System();

    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
    
    friend class Factory;
};

}

}

#endif
