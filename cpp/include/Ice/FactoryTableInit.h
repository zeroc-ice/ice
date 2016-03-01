// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FACTORYTABLEINIT_H
#define ICE_FACTORYTABLEINIT_H

#include <Ice/FactoryTable.h>
#include <Ice/DefaultObjectFactory.h>

namespace IceInternal
{

class ICE_API FactoryTableInit
{
public:

    FactoryTableInit();
    ~FactoryTableInit();
};

static FactoryTableInit factoryTableInitializer;    // Dummy variable to force initialization of factoryTable

extern ICE_API FactoryTable* factoryTable;


class ICE_API CompactIdInit
{
public:
    
    CompactIdInit(const char*, int);
        
    ~CompactIdInit();
    
private:
    const int _compactId;
};

template<class E>
class DefaultUserExceptionFactoryInit
{
public:
    
    DefaultUserExceptionFactoryInit(const char* typeId) :
        _typeId(typeId)
    {
        factoryTable->addExceptionFactory(_typeId, new DefaultUserExceptionFactory<E>(_typeId));
    }

    ~DefaultUserExceptionFactoryInit()
    {
        factoryTable->removeExceptionFactory(_typeId);
    }
    
private:
    const ::std::string _typeId;
};

template<class O>
class DefaultObjectFactoryInit
{
public:
    
    DefaultObjectFactoryInit(const char* typeId) :
        _typeId(typeId)
    {
        factoryTable->addObjectFactory(_typeId, new DefaultObjectFactory<O>(_typeId));
    }

    ~DefaultObjectFactoryInit()
    {
        factoryTable->removeObjectFactory(_typeId);
    }
    
private:
    const ::std::string _typeId;
 
};

}

#endif
