// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Evictor.h>

using namespace std;
using namespace Ice;

Evictor::Evictor(const PhoneBookIPtr& phoneBook) :
    _phoneBook(phoneBook),
    _evictorSize(5) // TODO: Configurable
{
}

Evictor::~Evictor()
{
}

ObjectPtr
Evictor::locate(const ObjectAdapterPtr&, const string& identity, ObjectPtr&)
{
    JTCSyncT<JTCMutex> sync(*this);
    
    std::map<std::string, EvictorEntry>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	assert(*(p->second._position) == identity);
	_evictorList.erase(p->second._position);
	_evictorList.push_front(identity);
	p->second._position = _evictorList.begin();

	return p->second._servant;
    }
    else
    {
	//
	// With most STL implementations, _evictorMap.size() is faster
	// than _evictorList.size().
	//
	if (_evictorMap.size() > _evictorSize)
	{
	    string evictedIdentity = _evictorList.back();
	    _evictorList.pop_back();
	    _evictorMap.erase(evictedIdentity);
	    assert(_evictorMap.size() == _evictorSize);
	}

	_evictorList.push_front(identity);

	EvictorEntry evictorEntry;
	evictorEntry._servant = new EntryI(identity, _phoneBook);
	evictorEntry._position = _evictorList.begin();
	_evictorMap[identity] = evictorEntry;

	return evictorEntry._servant;
    }
}

void
Evictor::finished(const ObjectAdapterPtr&, const string&, const ObjectPtr&, const ObjectPtr&)
{
}
