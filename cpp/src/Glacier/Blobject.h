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
#include <Glacier/Request.h>

namespace Glacier
{

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
    const Ice::LoggerPtr _logger;
    const bool _reverse;
    const int _traceLevel;
    const bool _forwardContext;
    const IceUtil::Time _batchSleepTime;

private:

    RequestQueuePtr _requestQueue;
    IceUtil::ThreadControl _requestQueueControl;
};

}

#endif
