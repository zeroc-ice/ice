// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/FactoryTableInit.h>
#include <Ice/UserExceptionFactory.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

namespace IceInternal
{

//
// Single global instance of the factory table for non-local
// exceptions and non-abstract classes.
//
ICE_API FactoryTable* factoryTable;

}

namespace
{

int initCount = 0;   // Initialization count
IceUtil::Mutex* initCountMutex = 0;

class Init
{
public:

    Init()
    {
        initCountMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete initCountMutex;
        initCountMutex = 0;
    }
};

Init init;

}

//
// This constructor initializes the single global
// IceInternal::factoryTable instance from the outside (if it hasn't
// been initialized yet). The constructor here is triggered by a
// file-static instance of FactoryTable in each slice2cpp-generated
// header file that uses non-local exceptions or non-abstract classes.
// This ensures that IceInternal::factoryTable is always initialized
// before it is used.
//
IceInternal::FactoryTableInit::FactoryTableInit()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(initCountMutex);
    if(0 == initCount++)
    {
        factoryTable = new FactoryTable;
    }
}

//
// The destructor decrements the reference count and, once the
// count drops to zero, deletes the table.
//
IceInternal::FactoryTableInit::~FactoryTableInit()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(initCountMutex);
    if(0 == --initCount)
    {
        delete factoryTable;
    }
}


IceInternal::CompactIdInit::CompactIdInit(const char* typeId, int compactId) :
    _compactId(compactId)
{
    assert(_compactId >= 0);
    factoryTable->addTypeId(_compactId, typeId);
}

IceInternal::CompactIdInit::~CompactIdInit()
{
    factoryTable->removeTypeId(_compactId);  
}

