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

    void setTrace(TraceLevelsPtr traceLevels) { _traceLevels = traceLevels; };
    bool isTraceSet() const { return _traceLevels; };

    void setLogger(LoggerPtr traceLevels) { _logger = traceLevels; };
    bool isLoggerSet() const { return _logger; };

    void setProperties(PropertiesPtr properties) { _properties = properties; };
    bool isPropertiesSet() const { return _properties; };

protected:

    System();
    virtual ~System();

    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;
    PropertiesPtr _properties;
    
    friend class Factory;
};

}

}

#endif
