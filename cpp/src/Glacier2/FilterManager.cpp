// Copyright (c) ZeroC, Inc.

#include "FilterManager.h"
#include "FilterT.h"
#include "Ice/Communicator.h"
#include "Ice/Ice.h"
#include "Ice/Logger.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"

using namespace std;
using namespace Ice;

// Parse a space delimited string into a sequence of strings.

static void
stringToSeq(const string& str, vector<string>& seq)
{
    IceInternal::splitString(str, " \t", seq);

    // TODO: do something about unmatched quotes
}

static void
stringToSeq(const string& str, vector<Identity>& seq)
{
    string const ws = " \t";

    // Eat white space.
    string::size_type current = str.find_first_not_of(ws, 0);
    string::size_type end = 0;
    while (current != string::npos)
    {
        switch (str[current])
        {
            case '"':
            case '\'':
            {
                char quote = str[current];
                end = current + 1;
                while (true)
                {
                    end = str.find(quote, end);

                    if (end == string::npos)
                    {
                        // TODO: should this be an unmatched quote error?
                        seq.push_back(stringToIdentity(str.substr(current)));
                        break;
                    }

                    bool markString = true;
                    for (string::size_type r = end - 1; r > current && str[r] == '\\'; --r)
                    {
                        markString = !markString;
                    }
                    // We don't want the quote so we skip that.
                    if (markString)
                    {
                        ++current;
                        seq.push_back(stringToIdentity(str.substr(current, end - current)));
                        break;
                    }
                    else
                    {
                        ++end;
                    }
                }
                if (end != string::npos)
                {
                    ++end;
                }
                break;
            }

            default:
            {
                end = str.find_first_of(ws, current);
                string::size_type len = (end == string::npos) ? string::npos : end - current;
                seq.push_back(stringToIdentity(str.substr(current, len)));
                break;
            }
        }
        current = str.find_first_not_of(ws, end);
    }
}

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
        catch (const Exception&)
        {
        }
        try
        {
            if (_adapterIdsPrx)
            {
                adapter->remove(_adapterIdsPrx->ice_getIdentity());
            }
        }
        catch (const Exception&)
        {
        }
        try
        {
            if (_identitiesPrx)
            {
                adapter->remove(_identitiesPrx->ice_getIdentity());
            }
        }
        catch (const Exception&)
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
    auto props = instance->properties();
    string allow = props->getIceProperty("Glacier2.Filter.Category.Accept");
    vector<string> allowSeq;
    stringToSeq(allow, allowSeq);

    if (allowAddUser)
    {
        int addUserMode = props->getIcePropertyAsInt("Glacier2.Filter.Category.AcceptUser");

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

    // TODO: refactor initialization of filters.
    allow = props->getIceProperty("Glacier2.Filter.AdapterId.Accept");
    stringToSeq(allow, allowSeq);
    auto adapterIdFilter = make_shared<Glacier2::StringSetI>(allowSeq);

    // TODO: Object id's from configurations?
    IdentitySeq allowIdSeq;
    allow = props->getIceProperty("Glacier2.Filter.Identity.Accept");
    stringToSeq(allow, allowIdSeq);
    auto identityFilter = make_shared<Glacier2::IdentitySetI>(allowIdSeq);

    return make_shared<Glacier2::FilterManager>(
        std::move(instance),
        std::move(categoryFilter),
        std::move(adapterIdFilter),
        std::move(identityFilter));
}
