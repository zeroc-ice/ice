//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/AdminCallbackRouter.h>

using namespace std;
using namespace IceGrid;

void
AdminCallbackRouter::addMapping(const string& category, const shared_ptr<Ice::Connection>& con)
{
    lock_guard lock(_mutex);

#ifdef NDEBUG
    _categoryToConnection.insert({ category, con });
#else
    bool inserted = _categoryToConnection.insert({ category, con }).second;
    assert(inserted == true);
#endif
}

void
AdminCallbackRouter::removeMapping(const string& category)
{
    lock_guard lock(_mutex);

#ifndef NDEBUG
    size_t one =
#endif
        _categoryToConnection.erase(category);

    assert(one == 1);
}

void
AdminCallbackRouter::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                                     function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                     function<void(exception_ptr)> exception,
                                     const Ice::Current& current)
{
    shared_ptr<Ice::Connection> con;

    {
        lock_guard lock(_mutex);
        auto p = _categoryToConnection.find(current.id.category);
        if(p == _categoryToConnection.end())
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        con = p->second;
    }

    auto target = con->createProxy(current.id)->ice_facet(current.facet);

    //
    // Call with AMI
    //
    target->ice_invokeAsync(current.operation, current.mode, inParams,
                            move(response),
                            [exception = move(exception)] (exception_ptr)
                            {
                                exception(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
                            },
                            nullptr,
                            current.ctx);
}
