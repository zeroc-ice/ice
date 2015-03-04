// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ITEM_I_H
#define ITEM_I_H

#include <IceUtil/IceUtil.h>
#include <Item.h>
#include <ItemInfo.h>
#include <CurrentDatabase.h>

class ItemI : public Warehouse::Item
{
public:

    ItemI(CurrentDatabase&, const Warehouse::ItemInfo&); 

    virtual std::string getDescription(const Ice::Current&);
    virtual void setDescription(const std::string&,const Ice::Current&);

    virtual float getUnitPrice(const Ice::Current&);
    virtual void setUnitPrice(float, const Ice::Current&);

    virtual int quantityInStock(const Ice::Current&);
    virtual void adjustStock(int, const Ice::Current&);

private:
    
    void save(const Ice::Current&);
    
    CurrentDatabase& _currentDb;
    Warehouse::ItemInfo _cachedInfo;
    IceUtil::Mutex _mutex;
};
typedef IceUtil::Handle<ItemI> ItemIPtr;

#endif
