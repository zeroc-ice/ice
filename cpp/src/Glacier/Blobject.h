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
#include <Glacier/Missive.h>

namespace Glacier
{

class Blobject : public Ice::BlobjectAsync
{
public:
    
    Blobject(const Ice::CommunicatorPtr&);
    virtual ~Blobject();

    virtual bool reverse() = 0;
    
    void destroy();
    void invoke(Ice::ObjectPrx&, const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
		const Ice::Current&);
    MissiveQueuePtr modifyProxy(Ice::ObjectPrx&, const Ice::Current&);

protected:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;

    int _traceLevel;
    bool _forwardContext;
    IceUtil::Time _batchSleepTime;

private:

    MissiveQueuePtr getMissiveQueue();

    MissiveQueuePtr _missiveQueue;
    IceUtil::ThreadControl _missiveQueueControl;
    IceUtil::Mutex _missiveQueueMutex;
};

}

#endif
