// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/FactoryTable.h>
#include <Ice/UserExceptionFactory.h>

#ifdef __APPLE__
#   include <dlfcn.h>
#endif

//
// Add a factory to the exception factory table.
// If the factory is present already, increment its reference count.
//
void
IceInternal::FactoryTable::addExceptionFactory(const std::string& t, const IceInternal::UserExceptionFactoryPtr& f)
{
    IceUtil::Mutex::Lock lock(_m);
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
#ifdef __APPLE__
    if(i == _eft.end())
    {
        lock.release();

        //
        // Try to find the symbol, if found this should trigger the
        // object static constructors to be called.
        //
        std::string symbol = "__F";
        for(std::string::const_iterator p = t.begin(); p != t.end(); ++p)
        {
            symbol += ((*p) == ':') ? '_' : *p;
        }
        symbol += "__initializer";
        dlsym(RTLD_DEFAULT, symbol.c_str());

        lock.acquire(); 

        i = _eft.find(t);
    }
#endif
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
#ifdef __APPLE__
    if(i == _oft.end())
    {
        lock.release();

        //
        // Try to find the symbol, if found this should trigger the
        // object static constructors to be called.
        //
        std::string symbol = "__F";
        for(std::string::const_iterator p = t.begin(); p != t.end(); ++p)
        {
            symbol += ((*p) == ':') ? '_' : *p;
        }
        symbol += "__initializer";
        dlsym(RTLD_DEFAULT, symbol.c_str());

        lock.acquire(); 

        i = _oft.find(t);
    }
#endif
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

