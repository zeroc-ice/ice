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
#include <Test.h>
#include <IceUtil/AbstractMutex.h>

namespace Test
{

class ServantI;
typedef IceUtil::Handle<ServantI> ServantIPtr;

class RemoteEvictorI;
typedef IceUtil::Handle<RemoteEvictorI> RemoteEvictorIPtr;

class ServantI : virtual public Servant, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    ServantI();
    ServantI(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&, Ice::Int);

    void init(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&);

    virtual ::Ice::Int getValue(const Ice::Current& = Ice::Current()) const;

    virtual void setValue(::Ice::Int, const Ice::Current& = Ice::Current());

    virtual void setValueAsync_async(const AMD_Servant_setValueAsyncPtr&, Ice::Int,
                                     const Ice::Current& = Ice::Current());

    virtual void releaseAsync(const Ice::Current& = Ice::Current()) const;

    virtual void addFacet(const std::string&, const std::string&, const Ice::Current& = Ice::Current()) const;

    virtual void removeFacet(const std::string&, const Ice::Current& = Ice::Current()) const;

    virtual void removeAllFacets(const Ice::Current& = Ice::Current()) const;

    virtual void destroy(const Ice::Current& = Ice::Current());

    virtual void __write(::IceInternal::BasicStream*, bool) const;

protected:

    RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
    AMD_Servant_setValueAsyncPtr _setValueAsyncCB;
    Ice::Int _setValueAsyncValue;
};


class FacetI : public virtual Facet, public ServantI
{
public:

    FacetI();

    FacetI(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&, Ice::Int, const std::string&);

    virtual std::string getData(const Ice::Current& = Ice::Current()) const;

    virtual void setData(const std::string&, const Ice::Current& = Ice::Current());

    virtual void __write(::IceInternal::BasicStream*, bool) const;
};

class RemoteEvictorI : virtual public RemoteEvictor
{
public:

    RemoteEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, const Freeze::EvictorPtr&);

    virtual void setSize(::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx createServant(::Ice::Int, ::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx getServant(::Ice::Int, const Ice::Current&);

    virtual ::Ice::Int getLastSavedValue(const Ice::Current&) const;

    virtual void clearLastSavedValue(const Ice::Current&);

    virtual void saveNow(const Ice::Current&);

    virtual void deactivate(const Ice::Current&);

    virtual void destroyAllServants(const Ice::Current&);

    void setLastSavedValue(Ice::Int);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::string _category;
    Freeze::EvictorPtr _evictor;
    Ice::ObjectAdapterPtr _evictorAdapter;
    Ice::Int _lastSavedValue;
};

class RemoteEvictorFactoryI : virtual public RemoteEvictorFactory
{
public:

    RemoteEvictorFactoryI(const Ice::ObjectAdapterPtr&, const std::string&);

    virtual ::Test::RemoteEvictorPrx createEvictor(const ::std::string&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    const std::string _envName;
};

}

#endif
