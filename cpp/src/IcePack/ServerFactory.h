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

#ifndef ICE_PACK_SERVER_FACTORY_H
#define ICE_PACK_SERVER_FACTORY_H

#include <IcePack/Internal.h>
#include <IcePack/Activator.h>
#include <IcePack/WaitQueue.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory : public Ice::ObjectFactory
{
public:

    ServerFactory(const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const Freeze::DBEnvironmentPtr&,
		  const ActivatorPtr&, const WaitQueuePtr&);
    
    //
    // Ice::ObjectFactory method implementation.
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    ServerPrx createServerAndAdapters(const ServerDescription&, const std::vector<std::string>&,
				      std::map<std::string, ServerAdapterPrx>&);

    const WaitQueuePtr& getWaitQueue() const;

private:

    friend class ServerI;
    friend class ServerAdapterI;

    ServerAdapterPrx createServerAdapter(const std::string& name, const ServerPrx& server);

    void destroy(const ServerPtr&, const Ice::Identity&);
    void destroy(const ServerAdapterPtr&, const Ice::Identity&);
    
    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    ActivatorPtr _activator;
    WaitQueuePtr _waitQueue;

    Freeze::EvictorPtr _serverEvictor;
    Freeze::EvictorPtr _serverAdapterEvictor;
    Ice::Int _waitTime;
};

typedef ::IceUtil::Handle< ServerFactory> ServerFactoryPtr;

}

#endif
