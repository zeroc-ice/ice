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
#include <Ice/ValueFactoryF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API Communicator : public ::__Ice::Shared, public JTCRecursiveMutex
{
public:
    
    void destroy();
    void shutdown();
    void waitForShutdown();

    Object_pptr stringToProxy(const std::string&);

    ObjectAdapter_ptr createObjectAdapter(const std::string&);
    ObjectAdapter_ptr createObjectAdapter(const std::string&, const std::string&);

    void installValueFactory(const ValueFactory_ptr&);

    Properties_ptr properties();

    Logger_ptr logger();
    void logger(const Logger_ptr&);

private:

    Communicator(const Properties_ptr&);
    virtual ~Communicator();

    // The following operations may create CommunicatorIs
    friend ICE_API Communicator_ptr initialize(int&, char*[]);
    friend ICE_API Communicator_ptr initialize(int&, char*[], const Properties_ptr&);

    ::__Ice::Instance_ptr instance_;
    std::map<std::string, ObjectAdapter_ptr> adapters_;
};

ICE_API Communicator_ptr initialize(int&, char*[]);
ICE_API Communicator_ptr initialize(int&, char*[], const Properties_ptr&);

ICE_API Properties_ptr createProperties();
ICE_API Properties_ptr createProperties(const Properties_ptr&);
ICE_API Properties_ptr createProperties(const std::string&);

}

#endif
