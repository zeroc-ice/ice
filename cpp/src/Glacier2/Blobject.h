// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

    class InvokeCookie : public Ice::LocalObject
    {
    public:
        
        InvokeCookie(const Ice::AMD_Object_ice_invokePtr& cb) :
            _cb(cb)
        {
        }
        
        Ice::AMD_Object_ice_invokePtr cb()
        {
            return _cb;
        }
        
    private:
        
        Ice::AMD_Object_ice_invokePtr _cb;
    };
    typedef IceUtil::Handle<InvokeCookie> InvokeCookiePtr;
    
    void invokeResponse(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                        const InvokeCookiePtr&);
    void invokeSent(bool, const InvokeCookiePtr&);
    void invokeException(const Ice::Exception&, const InvokeCookiePtr&);

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
