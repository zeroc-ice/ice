// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

//
// It'd be better if we didn't have to include this everywhere, but
// this is the most expeditious approach for now.
//
#include <Glacier2/Instance.h>
#include <Glacier2/FilterI.h>
#include <Ice/ObjectAdapter.h>

namespace Glacier2
{

class FilterManager;
typedef IceUtil::Handle<FilterManager> FilterManagerPtr;

class FilterManager : public IceUtil::Shared
{
public:
    virtual ~FilterManager();

    void destroy();

    StringSetIPtr 
    categories() const
    {
        return _categories;
    }

    StringSetIPtr 
    adapterIds() const
    {
        return _adapters;
    }

    IdentitySetIPtr 
    identities() const
    {
        return _identities;
    }

    StringSetPrx 
    categoriesPrx() const
    {
        return _categoriesPrx;
    }

    StringSetPrx
    adapterIdsPrx() const
    {
        return _adapterIdsPrx;
    }

    IdentitySetPrx
    identitiesPrx() const
    {
        return _identitiesPrx;
    }

    static FilterManagerPtr 
    create(const InstancePtr&, const std::string&, const bool);

private:

    StringSetPrx _categoriesPrx;
    StringSetPrx _adapterIdsPrx;
    IdentitySetPrx _identitiesPrx;

    const StringSetIPtr _categories;
    const StringSetIPtr _adapters;
    const IdentitySetIPtr _identities;
    const InstancePtr _instance;

    FilterManager(const InstancePtr& , const StringSetIPtr&, const StringSetIPtr&, const IdentitySetIPtr&);
};
};

#endif /* FILTER_MANAGER_H */

