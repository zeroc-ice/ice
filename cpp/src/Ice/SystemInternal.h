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
#include <Ice/System.h>
#include <Ice/InstanceF.h>
#include <Ice/SslConnectionF.h>
#include <Ice/Properties.h>
#include <Ice/SystemInternalF.h>
#include <Ice/CertificateVerifierF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>

namespace IceSSL
{

class Factory;

class SystemInternal : public System
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

    SystemInternal(const IceInternal::InstancePtr&);
    virtual ~SystemInternal();

    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
    
    friend class Factory;
};

}

#endif
