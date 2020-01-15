//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/AdminRouter.h>

using namespace std;
using namespace IceGrid;

namespace
{

class AdminCallback
{
public:

    AdminCallback(function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                  function<void(exception_ptr)> exception,
                  const shared_ptr<Ice::ObjectPrx>& target,
                  const string& operation,
                  const shared_ptr<TraceLevels>& traceLevels) :
        _responseCallback(move(response)),
        _exceptionCallback(move(exception)),
        _target(target),
        _operation(operation),
        _traceLevels(traceLevels)
    {
    }

    void response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
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
        _responseCallback(ok, outParams);
    }

    void exception(exception_ptr exptr)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            try
            {
                rethrow_exception(exptr);
            }
            catch(const std::exception& ex)
            {
                out << "operation `" << _operation << "' routed to `"
                    << Ice::identityToString(_target->ice_getIdentity())
                    << " -f " << _target->ice_getFacet() << "' failed with " << ex;
            }
        }
        // Admin object is unreachable
        _exceptionCallback(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
    }

private:

    const function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> _responseCallback;
    const function<void(exception_ptr)> _exceptionCallback;
    const shared_ptr<Ice::ObjectPrx> _target;
    const string _operation;
    const shared_ptr<TraceLevels> _traceLevels;
};

}

IceGrid::AdminRouter::AdminRouter(const shared_ptr<TraceLevels>& traceLevels) :
    _traceLevels(traceLevels)
{
}

void
IceGrid::AdminRouter::invokeOnTarget(const shared_ptr<Ice::ObjectPrx>& target,
                                     const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                                     function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)>&& response,
                                     function<void(exception_ptr)>&& exception,
                                     const Ice::Current& current)
{
    assert(target);

    //
    // Call with AMI
    //
    auto cb = make_shared<AdminCallback>(move(response), move(exception), target, current.operation, _traceLevels);

    if(_traceLevels->admin > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
        out << "routing operation `" << current.operation << "' to `" << target->ice_toString() << "'";
    }

    target->ice_invokeAsync(current.operation, current.mode, inParams,
                            [cb] (bool ok, auto bytes)
                            {
                                cb->response(move(ok), move(bytes));
                            },
                            [cb] (exception_ptr ex)
                            {
                                cb->exception(ex);
                            },
                            nullptr,
                            current.ctx);
}
