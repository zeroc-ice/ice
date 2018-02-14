// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace IceUtil;


int
Test::AccountI::getBalance(const Current&)
{
    return balance;
}
    
void 
Test::AccountI::deposit(int amount, const Current&)
{ 
    //
    // No need to synchronize since everything occurs within its own transaction
    //
    int newBalance = balance + amount;
    if(newBalance < 0)
    {
        throw Test::InsufficientFundsException();
    }
    balance = newBalance;
}

void 
Test::AccountI::transfer(int amount, const Test::AccountPrx& toAccount, const Current& current)
{
    test(_evictor->getCurrentTransaction() != 0);

    toAccount->deposit(amount); // collocated call
    deposit(-amount, current); // direct call
}

void 
Test::AccountI::transfer2_async(const AMD_Account_transfer2Ptr& cb, int amount, const Test::AccountPrx& toAccount, const Current& current)
{
    //
    // Here the dispatch thread does everything
    //
    test(_evictor->getCurrentTransaction() != 0);

    try
    {
        toAccount->deposit(amount); // collocated call
        deposit(-amount, current); // direct call
    }
    catch(const InsufficientFundsException& ex)
    {
        cb->ice_exception(ex);
        return;
    }
    
    cb->ice_response();
}

class ResponseThread : public IceUtil::Thread, private IceUtil::Monitor<IceUtil::Mutex>
{
public:
        
    ResponseThread(const Test::AMD_Account_transfer3Ptr& cb) :
        _cb(cb),
        _response(false),
        _cancelled(false)
    {
    }

    void response()
    {
        Lock sync(*this);
        _response = true;
        notify();
    }

    void exception(const Ice::UserException& e)
    {
        Lock sync(*this);
        _exception.reset(e.ice_clone());
        notify();
    }

    void cancel()
    {
        Lock sync(*this);
        _cancelled = true;
        notify();
    }


    virtual void run()
    {
        Lock sync(*this);

        bool timedOut = false;

        while(!timedOut && _response == false && _cancelled == false && _exception.get() == 0)
        {
            timedOut = !timedWait(IceUtil::Time::seconds(1));
        }

        if(_cancelled)
        {
            return;
        }

        if(_response)
        {
            _cb->ice_response();
        }
        else if(_exception.get() != 0)
        {
            _cb->ice_exception(*_exception.get());
        }
        else
        {
            _cb->ice_exception(Ice::TimeoutException(__FILE__, __LINE__));
        }
    }
        
private:
    Test::AMD_Account_transfer3Ptr _cb;
    bool _response;
    bool _cancelled;
    IceUtil::UniquePtr<Ice::UserException> _exception;
};
typedef IceUtil::Handle<ResponseThread> ResponseThreadPtr;





void 
Test::AccountI::transfer3_async(const AMD_Account_transfer3Ptr& cb, int amount, const Test::AccountPrx& toAccount, const Current& current)
{
    //
    // Here the dispatch thread does the actual work, but a separate thread sends the response
    //

    ResponseThreadPtr thread = new ResponseThread(cb);
    IceUtil::ThreadControl tc = thread->start(33000);
    
    test(_evictor->getCurrentTransaction() != 0);

    try
    {
        toAccount->deposit(amount); // collocated call
        deposit(-amount, current); // direct call
    }
    catch(const Ice::UserException& e)
    {
        tc.detach();

        //
        // Need to rollback here -- "rollback on user exception" does not work
        // when the dispatch commits before it gets any response!
        //
        _evictor->getCurrentTransaction()->rollback();
        
        thread->exception(e);

        return;
    }
    catch(...)
    {
        thread->cancel();
        tc.join();
        throw;
    }

    tc.detach();
    thread->response();
}


Test::AccountI::AccountI(int initialBalance, const Freeze::TransactionalEvictorPtr& evictor) :
    Account(initialBalance),
    _evictor(evictor)
{
}


Test::AccountI::AccountI() :
    Account(0)
{
}

void 
Test::AccountI::init(const Freeze::TransactionalEvictorPtr& evictor)
{
    test(_evictor == 0);
    _evictor = evictor;
}


class DelayedResponse : public Thread
{
public:

    DelayedResponse(const Test::AMD_Servant_slowGetValuePtr& cb, int val) :
        _cb(cb),
        _val(val)
    {
    }
    
    virtual void
    run()
    {
        ThreadControl::sleep(Time::milliSeconds(500));
        _cb->ice_response(_val);
    }

private:
    Test::AMD_Servant_slowGetValuePtr _cb;
    int _val;
};


Test::ServantI::ServantI() :
    _transientValue(-1)
{
}

Test::ServantI::ServantI(const RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor, Ice::Int val) :
    _transientValue(-1),
    _remoteEvictor(remoteEvictor),
    _evictor(evictor)
{
    this->value = val;
}

void
Test::ServantI::init(const RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor)
{
    _remoteEvictor = remoteEvictor;
    _evictor = evictor;
}

Int
Test::ServantI::getValue(const Current&) const
{
    Monitor<Mutex>::Lock sync(*this);
    return value;
}

Int
Test::ServantI::slowGetValue(const Current&) const
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
    Monitor<Mutex>::Lock sync(*this);
    return value;
}

void
Test::ServantI::slowGetValue_async(const AMD_Servant_slowGetValuePtr& cb,
                                   const Current&) const
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
    Monitor<Mutex>::Lock sync(*this);
    (new DelayedResponse(cb, value))->start().detach();
}


void
Test::ServantI::setValue(Int val, const Current&)
{
    Monitor<Mutex>::Lock sync(*this);
    value = val;
}


void
Test::ServantI::setValueAsync_async(const AMD_Servant_setValueAsyncPtr& __cb, Int value, const Current&)
{
    Monitor<Mutex>::Lock sync(*this);
    _setValueAsyncCB = __cb;
    _setValueAsyncValue = value;
    notify();
}

void
Test::ServantI::releaseAsync(const Current&) const
{
    Monitor<Mutex>::Lock sync(*this);
    //
    // Wait until the previous _async has been dispatched
    //
    while(_setValueAsyncCB == 0)
    {
        wait();
    }
    const_cast<Int&>(value) = _setValueAsyncValue;
    _setValueAsyncCB->ice_response();
    const_cast<AMD_Servant_setValueAsyncPtr&>(_setValueAsyncCB) = 0;
}

void
Test::ServantI::addFacet(const string& name, const string& data, const Current& current) const
{
    FacetPtr facet = new FacetI(_remoteEvictor, _evictor, value, data);

    try
    {
        _evictor->addFacet(facet, current.id, name);
    }
    catch(const Ice::AlreadyRegisteredException&)
    {
        throw Test::AlreadyRegisteredException();
    }
}

void
Test::ServantI::removeFacet(const string& name, const Current& current) const
{
    try
    {
        _evictor->removeFacet(current.id, name);
    }
    catch(const Ice::NotRegisteredException&)
    {
        throw Test::NotRegisteredException();
    }
}


Ice::Int
Test::ServantI::getTransientValue(const Current&) const
{
    Monitor<Mutex>::Lock sync(*this);
    return _transientValue;
}

void
Test::ServantI::setTransientValue(Ice::Int val, const Current&)
{
    Monitor<Mutex>::Lock sync(*this);
    _transientValue = val;
}

void
Test::ServantI::keepInCache(const Current& current)
{
    Freeze::BackgroundSaveEvictorPtr::dynamicCast(_evictor)->keep(current.id);
}

void
Test::ServantI::release(const Current& current)
{
    try
    {
        Freeze::BackgroundSaveEvictorPtr::dynamicCast(_evictor)->release(current.id);
    }
    catch(const Ice::NotRegisteredException&)
    {
        throw NotRegisteredException();
    }
}


Test::AccountPrxSeq
Test::ServantI::getAccounts(const Current& current)
{
    Freeze::TransactionalEvictorPtr te = Freeze::TransactionalEvictorPtr::dynamicCast(_evictor);

    if(te->getCurrentTransaction() != 0)
    {
        if(accounts.empty())
        {
            for(int i = 0; i < 10; ++i)
            {
                Ice::Identity ident;
                ident.name = current.id.name + "-account#" + char('0' + i);
                ident.category = current.id.category;
                accounts.push_back(ident);
                _evictor->add(new AccountI(1000, te), ident);
            }
        }
        else
        {
            te->getCurrentTransaction()->rollback(); // not need to re-write this servant
        }
    }

    Test::AccountPrxSeq result;
    for(size_t i = 0; i < accounts.size(); ++i)
    {
        result.push_back(Test::AccountPrx::uncheckedCast(current.adapter->createProxy(accounts[i])));
    }
    return result;
}

int
Test::ServantI::getTotalBalance(const Current& current)
{
    Test::AccountPrxSeq proxies = getAccounts(current);

    //
    // Need to start a transaction to ensure a consistent result
    //
    Freeze::TransactionalEvictorPtr te = Freeze::TransactionalEvictorPtr::dynamicCast(_evictor);

    for(;;)
    {
        test(te->getCurrentTransaction() == 0);
        Freeze::ConnectionPtr con = Freeze::createConnection(current.adapter->getCommunicator(), _remoteEvictor->envName());
        te->setCurrentTransaction(con->beginTransaction());
        int total = 0;
        try
        {
            for(size_t i = 0; i < proxies.size(); ++i)
            {
                total += proxies[i]->getBalance();
            }
            te->getCurrentTransaction()->rollback();
            te->setCurrentTransaction(0);
            return total;
        }
        catch(const Freeze::TransactionalEvictorDeadlockException&)
        {
            te->getCurrentTransaction()->rollback();
            te->setCurrentTransaction(0);
            // retry
        }
        catch(...)
        {
            te->getCurrentTransaction()->rollback();
            te->setCurrentTransaction(0);
            throw;
        }
    }
    return -1;
}


void
Test::ServantI::destroy(const Current& current)
{
    try
    {
        _evictor->remove(current.id);
    }
    catch(const Ice::NotRegisteredException&)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
}


Test::FacetI::FacetI()
{
}

Test::FacetI::FacetI(const RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor, Ice::Int val,
                     const string& d) :
    ServantI(remoteEvictor, evictor, val)
{
    data = d;
}

string
Test::FacetI::getData(const Current&) const
{
    Monitor<Mutex>::Lock sync(*this);
    return data;
}

void
Test::FacetI::setData(const string& d, const Current&)
{
    Monitor<Mutex>::Lock sync(*this);
    data = d;
}


class Initializer : public Freeze::ServantInitializer
{
public:

    void init(const Test::RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor)
    {
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
    }
    
    virtual void
    initialize(const ObjectAdapterPtr&, const Identity&, const string&, const ObjectPtr& servant)
    {
        Test::ServantI* servantI = dynamic_cast<Test::ServantI*>(servant.get());
        if(servantI != 0)
        {
            servantI->init(_remoteEvictor, _evictor);
        }
        else
        {
            Test::AccountI* account = dynamic_cast<Test::AccountI*>(servant.get());
            test(account != 0);
            account->init(Freeze::TransactionalEvictorPtr::dynamicCast(_evictor));
        }
    }

private:

    Test::RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
};


Test::RemoteEvictorI::RemoteEvictorI(const CommunicatorPtr& communicator, const string& envName,
                                     const string& category, bool transactional) :
    _envName(envName),
    _category(category)
{
    _evictorAdapter = communicator->createObjectAdapterWithEndpoints(IceUtil::generateUUID(), "default");
 
    Initializer* initializer = new Initializer;
    
    if(transactional)
    {
        _evictor = Freeze::createTransactionalEvictor(_evictorAdapter, envName, category, Freeze::FacetTypeMap(), initializer);
    }
    else
    {
        _evictor = Freeze::createBackgroundSaveEvictor(_evictorAdapter, envName, category, initializer);
    }

    //
    // Check that we can get an iterator on a non-existing facet
    //
    Freeze::EvictorIteratorPtr p = _evictor->getIterator("foo", 1);
    test(p->hasNext() == false);


    initializer->init(this, _evictor);

    _evictorAdapter->addServantLocator(_evictor, category);
    _evictorAdapter->activate();
}


void
Test::RemoteEvictorI::setSize(Int size, const Current&)
{
    _evictor->setSize(size);
}

Test::ServantPrx
Test::RemoteEvictorI::createServant(const string& id, Int value, const Current&)
{
    Identity ident;
    ident.category = _category;
    ident.name = id;
    ServantPtr servant = new ServantI(this, _evictor, value);
    try
    {
        return ServantPrx::uncheckedCast(_evictor->add(servant, ident));
    }
    catch(const Ice::AlreadyRegisteredException&)
    {
        throw Test::AlreadyRegisteredException();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        throw EvictorDeactivatedException();
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
        throw EvictorDeactivatedException();
    }

}

Test::ServantPrx
Test::RemoteEvictorI::getServant(const string& id, const Current&)
{
    Identity ident;
    ident.category = _category;
    ident.name = id;
    return ServantPrx::uncheckedCast(_evictorAdapter->createProxy(ident));
}


void
Test::RemoteEvictorI::saveNow(const Current&)
{
    _evictor->getIterator("", 1);
}

void
Test::RemoteEvictorI::deactivate(const Current& current)
{
    _evictorAdapter->destroy();
    current.adapter->remove(current.adapter->getCommunicator()->stringToIdentity(_category));
}


void
Test::RemoteEvictorI::destroyAllServants(const string& facetName, const Current&)
{
    //
    // Don't use such a small value in real applications!
    //
    Ice::Int batchSize = 1;

    Freeze::EvictorIteratorPtr p = _evictor->getIterator(facetName, batchSize);
    while(p->hasNext())
    {
        _evictor->remove(p->next());
    }
}

Test::RemoteEvictorFactoryI::RemoteEvictorFactoryI(const std::string& envName) :
    _envName(envName)
{
}

::Test::RemoteEvictorPrx
Test::RemoteEvictorFactoryI::createEvictor(const string& name, bool transactional, const Current& current)
{
    RemoteEvictorIPtr remoteEvictor = 
        new RemoteEvictorI(current.adapter->getCommunicator(), _envName, name, transactional);  
    return RemoteEvictorPrx::uncheckedCast(
        current.adapter->add(remoteEvictor, current.adapter->getCommunicator()->stringToIdentity(name)));
}

void
Test::RemoteEvictorFactoryI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
