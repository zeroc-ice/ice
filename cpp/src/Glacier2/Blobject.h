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
#include <Glacier2/Request.h>

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

protected:

    const Ice::LoggerPtr _logger;

private:

    void modifyProxy(Ice::ObjectPrx&, const Ice::Current&) const;

    const bool _alwaysBatch;
    RequestQueuePtr _requestQueue;
    IceUtil::ThreadControl _requestQueueControl;
};

}

#endif
