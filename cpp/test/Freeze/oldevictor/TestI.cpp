// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace IceUtil;


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
Test::ServantI::releaseAsync(const Current& current) const
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
Test::ServantI::getTransientValue(const Current& current) const
{
    Monitor<Mutex>::Lock sync(*this);
    return _transientValue;
}

void
Test::ServantI::setTransientValue(Ice::Int val, const Current& current)
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
    initialize(const ObjectAdapterPtr& adapter, const Identity& ident, const string& facet, const ObjectPtr& servant)
    {
        Test::ServantI* servantI = dynamic_cast<Test::ServantI*>(servant.get());
        servantI->init(_remoteEvictor, _evictor);
    }

private:

    Test::RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
};


Test::RemoteEvictorI::RemoteEvictorI(const ObjectAdapterPtr& adapter, const string& envName,
                                     const string& category) :
    _adapter(adapter),
    _category(category)
{
    CommunicatorPtr communicator = adapter->getCommunicator();
    _evictorAdapter = communicator->createObjectAdapterWithEndpoints(IceUtil::generateUUID(), "default");
 
    Initializer* initializer = new Initializer;
    
    _evictor = Freeze::createBackgroundSaveEvictor(_evictorAdapter, envName, category, initializer);
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
Test::RemoteEvictorI::saveNow(const Current& current)
{
    _evictor->getIterator("", 1);
}

void
Test::RemoteEvictorI::deactivate(const Current& current)
{
    _evictorAdapter->destroy();
    _adapter->remove(_adapter->getCommunicator()->stringToIdentity(_category));
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


Test::RemoteEvictorFactoryI::RemoteEvictorFactoryI(const ObjectAdapterPtr& adapter,
                                                   const std::string& envName) :
    _adapter(adapter),
    _envName(envName)
{
}

::Test::RemoteEvictorPrx
Test::RemoteEvictorFactoryI::createEvictor(const string& name, const Current& current)
{
    RemoteEvictorIPtr remoteEvictor = new RemoteEvictorI(_adapter, _envName, name);  
    return RemoteEvictorPrx::uncheckedCast(_adapter->add(remoteEvictor, 
                                                         _adapter->getCommunicator()->stringToIdentity(name)));
}

void
Test::RemoteEvictorFactoryI::shutdown(const Current&)
{
    _adapter->getCommunicator()->shutdown();
}
