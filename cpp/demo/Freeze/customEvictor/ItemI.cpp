// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ItemI.h>

using namespace std;
using namespace IceUtil;
using namespace Warehouse;

ItemI::ItemI(CurrentDatabase& currentDb, const ItemInfo& info) :
    _currentDb(currentDb),
    _cachedInfo(info)
{
} 

string 
ItemI::getDescription(const Ice::Current&)
{
    Mutex::Lock lock(_mutex);
    return _cachedInfo.description;
} 

void 
ItemI::setDescription(const string& newDescription, const Ice::Current& current)
{
    Mutex::Lock lock(_mutex);
    _cachedInfo.description = newDescription;
    save(current);
} 

float 
ItemI::getUnitPrice(const Ice::Current&)
{
    Mutex::Lock lock(_mutex);
    return _cachedInfo.unitPrice;
} 

void 
ItemI::setUnitPrice(float newUnitPrice, const Ice::Current& current)
{
    Mutex::Lock lock(_mutex);
    _cachedInfo.unitPrice = newUnitPrice;
    save(current);
} 

int 
ItemI::quantityInStock(const Ice::Current&)
{
    Mutex::Lock lock(_mutex);
    return _cachedInfo.quantityInStock;
} 

void 
ItemI::adjustStock(int value, const Ice::Current& current)
{
    Mutex::Lock lock(_mutex);
    int newQuantity = _cachedInfo.quantityInStock + value;
    if(newQuantity < 0)
    {
        throw OutOfStock();
    }
    _cachedInfo.quantityInStock = newQuantity;
    save(current);
} 

void
ItemI::save(const Ice::Current& current)
{
    //
    // Called with _mutex locked (to protect _cachedInfo)
    //
    _currentDb.get().put(Database::value_type(current.id.name, _cachedInfo));
}
