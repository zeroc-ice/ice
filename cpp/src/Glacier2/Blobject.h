// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class Blobject : public Ice::BlobjectArrayAsync
{
public:
    
    Blobject(const Ice::CommunicatorPtr&, bool, const Ice::Context&);
    virtual ~Blobject();

    virtual void destroy();

protected:

    void invoke(Ice::ObjectPrx&, const Ice::AMD_Array_Object_ice_invokePtr&, 
                const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

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
    const RequestQueuePtr _requestQueue;
    const Ice::Context _sslContext;
};

}

#endif
