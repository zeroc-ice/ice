// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_MANAGER_I_H
#define ICE_PACK_ADAPTER_MANAGER_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Freeze/DBF.h>
#include <IcePack/AdapterManager.h>
#include <set>

namespace IcePack
{

class AdapterI : public Adapter, public IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    AdapterI(::Ice::Int);
    virtual ~AdapterI();
    
    virtual AdapterDescription getAdapterDescription(const ::Ice::Current&);
    virtual ::Ice::ObjectPrx getDirectProxy(bool, const ::Ice::Current&);
    virtual void setDirectProxy(const ::Ice::ObjectPrx&,const ::Ice::Current&);
    virtual void markAsActive(const ::Ice::Current&);
    virtual void markAsInactive(const ::Ice::Current&);

private:

    ::Ice::Int _waitTime;
    bool _active;
};

class AdapterManagerI : public AdapterManager, public IceUtil::Mutex
{
public:

    AdapterManagerI(const Ice::ObjectAdapterPtr&, const Freeze::DBEnvironmentPtr&);
    
    virtual AdapterPrx create(const AdapterDescription&, const ::Ice::Current&);
    virtual AdapterPrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual void remove(const ::std::string&, const ::Ice::Current&);
    virtual AdapterNames getAll(const ::Ice::Current&);

private:

    ::Ice::ObjectAdapterPtr _adapter;
    ::Freeze::EvictorPtr _evictor;
    ::std::set< ::std::string> _adapterNames;
    ::Ice::Int _waitTime;
};

}

#endif
