// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Freeze/Evictor.h>
#include <Test.h>
#include <IceUtil/IceUtil.h>

namespace Test
{

class ServantI;
typedef IceUtil::Handle<ServantI> ServantIPtr;

class RemoteEvictorI;
typedef IceUtil::Handle<RemoteEvictorI> RemoteEvictorIPtr;

class ServantI : public virtual Servant, public IceUtil::AbstractMutexI<IceUtil::Monitor<IceUtil::Mutex> >
{
public:

    ServantI();
    ServantI(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&, Ice::Int);

    void init(const RemoteEvictorIPtr&, const Freeze::EvictorPtr&);

    virtual ::Ice::Int getValue(const Ice::Current& = Ice::Current()) const;

    //
    // Used only if you remove ["amd"] from Test.ice
    // 
    virtual ::Ice::Int slowGetValue(const Ice::Current& = Ice::Current()) const;

    
    virtual void slowGetValue_async(const AMD_Servant_slowGetValuePtr&,
                                    const Ice::Current& = Ice::Current()) const;

    virtual void setValue(::Ice::Int, const Ice::Current& = Ice::Current());

    virtual void setValueAsync_async(const AMD_Servant_setValueAsyncPtr&, Ice::Int,
                                     const Ice::Current& = Ice::Current());

    virtual void releaseAsync(const Ice::Current& = Ice::Current()) const;

    virtual void addFacet(const std::string&, const std::string&, const Ice::Current& = Ice::Current()) const;

    virtual void removeFacet(const std::string&, const Ice::Current& = Ice::Current()) const;


    virtual Ice::Int getTransientValue(const Ice::Current& = Ice::Current()) const;
    virtual void setTransientValue(Ice::Int, const Ice::Current& = Ice::Current());
    virtual void keepInCache(const Ice::Current& = Ice::Current());
    virtual void release(const Ice::Current& = Ice::Current());

    virtual void destroy(const Ice::Current& = Ice::Current());

protected:

    Ice::Int _transientValue;
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

};

class RemoteEvictorI : virtual public RemoteEvictor
{
public:

    RemoteEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, const std::string&);

    virtual void setSize(::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx createServant(const std::string&, ::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx getServant(const std::string&, const Ice::Current&);

    virtual void saveNow(const Ice::Current&);

    virtual void deactivate(const Ice::Current&);

    virtual void destroyAllServants(const std::string&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::string _category;
    Freeze::EvictorPtr _evictor;
    Ice::ObjectAdapterPtr _evictorAdapter;
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
