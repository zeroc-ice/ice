// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_FACTORY_H
#define ICE_PACK_SERVER_FACTORY_H

#include <IcePack/Internal.h>
#include <IcePack/Activator.h>
#include <IcePack/WaitQueue.h>
#include <Freeze/Evictor.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory : public Ice::ObjectFactory
{
public:

    ServerFactory(const Ice::ObjectAdapterPtr&, 
		  const TraceLevelsPtr&, 
		  const std::string&,
		  const ActivatorPtr&, 
		  const WaitQueuePtr&);
    
    //
    // Ice::ObjectFactory method implementation.
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    void checkConsistency();

    ServerPrx createServer(const std::string&, const ServerDescriptorPtr&);
    ServerAdapterPrx createServerAdapter(const std::string& name, const ServerPrx& server);

    const WaitQueuePtr& getWaitQueue() const;
    const Freeze::EvictorPtr& getServerEvictor() const;
    const Freeze::EvictorPtr& getServerAdapterEvictor() const;

private:

    friend class ServerI;
    friend class ServerAdapterI;

    void destroy(const ServerPtr&, const Ice::Identity&);
    void destroy(const ServerAdapterPtr&, const Ice::Identity&);

    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    ActivatorPtr _activator;
    WaitQueuePtr _waitQueue;

    Freeze::EvictorPtr _serverEvictor;
    Freeze::EvictorPtr _serverAdapterEvictor;
    Ice::Int _waitTime;
    std::string _serversDir;
};

typedef ::IceUtil::Handle< ServerFactory> ServerFactoryPtr;

}

#endif
