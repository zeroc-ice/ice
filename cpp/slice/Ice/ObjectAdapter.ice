// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_ICE
#define ICE_OBJECT_ADAPTER_ICE

#include <Ice/CommunicatorF.ice>

module Ice
{

local class ObjectAdapter
{
    string getName();
    Communicator getCommunicator();

    void activate();
    void hold();
    void deactivate();

    void add(Object object, string identity);
    void remove(string identity);

    Object identityToObject(string identity);
    string objectToIdentity(Object object);

    Object proxyToObject(Object* proxy);
    Object* objectToProxy(Object object);

    Object* identityToProxy(string identity);
    string proxyToIdentity(Object* proxy);
};

};

#endif
