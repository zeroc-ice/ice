// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/AdminRouter.h>

using namespace Ice;
using namespace std;

namespace
{

class CallbackI : public IceUtil::Shared
{
public:

    CallbackI(const AMD_Object_ice_invokePtr& cb) :
        _cb(cb)
    {
    }

    void response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        _cb->ice_response(ok, outParams);
    }
    
    void exception(const Ice::Exception&)
    {
        // Admin object is unreachable
        _cb->ice_exception(ObjectNotExistException(__FILE__, __LINE__));
    }
    
private:
    AMD_Object_ice_invokePtr _cb;
};

}

void
IceGrid::AdminRouter::ice_invoke_async(const AMD_Object_ice_invokePtr& cb, 
                                       const pair<const Byte*, const Byte*>& inParams,
                                       const Current& current)
{
    ObjectPrx target = getTarget(current);
    assert(target != 0);

    //
    // Call with AMI
    //
    Callback_Object_ice_invokePtr amiCb = 
        newCallback_Object_ice_invoke(new CallbackI(cb), &CallbackI::response, &CallbackI::exception);

    target->begin_ice_invoke(current.operation, current.mode, inParams, current.ctx, amiCb);
}

