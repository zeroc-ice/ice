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

using std::string;
using Ice::LoggerPtr;
using IceInternal::TraceLevelsPtr;
using Ice::PropertiesPtr;
using IceUtil::Shared;

class Factory;

class System : public Shared
{
public:

    virtual bool isConfigLoaded() = 0;
    virtual void loadConfig() = 0;
    virtual void shutdown() = 0;

    virtual Connection* createServerConnection(int) = 0;
    virtual Connection* createClientConnection(int) = 0;

    virtual void setServerCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setClientCertificateVerifier(const CertificateVerifierPtr&);

    void setTrace(const TraceLevelsPtr&);
    bool isTraceSet() const;

    void setLogger(const LoggerPtr&);
    bool isLoggerSet() const;

    void setProperties(const PropertiesPtr&);
    bool isPropertiesSet() const;

protected:

    System();
    virtual ~System();

    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;
    PropertiesPtr _properties;
    CertificateVerifierPtr _clientVerifier;
    CertificateVerifierPtr _serverVerifier;
    
    friend class Factory;
};

}

}

#endif
