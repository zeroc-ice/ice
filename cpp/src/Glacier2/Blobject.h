// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Ice.h>
#include <Glacier2/RequestQueue.h>

namespace Glacier2
{

class Blobject : public Ice::BlobjectAsync
{
public:
    
    Blobject(const Ice::CommunicatorPtr&, bool);
    virtual ~Blobject();

    virtual void destroy();

protected:

    void invoke(Ice::ObjectPrx&, const Ice::AMD_Object_ice_invokePtr&, const Ice::ByteSeq&, const Ice::Current&);

    const Ice::CommunicatorPtr _communicator;
    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;

private:

    const bool _reverse;
    const bool _forwardContext;
    const bool _buffered;
    const bool _alwaysBatch;
    const int _requestTraceLevel;
    const int _overrideTraceLevel;
    RequestQueuePtr _requestQueue;
};

}

#endif
