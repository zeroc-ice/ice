// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>
#include <IceUtil/StaticMutex.h>
#include <IceUtil/GCF.h>

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>

namespace Ice
{

class CommunicatorI : public Communicator, public ::IceUtil::RecMutex
{
public:
    
    virtual void destroy();
    virtual void shutdown();
    virtual void waitForShutdown();

    virtual ObjectPrx stringToProxy(const std::string&);
    virtual std::string proxyToString(const ObjectPrx&);

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual void removeObjectFactory(const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&);

    virtual PropertiesPtr getProperties();

    virtual LoggerPtr getLogger();
    virtual void setLogger(const LoggerPtr&);

    virtual StatsPtr getStats();
    virtual void setStats(const StatsPtr&);

    virtual void setDefaultRouter(const RouterPrx&);

    virtual void setDefaultLocator(const LocatorPrx&);

    virtual PluginManagerPtr getPluginManager();

    virtual void flushBatchRequests();

private:

    CommunicatorI(int&, char*[], const PropertiesPtr&);
    virtual ~CommunicatorI();

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);

    bool _destroyed;
    ::IceInternal::InstancePtr _instance;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;

    struct GarbageCollectorStats
    {
	GarbageCollectorStats()
	    : runs(0), examined(0), collected(0), msec(0.0)
	{
	}
	int runs;
	int examined;
	int collected;
	double msec;
    };

    static void printGCStats(const ::IceUtil::GCStats&);

    static int _communicatorCount;
    static ::IceUtil::StaticMutex _gcMutex;
    static GarbageCollectorStats _gcStats;
    static int _gcTraceLevel;
    static std::string _gcTraceCat;
    static LoggerPtr _gcLogger;
};

}

#endif
