// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Freeze/EvictorF.h>
#include <Freeze/DBF.h>
#include <Test.h>

namespace Test
{

class ServantI;
typedef IceUtil::Handle<ServantI> ServantIPtr;

class RemoteEvictorI;
typedef IceUtil::Handle<RemoteEvictorI> RemoteEvictorIPtr;

class StrategyI;
typedef IceUtil::Handle<StrategyI> StrategyIPtr;

class ServantI : virtual public Servant
{
public:

    ServantI();
    ServantI(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&, Ice::Int);

    void init(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&);

    virtual ::Ice::Int getValue(const Ice::Current& = Ice::Current()) const;

    virtual void setValue(::Ice::Int, const Ice::Current& = Ice::Current());

    virtual void setValueAsync_async(const AMD_Servant_setValueAsyncPtr&, Ice::Int,
                                     const Ice::Current& = Ice::Current());

    virtual void saveValue(Ice::Int, const Ice::Current&);

    virtual void releaseAsync(const Ice::Current& = Ice::Current()) const;

    virtual void destroy(const Ice::Current& = Ice::Current());

    virtual void __write(::IceInternal::BasicStream*) const;

    virtual void __marshal(const ::Ice::StreamPtr&) const;

private:

    RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
    AMD_Servant_setValueAsyncPtr _setValueAsyncCB;
    Ice::Int _setValueAsyncValue;
};

class RemoteEvictorI : virtual public RemoteEvictor
{
public:

    RemoteEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, const Freeze::DBPtr& db,
                   const StrategyIPtr& strategy, const Freeze::EvictorPtr&);

    virtual void setSize(::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx createServant(::Ice::Int, ::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx getServant(::Ice::Int, const Ice::Current&);

    virtual ::Ice::Int getLastSavedValue(const Ice::Current&) const;

    virtual void clearLastSavedValue(const Ice::Current&);

    virtual ::Ice::Int getLastEvictedValue(const Ice::Current&) const;

    virtual void clearLastEvictedValue(const Ice::Current&);

    virtual void deactivate(const Ice::Current&);

    void setLastSavedValue(Ice::Int);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::string _category;
    Freeze::DBPtr _db;
    StrategyIPtr _strategy;
    Freeze::EvictorPtr _evictor;
    Ice::ObjectAdapterPtr _evictorAdapter;
    Ice::Int _lastSavedValue;
};

class RemoteEvictorFactoryI : virtual public RemoteEvictorFactory
{
public:

    RemoteEvictorFactoryI(const Ice::ObjectAdapterPtr&, const Freeze::DBEnvironmentPtr&);

    virtual ::Test::RemoteEvictorPrx createEvictor(const ::std::string&,
						   ::Test::Strategy,
						   const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Freeze::DBEnvironmentPtr _dbEnv;
};

class StrategyI : virtual public Freeze::PersistenceStrategy
{
public:

    StrategyI(const Freeze::PersistenceStrategyPtr&);

    virtual Ice::LocalObjectPtr activatedObject(const Ice::Identity&,
                                                const Ice::ObjectPtr&);

    virtual void destroyedObject(const Ice::Identity&, const Ice::LocalObjectPtr&);

    virtual void evictedObject(const Freeze::ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               const Ice::LocalObjectPtr&);

    virtual void savedObject(const Freeze::ObjectStorePtr&,
			     const Ice::Identity&,
			     const Ice::ObjectPtr&,
			     const Ice::LocalObjectPtr&,
			     Ice::Int);

    virtual void preOperation(const Freeze::ObjectStorePtr&,
                              const Ice::Identity&,
                              const Ice::ObjectPtr&,
                              bool,
                              const Ice::LocalObjectPtr&);

    virtual void postOperation(const Freeze::ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               bool,
                               const Ice::LocalObjectPtr&);

    virtual void destroy();

    Ice::Int getLastEvictedValue();

    void clearLastEvictedValue();

private:

    Freeze::PersistenceStrategyPtr _delegate;
    Ice::Int _lastEvictedValue;
};

}

#endif
