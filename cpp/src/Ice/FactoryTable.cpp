// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/FactoryTable.h>
#include <Ice/UserExceptionFactory.h>

namespace IceInternal
{

//
// Single global instance of the factory table for non-local
// exceptions and non-abstract classes.
//
ICE_DECLSPEC_EXPORT FactoryTableDef* factoryTable;

}

namespace
{

static int initCount = 0;   // Initialization count
IceUtil::StaticMutex initCountMutex = ICE_STATIC_MUTEX_INITIALIZER;


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
IceInternal::FactoryTable::FactoryTable()
{
    IceUtil::StaticMutex::Lock lock(initCountMutex);
    if(0 == initCount++)
    {
        factoryTable = new FactoryTableDef;
    }
}

//
// The destructor decrements the reference count and, once the
// count drops to zero, deletes the table.
//
IceInternal::FactoryTable::~FactoryTable()
{
    IceUtil::StaticMutex::Lock lock(initCountMutex);
    if(0 == --initCount)
    {
        delete factoryTable;
    }
}
