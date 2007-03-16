// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/FactoryTable.h>
#include <IceE/UserExceptionFactory.h>

//
// This constructor initializes the single global Ice::factoryTable instance
// from the outside (if it hasn't been initialized yet). The constructor here
// is triggered by a file-static instance of FactoryTable in each
// slice2cpp-generated header file that uses non-local exceptions or non-abstract classes.
// This ensures that Ice::factoryTable is always initialized before it is used.
//
IceInternal::FactoryTable::FactoryTable()
{

    IceInternal::factoryTableWrapper.initialize();
}

//
// Similarly, the destructor calls the finalize() method on the factory table wrapper which,
// once the tables reference count drops to zero, deletes the table.
//
IceInternal::FactoryTable::~FactoryTable()
{
    IceInternal::factoryTableWrapper.finalize();
}
