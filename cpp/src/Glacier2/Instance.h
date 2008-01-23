// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_INSTANCE_H
#define GLACIER2_INSTANCE_H

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <IceUtil/Time.h>

#include <Glacier2/RequestQueue.h>

namespace Glacier2
{

class Instance : public IceUtil::Shared
{
public:

    Instance(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&);
    ~Instance();

    //
    // COMPILERFIX: returning const& is necessary on HP-UX (otherwise, it crashes in the Blobject
    // constructor).
    //
    const Ice::CommunicatorPtr& communicator() const { return _communicator; }
    const Ice::ObjectAdapterPtr& clientObjectAdapter() const { return _clientAdapter; }
    const Ice::ObjectAdapterPtr& serverObjectAdapter() const { return _serverAdapter; }
    const Ice::PropertiesPtr& properties() const { return _properties; }
    const Ice::LoggerPtr& logger() const { return _logger; }

    const RequestQueueThreadPtr& clientRequestQueueThread() const { return _clientRequestQueueThread; }
    const RequestQueueThreadPtr& serverRequestQueueThread() const { return _serverRequestQueueThread; }

    void destroy();
    
private:

    const Ice::CommunicatorPtr _communicator;
    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const Ice::ObjectAdapterPtr _serverAdapter;
    const RequestQueueThreadPtr _clientRequestQueueThread;
    const RequestQueueThreadPtr _serverRequestQueueThread;
};
typedef IceUtil::Handle<Instance> InstancePtr;

} // End namespace Glacier2

#endif
