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

    TwowayThrottle(int);
    ~TwowayThrottle();

    void twowayStarted();
    void twowayFinished();

private:

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
