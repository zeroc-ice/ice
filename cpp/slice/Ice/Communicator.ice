// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_ICE
#define ICE_COMMUNICATOR_ICE

#include <Ice/LoggerF.ice>
#include <Ice/ObjectAdapterF.ice>
#include <Ice/PicklerF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/ValueFactoryF.ice>

module Ice
{

local class Communicator
{
    void destroy();
    void shutdown();
    void waitForShutdown();

    Object* stringToProxy(string str);

    ObjectAdapter createObjectAdapter(string name);
    ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpts);

    void installValueFactory(ValueFactory factory, string id);

    Properties getProperties();

    Logger getLogger();
    void setLogger(Logger logger);

    Pickler getPickler();
};

};

#endif
