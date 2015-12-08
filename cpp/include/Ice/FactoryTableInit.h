// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FACTORYTABLEINIT_H
#define ICE_FACTORYTABLEINIT_H

#include <Ice/FactoryTable.h>
#include <Ice/DefaultValueFactory.h>

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
class DefaultValueFactoryInit
{
public:

    DefaultValueFactoryInit(const char* typeId) :
        _typeId(typeId)
    {
#ifdef ICE_CPP11_MAPPING
        factoryTable->addValueFactory(_typeId,
                                    [](const std::string&)
                                    {
                                        return ::std::make_shared<O>();
                                    });
#else
        factoryTable->addValueFactory(_typeId, new DefaultValueFactory<O>(_typeId));
#endif
    }

    ~DefaultValueFactoryInit()
    {
        factoryTable->removeValueFactory(_typeId);
    }

private:
    const ::std::string _typeId;

};

}

#endif
