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

class RemoteEvictorI;
typedef IceUtil::Handle<RemoteEvictorI> RemoteEvictorIPtr;

class ServantI : virtual public Servant
{
public:

    ServantI();
    ServantI(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&, Ice::Int);

    void init(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&);

    virtual ::Ice::Int getValue(const Ice::Current&) const;

    virtual void setValue(::Ice::Int, const Ice::Current&);

    virtual void destroy(const Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;

    virtual void __marshal(const ::Ice::StreamPtr&) const;

private:

    RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
};

class RemoteEvictorI : virtual public RemoteEvictor
{
public:

    RemoteEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, const Freeze::DBPtr& db,
                   const Freeze::EvictorPtr&);

    virtual void setSize(::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx createServant(::Ice::Int, const Ice::Current&);

    virtual ::Ice::Int getLastSavedValue(const Ice::Current&) const;

    virtual void clearLastSavedValue(const Ice::Current&);

    virtual void deactivate(const Ice::Current&);

    void setLastSavedValue(Ice::Int);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::string _category;
    Freeze::DBPtr _db;
    Freeze::EvictorPtr _evictor;
    Ice::Int _lastSavedValue;
};

class RemoteEvictorFactoryI : virtual public RemoteEvictorFactory
{
public:

    RemoteEvictorFactoryI(const Ice::ObjectAdapterPtr&, const Freeze::DBEnvironmentPtr&);

    virtual ::Test::RemoteEvictorPrx createEvictor(const ::std::string&,
						   ::Test::EvictorPersistenceMode,
						   const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Freeze::DBEnvironmentPtr _dbEnv;
};

}

#endif
