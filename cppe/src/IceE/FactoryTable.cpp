// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/FactoryTable.h>
#include <IceE/UserExceptionFactory.h>

//
// This constructor initializes the single global IceE::factoryTable instance
// from the outside (if it hasn't been initialized yet). The constructor here
// is triggered by a file-static instance of FactoryTable in each
// slice2cpp-generated header file that uses non-local exceptions or non-abstract classes.
// This ensures that IceE::factoryTable is always initialized before it is used.
//
IceE::FactoryTable::FactoryTable()
{

    IceE::factoryTableWrapper.initialize();
}

//
// Similarly, the destructor calls the finalize() method on the factory table wrapper which,
// once the tables reference count drops to zero, deletes the table.
//
IceE::FactoryTable::~FactoryTable()
{
    IceE::factoryTableWrapper.finalize();
}
