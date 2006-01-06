// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_CACHE_H
#define ICE_GRID_CACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceGrid/Util.h>
#include <IceGrid/TraceLevels.h>

namespace IceGrid
{


template<typename Key, typename Value>
class Cache : public IceUtil::Mutex
{
    typedef IceUtil::Handle<Value> ValuePtr;

public:

    Cache() : _entriesHint(_entries.end())
    {
    }

    virtual 
    ~Cache()
    {
    }

    bool 
    has(const Key& key)
    {
	Lock sync(*this);
	return getImpl(key);
    }

    ValuePtr
    remove(const Key& key)
    {
	Lock sync(*this);
	return removeImpl(key);
    }

    void 
    setTraceLevels(const TraceLevelsPtr& traceLevels)
    { 
	_traceLevels = traceLevels;
    }

    const TraceLevelsPtr& getTraceLevels() const { return _traceLevels; }

protected:

    virtual ValuePtr 
    getImpl(const Key& key, bool create = false)
    {
	typename std::map<Key, ValuePtr>::iterator p = _entries.end();
	if(_entriesHint != _entries.end())
	{
	    if(_entriesHint->first == key)
	    {
		p = _entriesHint;
	    }
	}
	
	if(p == _entries.end())
	{
	    p = _entries.find(key);
	}
	
	if(p != _entries.end())
	{
	    _entriesHint = p;
	    return p->second;
	}
	else
	{
	    if(create)
	    {
		return createAndAddImpl(key);
	    }
	    else
	    {
		return 0;
	    }
	}
    }

    virtual ValuePtr
    createAndAddImpl(const Key& key)
    {
	return addImpl(key, createEntry(key));
    }

    virtual ValuePtr
    addImpl(const Key& key, const ValuePtr& entry)
    {
	typename std::map<Key, ValuePtr>::value_type v(key, entry);
	_entriesHint = _entries.insert(_entriesHint, v);
	return entry;
    }

    virtual ValuePtr
    removeImpl(const Key& key)
    {
	typename std::map<Key, ValuePtr>::iterator p = _entries.end();
	if(_entriesHint != _entries.end())
	{
	    if(_entriesHint->first == key)
	    {
		p = _entriesHint;
	    }
	}
	
	if(p == _entries.end())
	{
	    p = _entries.find(key);
	}
	
	assert(p != _entries.end());
	if(p->second->canRemove())
	{
	    ValuePtr entry = p->second;
	    _entries.erase(p);
	    _entriesHint = _entries.end();
	    return entry;
	}
	else
	{
	    _entriesHint = p;
	    return p->second;
	}
    }

    virtual ValuePtr
    createEntry(const Key& key)
    {
	return new Value(*this, key);
    }

    TraceLevelsPtr _traceLevels;
    std::map<Key, ValuePtr> _entries;
    typename std::map<Key, ValuePtr>::iterator _entriesHint;    
};

template<typename T>
class CacheByString : public Cache<std::string, T>
{
    typedef IceUtil::Handle<T> TPtr;

public:

    virtual std::vector<std::string>
    getAll(const std::string& expr)
    {
	IceUtil::Mutex::Lock sync(*this);
	return getMatchingKeys<std::map<std::string,TPtr> >(Cache<std::string, T>::_entries, expr);
    }
};

};

#endif
