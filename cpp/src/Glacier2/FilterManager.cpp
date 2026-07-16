// Copyright (c) ZeroC, Inc.

#include "FilterManager.h"
#include "FilterT.h"
#include "Ice/Communicator.h"
#include "Ice/Ice.h"
#include "Ice/Logger.h"

using namespace std;
using namespace Ice;

Glacier2::FilterManager::~FilterManager() { destroy(); }

void
Glacier2::FilterManager::destroy()
{
    auto adapter = _instance->serverObjectAdapter();
    if (adapter)
    {
        try
        {
            if (_categoriesPrx)
            {
                adapter->remove(_categoriesPrx->ice_getIdentity());
            }
        }
        catch (...)
        {
        }
        try
        {
            if (_adapterIdsPrx)
            {
                adapter->remove(_adapterIdsPrx->ice_getIdentity());
            }
        }
        catch (...)
        {
        }
        try
        {
            if (_identitiesPrx)
            {
                adapter->remove(_identitiesPrx->ice_getIdentity());
            }
        }
        catch (...)
        {
        }
    }
}

Glacier2::FilterManager::FilterManager(
    shared_ptr<Instance> instance,
    shared_ptr<Glacier2::StringSetI> categories,
    shared_ptr<Glacier2::StringSetI> adapters,
    shared_ptr<Glacier2::IdentitySetI> identities)
    : _categories(std::move(categories)),
      _adapters(std::move(adapters)),
      _identities(std::move(identities)),
      _instance(std::move(instance))
{
    try
    {
        auto adapter = _instance->serverObjectAdapter();
        if (adapter)
        {
            _categoriesPrx = adapter->addWithUUID<Glacier2::StringSetPrx>(_categories);
            _adapterIdsPrx = adapter->addWithUUID<Glacier2::StringSetPrx>(_adapters);
            _identitiesPrx = adapter->addWithUUID<Glacier2::IdentitySetPrx>(_identities);
        }
    }
    catch (...)
    {
        destroy();
        throw;
    }
}

shared_ptr<Glacier2::FilterManager>
Glacier2::FilterManager::create(shared_ptr<Instance> instance, const string& userId, bool allowAddUser)
{
    vector<string> allowSeq = instance->filterCategories();

    if (allowAddUser)
    {
        int addUserMode = instance->filterAddUserMode();

        if (addUserMode > 0 && !userId.empty())
        {
            if (addUserMode == 1)
            {
                allowSeq.push_back(userId); // Add user id to allowed categories.
            }
            else if (addUserMode == 2)
            {
                allowSeq.push_back('_' + userId); // Add user id with prepended underscore to allowed categories.
            }
        }
    }

    auto categoryFilter = make_shared<Glacier2::StringSetI>(allowSeq);
    auto adapterIdFilter = make_shared<Glacier2::StringSetI>(instance->filterAdapterIds());
    auto identityFilter = make_shared<Glacier2::IdentitySetI>(instance->filterIdentities());

    return make_shared<Glacier2::FilterManager>(
        std::move(instance),
        std::move(categoryFilter),
        std::move(adapterIdFilter),
        std::move(identityFilter));
}
