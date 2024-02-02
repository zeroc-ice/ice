//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    DefaultUserExceptionFactoryInit(const char* tId) : typeId(tId)
    {
        factoryTable->addExceptionFactory(typeId, defaultUserExceptionFactory<E>);
    }

    ~DefaultUserExceptionFactoryInit()
    {
        factoryTable->removeExceptionFactory(typeId);
    }

    const ::std::string typeId;
};

template<class O>
class DefaultValueFactoryInit
{
public:

    DefaultValueFactoryInit(const char* tId) : typeId(tId)
    {
        factoryTable->addValueFactory(typeId, defaultValueFactory<O>);
    }

    ~DefaultValueFactoryInit()
    {
        factoryTable->removeValueFactory(typeId);
    }

    const ::std::string typeId;
};

}

#endif
