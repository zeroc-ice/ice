//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/FactoryTableInit.h>
#include <Ice/UserExceptionFactory.h>

#include <atomic>

using namespace std;

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

atomic<int> initCount = 0;   // Initialization count

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
    if(0 == --initCount)
    {
        delete factoryTable;
    }
}

IceInternal::CompactIdInit::CompactIdInit(string_view typeId, int compactId) :
    _compactId(compactId)
{
    assert(_compactId >= 0);
    factoryTable->addTypeId(_compactId, typeId);
}

IceInternal::CompactIdInit::~CompactIdInit()
{
    factoryTable->removeTypeId(_compactId);
}
