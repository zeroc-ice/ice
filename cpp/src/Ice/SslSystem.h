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
#include <Ice/SslConnection.h>
#include <Ice/Properties.h>

namespace IceSecurity
{

namespace Ssl
{

using std::string;
using Ice::LoggerPtr;
using Ice::PropertiesPtr;

class Factory;

// TODO: Can we derive this from Shared?  How hard would that be?

class System
{

public:
    inline string getSystemID() const { return _systemID; };

    virtual bool isConfigLoaded() = 0;
    virtual void loadConfig() = 0;
    virtual void shutdown() = 0;

    virtual Connection* createServerConnection(int) = 0;
    virtual Connection* createClientConnection(int) = 0;

    void setTrace(TraceLevelsPtr traceLevels) { _traceLevels = traceLevels; };
    bool isTraceSet() const { return (_traceLevels == 0 ? false : true); };

    void setLogger(LoggerPtr traceLevels) { _logger = traceLevels; };
    bool isLoggerSet() const { return (_logger == 0 ? false : true); };

    void setProperties(PropertiesPtr properties) { _properties = properties; };
    bool isPropertiesSet() const { return (_properties == 0 ? false : true); };

protected:

    System(string&);
    virtual ~System();

    // Reference counting.
    inline void incRef() { _refCount++; };
    inline bool decRef() { return (--_refCount ? true : false); };

    string _systemID;
    int _refCount;

    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;
    PropertiesPtr _properties;

friend class Factory;

};

}

}

#endif
