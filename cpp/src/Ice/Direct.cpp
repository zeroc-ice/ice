// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Direct.h>
#include <Ice/ObjectAdapterI.h> // For inc/decDirectCount().
#include <Ice/ServantManager.h>
#include <Ice/ServantLocator.h>
#include <Ice/Reference.h>
#include <Ice/Object.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Direct::Direct(const Current& current) :
    _current(current)
{
    ObjectAdapterI* adapter = dynamic_cast<ObjectAdapterI*>(_current.adapter.get());
    assert(adapter);

    //
    // Must call incDirectCount() first, because it checks for adapter
    // deactivation, and prevents deactivation completion until
    // decDirectCount() is called. This is important, because
    // getServantManager() may not be called afer deactivation
    // completion.
    //
    adapter->incDirectCount();

    ServantManagerPtr servantManager = adapter->getServantManager();
    assert(servantManager);

    _servant = servantManager->findServant(_current.id, _current.facet);
    if(!_servant)
    {
        _locator = servantManager->findServantLocator(_current.id.category);
        if(!_locator && !_current.id.category.empty())
        {
            _locator = servantManager->findServantLocator("");
        }
        if(_locator)
        {
            try
            {
                _servant = _locator->locate(_current, _cookie);
            }
            catch(...)
            {
                adapter->decDirectCount();
                throw;
            }
        }
    }

    if(!_servant)
    {
        adapter->decDirectCount();
        if(servantManager && servantManager->hasServant(_current.id))
        {
            FacetNotExistException ex(__FILE__, __LINE__);
            ex.id = _current.id;
            ex.facet = _current.facet;
            ex.operation = _current.operation;
            throw ex;
        }
        else
        {
            ObjectNotExistException ex(__FILE__, __LINE__);
            ex.id = _current.id;
            ex.facet = _current.facet;
            ex.operation = _current.operation;
            throw ex;
        }
    }
}

bool
IceInternal::Direct::isCollocated()
{
    return true;
}

const Current&
IceInternal::Direct::getCurrent()
{
    return _current;
}

void
IceInternal::Direct::throwUserException()
{
    if(_userException.get() == 0)
    {
        assert(0); // should never happen
        throw Ice::UnknownUserException(__FILE__, __LINE__);
    }
    _userException->ice_throw();
}

void 
IceInternal::Direct::setUserException(const Ice::UserException& ue)
{
    _userException.reset(ue.ice_clone());
}

void
IceInternal::Direct::destroy()
{
    //
    // NOTE: we can't do the following in the destructor because it
    // might throw.
    //

    ObjectAdapterI* adapter = dynamic_cast<ObjectAdapterI*>(_current.adapter.get());
    assert(adapter);

    if(_locator && _servant)
    {
        try
        {
            _locator->finished(_current, _servant, _cookie);
        }
        catch(...)
        {
            adapter->decDirectCount();
            throw;
        }
    }

    adapter->decDirectCount();
    if(_userException.get())
    {
        _userException->ice_throw();
    }
}

const ObjectPtr&
IceInternal::Direct::getServant()
{
    return _servant;
}
