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
#include <Glacier2/RequestQueue.h>

namespace Glacier
{

class Blobject : public Ice::BlobjectAsync
{
public:
    
    Blobject(const Ice::CommunicatorPtr&, bool);
    virtual ~Blobject();

    void destroy();

protected:

    void invoke(Ice::ObjectPrx&, const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
		const Ice::Current&);

    const Ice::LoggerPtr _logger;

private:

    const bool _alwaysBatch;
    RequestQueuePtr _requestQueue;
};

}

#endif
