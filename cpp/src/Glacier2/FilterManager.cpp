// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Logger.h>
#include <Ice/Properties.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/StringUtil.h>
#include <Glacier2/FilterManager.h>
#include <Glacier2/FilterI.h>

using namespace std;
using namespace Ice;

//
// Parse a space delimited string into a sequence of strings.
//

static void
stringToSeq(const string& str, vector<string>& seq)
{
    IceUtilInternal::splitString(str, " \t", seq);

    //
    // TODO: do something about unmatched quotes
    //
}

static void
stringToSeq(const string& str, vector<Identity>& seq)
{
    string const ws = " \t";

    //
    // Eat white space.
    //
    string::size_type current = str.find_first_not_of(ws, 0);
    string::size_type end = 0;
    while(current != string::npos)
    {
        switch(str[current])
        {
        case '"':
        case '\'':
        {
            char quote = str[current];
            end = current+1;
            while(true)
            {
                end = str.find(quote, end);

                if(end == string::npos)
                {
                    //
                    // TODO: should this be an unmatched quote error?
                    //
                    seq.push_back(stringToIdentity(str.substr(current)));
                    break;
                }

                bool markString = true;
                for(string::size_type r = end -1 ; r > current && str[r] == '\\' ; --r)
                {
                    markString = !markString;
                }
                //
                // We don't want the quote so we skip that.
                //
                if(markString)
                {
                    ++current;
                    seq.push_back(stringToIdentity(str.substr(current, end-current)));
                    break;
                }
                else
                {
                    ++end;
                }
            }
            if(end != string::npos)
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

Glacier2::FilterManager::~FilterManager()
{
    destroy();
}

void
Glacier2::FilterManager::destroy()
{
    Ice::ObjectAdapterPtr adapter = _instance->serverObjectAdapter();
    if(adapter)
    {
        try
        {
            if(_categoriesPrx)
            {
                adapter->remove(_categoriesPrx->ice_getIdentity());
            }
        }
        catch(const Exception&)
        {
        }
        try
        {
            if(_adapterIdsPrx)
            {
                adapter->remove(_adapterIdsPrx->ice_getIdentity());
            }
        }
        catch(const Exception&)
        {
        }
        try
        {
            if(_identitiesPrx)
            {
                adapter->remove(_identitiesPrx->ice_getIdentity());
            }
        }
        catch(const Exception&)
        {
        }
    }
}

Glacier2::FilterManager::FilterManager(const InstancePtr& instance, const Glacier2::StringSetIPtr& categories,
                                       const Glacier2::StringSetIPtr& adapters,
                                       const Glacier2::IdentitySetIPtr& identities) :
    _categories(categories),
    _adapters(adapters),
    _identities(identities),
    _instance(instance)
{
    try
    {
        Ice::ObjectAdapterPtr adapter = _instance->serverObjectAdapter();
        if(adapter)
        {
            _categoriesPrx = Glacier2::StringSetPrx::uncheckedCast(adapter->addWithUUID(_categories));
            _adapterIdsPrx = Glacier2::StringSetPrx::uncheckedCast(adapter->addWithUUID(_adapters));
            _identitiesPrx = Glacier2::IdentitySetPrx::uncheckedCast(adapter->addWithUUID(_identities));
        }
    }
    catch(...)
    {
        destroy();
        throw;
    }
}

Glacier2::FilterManagerPtr
Glacier2::FilterManager::create(const InstancePtr& instance, const string& userId, const bool allowAddUser)
{
    PropertiesPtr props = instance->properties();
    string allow = props->getProperty("Glacier2.Filter.Category.Accept");
    vector<string> allowSeq;
    stringToSeq(allow, allowSeq);

    if(allowAddUser)
    {
        int addUserMode = 0;
        if(!props->getProperty("Glacier2.Filter.Category.AcceptUser").empty())
        {
            addUserMode = props->getPropertyAsInt("Glacier2.Filter.Category.AcceptUser");
        }

        if(addUserMode > 0 && !userId.empty())
        {
            if(addUserMode == 1)
            {
                allowSeq.push_back(userId); // Add user id to allowed categories.
            }
            else if(addUserMode == 2)
            {
                allowSeq.push_back('_' + userId); // Add user id with prepended underscore to allowed categories.
            }
        }
    }
    Glacier2::StringSetIPtr categoryFilter = new Glacier2::StringSetI(allowSeq);

    //
    // TODO: refactor initialization of filters.
    //
    allow = props->getProperty("Glacier2.Filter.AdapterId.Accept");
    stringToSeq(allow, allowSeq);
    Glacier2::StringSetIPtr adapterIdFilter = new Glacier2::StringSetI(allowSeq);

    //
    // TODO: Object id's from configurations?
    //
    IdentitySeq allowIdSeq;
    allow = props->getProperty("Glacier2.Filter.Identity.Accept");
    stringToSeq(allow, allowIdSeq);
    Glacier2::IdentitySetIPtr identityFilter = new Glacier2::IdentitySetI(allowIdSeq);

    return new Glacier2::FilterManager(instance, categoryFilter, adapterIdFilter, identityFilter);
}
