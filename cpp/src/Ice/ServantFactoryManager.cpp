// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ServantFactoryManager.h>
#include <Ice/ServantFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ServantFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(ServantFactoryManager* p) { p->__decRef(); }

void
IceInternal::ServantFactoryManager::install(const ServantFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    _factories.insert(make_pair(id, factory));
}

ServantFactoryPtr
IceInternal::ServantFactoryManager::lookup(const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    map<string, ::Ice::ServantFactoryPtr>::const_iterator p;
    p = _factories.find(id);
    if (p != _factories.end())
    {
	return p->second;
    }
    else
    {
	return 0;
    }
}

IceInternal::ServantFactoryManager::ServantFactoryManager()
{
}

void
IceInternal::ServantFactoryManager::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);
    _factories.clear();
}
