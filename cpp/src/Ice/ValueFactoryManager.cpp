// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ValueFactoryManager.h>
#include <Ice/ValueFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ValueFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(ValueFactoryManager* p) { p->__decRef(); }

void
IceInternal::ValueFactoryManager::install(const ValueFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    _factories.insert(make_pair(id, factory));
}

ValueFactoryPtr
IceInternal::ValueFactoryManager::lookup(const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    map<string, ::Ice::ValueFactoryPtr>::const_iterator p;
    p = _factories.find(id);
    if (p != _factories.end())
	return p->second;
    else
	return 0;
}

IceInternal::ValueFactoryManager::ValueFactoryManager()
{
}

void
IceInternal::ValueFactoryManager::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);
    _factories.clear();
}
