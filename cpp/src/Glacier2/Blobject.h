// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Ice.h>
#include <Glacier2/RequestQueue.h>
#include <Glacier2/Instance.h>

namespace Glacier2
{

class Blobject : public Ice::BlobjectArrayAsync
{
public:

    Blobject(const InstancePtr&, const Ice::ConnectionPtr&, const Ice::Context&);
    virtual ~Blobject();

    void destroy();

    virtual void updateObserver(const Glacier2::Instrumentation::SessionObserverPtr&);

    void invokeResponse(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                        const Ice::AMD_Object_ice_invokePtr&);
    void invokeSent(bool, const Ice::AMD_Object_ice_invokePtr&);
    void invokeException(const Ice::Exception&, const Ice::AMD_Object_ice_invokePtr&);

protected:

    void invoke(Ice::ObjectPrx&, const Ice::AMD_Object_ice_invokePtr&,
                const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

    const InstancePtr _instance;
    const Ice::ConnectionPtr _reverseConnection;

private:

    const bool _forwardContext;
    const bool _alwaysBatch;
    const int _requestTraceLevel;
    const int _overrideTraceLevel;
    const RequestQueuePtr _requestQueue;
    const Ice::Context _context;
};

}

#endif
