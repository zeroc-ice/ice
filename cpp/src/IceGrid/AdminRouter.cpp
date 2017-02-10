// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    CallbackI(const AMD_Object_ice_invokePtr& cb, const Ice::ObjectPrx& target, const string& operation,
              const IceGrid::TraceLevelsPtr& traceLevels) :
        _cb(cb),
        _target(target),
        _operation(operation),
        _traceLevels(traceLevels)
    {
    }

    void response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);

            out << "operation `" << _operation << "' routed to `" << Ice::identityToString(_target->ice_getIdentity())
                << " -f " << _target->ice_getFacet() << "' is returning ";
            if(ok)
            {
                out << "successfully";
            }
            else
            {
                out << "a user exception";
            }
        }
        _cb->ice_response(ok, outParams);
    }

    void exception(const Ice::Exception& ex)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            out << "operation `" << _operation << "' routed to `" << Ice::identityToString(_target->ice_getIdentity())
                << " -f " << _target->ice_getFacet() << "' failed with " << ex;
        }
        // Admin object is unreachable
        _cb->ice_exception(ObjectNotExistException(__FILE__, __LINE__));
    }

private:
    AMD_Object_ice_invokePtr _cb;
    Ice::ObjectPrx _target;
    string _operation;
    const IceGrid::TraceLevelsPtr _traceLevels;
};

}

IceGrid::AdminRouter::AdminRouter(const TraceLevelsPtr& traceLevels) :
    _traceLevels(traceLevels)
{
}

void
IceGrid::AdminRouter::invokeOnTarget(const Ice::ObjectPrx& target,
                                     const AMD_Object_ice_invokePtr& cb,
                                     const pair<const Byte*, const Byte*>& inParams,
                                     const Current& current)
{
    assert(target != 0);

    //
    // Call with AMI
    //
    Callback_Object_ice_invokePtr amiCb = newCallback_Object_ice_invoke(
        new CallbackI(cb, target, current.operation, _traceLevels), &CallbackI::response, &CallbackI::exception);

    if(_traceLevels->admin > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
        out << "routing operation `" << current.operation << "' to `" << target->ice_toString() << "'";
    }
    target->begin_ice_invoke(current.operation, current.mode, inParams, current.ctx, amiCb);
}
