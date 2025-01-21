// Copyright (c) ZeroC, Inc.

#include "Ice/FactoryTable.h"
#include "Ice/ValueFactory.h"

#include <atomic>

using namespace std;

namespace IceInternal
{
    // Single global instance of the factory table.
    ICE_API FactoryTable* factoryTable;
}

namespace
{
    atomic<int> initCount = 0; // Initialization count
}

//
// Add a factory to the exception factory table.
// If the factory is present already, increment its reference count.
//
void
IceInternal::FactoryTable::addExceptionFactory(string_view t, Ice::UserExceptionFactory f)
{
    lock_guard lock(_mutex);
    assert(f);
    auto i = _eft.find(t);
    if (i == _eft.end())
    {
        _eft[string{t}] = EFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the exception factory for a given type ID
//
Ice::UserExceptionFactory
IceInternal::FactoryTable::getExceptionFactory(string_view t) const
{
    lock_guard lock(_mutex);
    auto i = _eft.find(t);
    return i != _eft.end() ? i->second.first : Ice::UserExceptionFactory();
}

//
// Remove a factory from the exception factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count; if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeExceptionFactory(string_view t)
{
    lock_guard lock(_mutex);
    auto i = _eft.find(t);
    if (i != _eft.end())
    {
        if (--i->second.second == 0)
        {
            _eft.erase(i);
        }
    }
}

//
// Add a factory to the value factory table.
//
void
IceInternal::FactoryTable::addValueFactory(string_view t, Ice::ValueFactory f)
{
    lock_guard lock(_mutex);
    assert(f);
    auto i = _vft.find(t);
    if (i == _vft.end())
    {
        _vft[string{t}] = VFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the value factory for a given type ID
//
Ice::ValueFactory
IceInternal::FactoryTable::getValueFactory(string_view t) const
{
    lock_guard lock(_mutex);
    auto i = _vft.find(t);
    return i != _vft.end() ? i->second.first : nullptr;
}

//
// Remove a factory from the value factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeValueFactory(string_view t)
{
    lock_guard lock(_mutex);
    auto i = _vft.find(t);
    if (i != _vft.end())
    {
        if (--i->second.second == 0)
        {
            _vft.erase(i);
        }
    }
}

//
// Add a factory to the value factory table.
//
void
IceInternal::FactoryTable::addTypeId(int compactId, string_view typeId)
{
    lock_guard lock(_mutex);
    assert(!typeId.empty() && compactId >= 0);
    auto i = _typeIdTable.find(compactId);
    if (i == _typeIdTable.end())
    {
        _typeIdTable[compactId] = TypeIdPair(string{typeId}, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the type ID for the given compact ID
//
string
IceInternal::FactoryTable::getTypeId(int compactId) const
{
    lock_guard lock(_mutex);
    auto i = _typeIdTable.find(compactId);
    return i != _typeIdTable.end() ? i->second.first : string{};
}

void
IceInternal::FactoryTable::removeTypeId(int compactId)
{
    lock_guard lock(_mutex);
    auto i = _typeIdTable.find(compactId);
    if (i != _typeIdTable.end())
    {
        if (--i->second.second == 0)
        {
            _typeIdTable.erase(i);
        }
    }
}

// This constructor initializes the single global
// IceInternal::factoryTable instance from the outside (if it hasn't
// been initialized yet). The constructor here is triggered by a
// file-static instance of FactoryTable in each slice2cpp-generated
// header file that uses non-local exceptions or non-abstract classes.
// This ensures that IceInternal::factoryTable is always initialized
// before it is used.
IceInternal::FactoryTableInit::FactoryTableInit() noexcept
{
    if (0 == initCount++)
    {
        factoryTable = new FactoryTable;
    }
}

// The destructor decrements the reference count and, once the
// count drops to zero, deletes the table.
IceInternal::FactoryTableInit::~FactoryTableInit()
{
    if (0 == --initCount)
    {
        delete factoryTable;
    }
}

IceInternal::CompactIdInit::CompactIdInit(const char* typeId, int compactId) noexcept : _compactId(compactId)
{
    assert(_compactId >= 0);
    factoryTable->addTypeId(_compactId, typeId);
}

IceInternal::CompactIdInit::~CompactIdInit() { factoryTable->removeTypeId(_compactId); }
