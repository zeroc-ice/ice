// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Ice.h>
#include <IceUtil/Monitor.h>
#include <Glacier/Request.h>

namespace Glacier
{

class TwowayThrottle : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TwowayThrottle(const Ice::CommunicatorPtr&, bool);
    ~TwowayThrottle();

    void twowayStarted(const Ice::ObjectPrx&, const Ice::Current&);
    void twowayFinished();

private:

    const Ice::CommunicatorPtr _communicator;
    const bool _reverse;

    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    const int _traceLevel;
    const int _max;

    int _count;
};

class Blobject : public Ice::BlobjectAsync
{
public:
    
    Blobject(const Ice::CommunicatorPtr&, bool);
    virtual ~Blobject();

    void destroy();
    void invoke(Ice::ObjectPrx&, const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
		const Ice::Current&);
    bool modifyProxy(Ice::ObjectPrx&, const Ice::Current&);

protected:

    Ice::CommunicatorPtr _communicator;
    const bool _reverse;

    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    const int _traceLevel;

private:

    const bool _forwardContext;
    const IceUtil::Time _sleepTime;

    RequestQueuePtr _requestQueue;
    IceUtil::ThreadControl _requestQueueControl;

    TwowayThrottle _twowayThrottle;
};

}

#endif
