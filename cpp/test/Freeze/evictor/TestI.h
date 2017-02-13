// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
#include <Test.h>

namespace Test
{

class ServantI;
typedef IceUtil::Handle<ServantI> ServantIPtr;

class RemoteEvictorI;
typedef IceUtil::Handle<RemoteEvictorI> RemoteEvictorIPtr;

class AccountI : public Account
{
public:

    virtual int getBalance(const Ice::Current&);
    
    virtual void deposit(int, const Ice::Current&);

    virtual void transfer(int, const Test::AccountPrx&, const Ice::Current&);

    virtual void transfer2_async(const AMD_Account_transfer2Ptr&, int, const Test::AccountPrx&, const Ice::Current&);

    virtual void transfer3_async(const AMD_Account_transfer3Ptr&, int, const Test::AccountPrx&, const Ice::Current&);

    AccountI(int, const Freeze::TransactionalEvictorPtr&);
    AccountI();

    void init(const Freeze::TransactionalEvictorPtr&);

private:

    Freeze::TransactionalEvictorPtr _evictor;
};


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

    virtual Test::AccountPrxSeq getAccounts(const Ice::Current&);
    virtual int getTotalBalance(const Ice::Current&);

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

    RemoteEvictorI(const Ice::CommunicatorPtr&, const std::string&, const std::string&, bool);

    virtual void setSize(::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx createServant(const std::string&, ::Ice::Int, const Ice::Current&);

    virtual ::Test::ServantPrx getServant(const std::string&, const Ice::Current&);

    virtual void saveNow(const Ice::Current&);

    virtual void deactivate(const Ice::Current&);

    virtual void destroyAllServants(const std::string&, const Ice::Current&);

    const std::string& 
    envName() const
    {
        return _envName;
    }

private:

    std::string _envName;
    std::string _category;
    Freeze::EvictorPtr _evictor;
    Ice::ObjectAdapterPtr _evictorAdapter;
};

class RemoteEvictorFactoryI : virtual public RemoteEvictorFactory
{
public:

    RemoteEvictorFactoryI(const std::string&);

    virtual ::Test::RemoteEvictorPrx createEvictor(const ::std::string&, bool, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    const std::string _envName;
};

}

#endif
