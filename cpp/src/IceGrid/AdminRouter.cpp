// Copyright (c) ZeroC, Inc.

#include "AdminRouter.h"

using namespace std;
using namespace IceGrid;

IceGrid::AdminRouter::AdminRouter(const shared_ptr<TraceLevels>& traceLevels) : _traceLevels(traceLevels) {}

void
IceGrid::AdminRouter::invokeOnTarget(
    const Ice::ObjectPrx& target,
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)>&& response,
    function<void(exception_ptr)>&& exception,
    const Ice::Current& current)
{
    if (_traceLevels->admin > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
        out << "routing operation '" << current.operation << "' to '" << target->ice_toString() << "'";
    }

    target->ice_invokeAsync(
        current.operation,
        current.mode,
        inParams,
        [response, operation = current.operation, traceLevels = _traceLevels, target](bool ok, auto bytes)
        {
            if (traceLevels->admin > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->adminCat);

                out << "operation '" << operation << "' routed to '" << Ice::identityToString(target->ice_getIdentity())
                    << " -f " << target->ice_getFacet() << "' is returning ";

                if (ok)
                {
                    out << "successfully";
                }
                else
                {
                    out << "a user exception";
                }
            }

            response(ok, std::move(bytes));
        },
        [exception, operation = current.operation, traceLevels = _traceLevels, target](exception_ptr exptr)
        {
            if (traceLevels->admin > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->adminCat);
                try
                {
                    rethrow_exception(exptr);
                }
                catch (const std::exception& ex)
                {
                    out << "operation '" << operation << "' routed to '"
                        << Ice::identityToString(target->ice_getIdentity()) << " -f " << target->ice_getFacet()
                        << "' failed with " << ex;
                }
            }

            exception(exptr);
        },
        nullptr,
        current.ctx);
}
