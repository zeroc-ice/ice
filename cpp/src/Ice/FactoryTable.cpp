// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/FactoryTable.h>
#include <Ice/ObjectFactory.h>

//
// Add a factory to the exception factory table.
// If the factory is present already, increment its reference count.
//
void
IceInternal::FactoryTable::addExceptionFactory(const std::string& t, const IceInternal::UserExceptionFactoryPtr& f)
{
    IceUtil::Mutex::Lock lock(_m);
    assert(f);
    EFTable::iterator i = _eft.find(t);
    if(i == _eft.end())
    {
        _eft[t] = EFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the exception factory for a given type ID
//
IceInternal::UserExceptionFactoryPtr
IceInternal::FactoryTable::getExceptionFactory(const std::string& t) const
{
    IceUtil::Mutex::Lock lock(_m);
    EFTable::const_iterator i = _eft.find(t);
    return i != _eft.end() ? i->second.first : IceInternal::UserExceptionFactoryPtr();
}

//
// Remove a factory from the exception factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count; if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeExceptionFactory(const std::string& t)
{
    IceUtil::Mutex::Lock lock(_m);
    EFTable::iterator i = _eft.find(t);
    if(i != _eft.end())
    {
        if(--i->second.second == 0)
        {
            _eft.erase(i);
        }
    }
}

//
// Add a factory to the object factory table.
//
void
IceInternal::FactoryTable::addObjectFactory(const std::string& t, const Ice::ObjectFactoryPtr& f)
{
    IceUtil::Mutex::Lock lock(_m);
    assert(f);
    OFTable::iterator i = _oft.find(t);
    if(i == _oft.end())
    {
        _oft[t] = OFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the object factory for a given type ID
//
Ice::ObjectFactoryPtr
IceInternal::FactoryTable::getObjectFactory(const std::string& t) const
{
    IceUtil::Mutex::Lock lock(_m);
    OFTable::const_iterator i = _oft.find(t);
    return i != _oft.end() ? i->second.first : Ice::ObjectFactoryPtr();
}

//
// Remove a factory from the object factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeObjectFactory(const std::string& t)
{
    IceUtil::Mutex::Lock lock(_m);
    OFTable::iterator i = _oft.find(t);
    if(i != _oft.end())
    {
        if(--i->second.second == 0)
        {
            _oft.erase(i);
        }
    }
}

//
// Add a factory to the object factory table.
//
void
IceInternal::FactoryTable::addTypeId(int compactId, const std::string& typeId)
{
    IceUtil::Mutex::Lock lock(_m);
    assert(!typeId.empty() && compactId >= 0);
    TypeIdTable::iterator i = _typeIdTable.find(compactId);
    if(i == _typeIdTable.end())
    {
        _typeIdTable[compactId] = TypeIdPair(typeId, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the type ID for the given compact ID
//
std::string
IceInternal::FactoryTable::getTypeId(int compactId) const
{
    IceUtil::Mutex::Lock lock(_m);
    TypeIdTable::const_iterator i = _typeIdTable.find(compactId);
    return i != _typeIdTable.end() ? i->second.first : std::string();
}

void
IceInternal::FactoryTable::removeTypeId(int compactId)
{
    IceUtil::Mutex::Lock lock(_m);
    TypeIdTable::iterator i = _typeIdTable.find(compactId);
    if(i != _typeIdTable.end())
    {
        if(--i->second.second == 0)
        {
            _typeIdTable.erase(i);
        }
    }
}


