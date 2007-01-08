// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/FactoryTable.h>
#include <Ice/UserExceptionFactory.h>

//
// This constructor initializes the single global IceInternal::factoryTable instance
// from the outside (if it hasn't been initialized yet). The constructor here
// is triggered by a file-static instance of FactoryTable in each
// slice2cpp-generated header file that uses non-local exceptions or non-abstract classes.
// This ensures that IceInternal::factoryTable is always initialized before it is used.
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
