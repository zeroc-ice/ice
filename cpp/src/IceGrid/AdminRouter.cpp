//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/AdminRouter.h>

using namespace std;
using namespace IceGrid;

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

    if(_traceLevels->admin > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
        out << "routing operation `" << current.operation << "' to `" << target->ice_toString() << "'";
    }

    target->ice_invokeAsync(current.operation, current.mode, inParams,
                            [response, operation = current.operation, traceLevels = _traceLevels, target]
                            (bool ok, auto bytes)
                            {
                                if(traceLevels->admin > 0)
                                {
                                    Ice::Trace out(traceLevels->logger, traceLevels->adminCat);

                                    out << "operation `" << operation << "' routed to `"
                                        << Ice::identityToString(target->ice_getIdentity())
                                        << " -f " << target->ice_getFacet() << "' is returning ";

                                    if(ok)
                                    {
                                        out << "successfully";
                                    }
                                    else
                                    {
                                        out << "a user exception";
                                    }
                                }

                                response(move(ok), move(bytes));
                            },
                            [exception, operation = current.operation, traceLevels = _traceLevels, target]
                            (exception_ptr exptr)
                            {
                                if(traceLevels->admin > 0)
                                {
                                    Ice::Trace out(traceLevels->logger, traceLevels->adminCat);
                                    try
                                    {
                                        rethrow_exception(exptr);
                                    }
                                    catch(const std::exception& ex)
                                    {
                                        out << "operation `" << operation << "' routed to `"
                                            << Ice::identityToString(target->ice_getIdentity())
                                            << " -f " << target->ice_getFacet() << "' failed with " << ex;
                                    }
                                }

                                exception(exptr);
                            },
                            nullptr,
                            current.ctx);
}
