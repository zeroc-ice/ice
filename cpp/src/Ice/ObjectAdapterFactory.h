// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_OBJECT_ADAPTER_FACTORY_H
#define ICE_OBJECT_ADAPTER_FACTORY_H

#include <Ice/ObjectAdapterI.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

namespace IceInternal
{

class ObjectAdapterFactory : public ::IceUtil::Shared, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void shutdown();
    void waitForShutdown();

    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&, const std::string&, const std::string&);
    ::Ice::ObjectAdapterPtr findObjectAdapter(const ::Ice::ObjectPrx&);

private:

    ObjectAdapterFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);
    virtual ~ObjectAdapterFactory();
    friend class Instance;

    InstancePtr _instance;
    ::Ice::CommunicatorPtr _communicator;
    std::map<std::string, ::Ice::ObjectAdapterIPtr> _adapters;
};

}

#endif
