// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_H
#define ICE_COMMUNICATOR_H

#include <Ice/CommunicatorF.h>
#include <Ice/InstanceF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API CommunicatorI : public ::__Ice::Shared, public JTCRecursiveMutex
{
public:
    
    void destroy();
    void shutdown();
    void waitForShutdown();

    ObjectProxy stringToProxy(const std::string&);

    ObjectAdapter createObjectAdapter(const std::string&);
    ObjectAdapter createObjectAdapter(const std::string&, const std::string&);

    Properties properties();

    Logger logger();
    void logger(const Logger&);

private:

    CommunicatorI(const Properties&);
    virtual ~CommunicatorI();

    // The following operations may create CommunicatorIs
    friend ICE_API Communicator initialize(int&, char*[]);
    friend ICE_API Communicator initialize(int&, char*[], const Properties&);

    ::__Ice::Instance instance_;
    std::map<std::string, ObjectAdapter> adapters_;
};

ICE_API Communicator initialize(int&, char*[]);
ICE_API Communicator initialize(int&, char*[], const Properties&);

ICE_API Properties createProperties();
ICE_API Properties createProperties(const Properties&);
ICE_API Properties createProperties(const std::string&);

}

#endif
