//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ServerBlobject::ServerBlobject(shared_ptr<Instance> instance, shared_ptr<Connection> connection) :
    Glacier2::Blobject(move(instance), move(connection), Ice::Context())
{
}

Glacier2::ServerBlobject::~ServerBlobject()
{
}

void
Glacier2::ServerBlobject::ice_invokeAsync(pair<const Byte*, const Byte*> inParams,
                                          function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                                          function<void(exception_ptr)> error,
                                          const Current& current)
{
    auto proxy = _reverseConnection->createProxy(current.id);
    assert(proxy);

    invoke(proxy, move(response), move(error), inParams, current);
}
