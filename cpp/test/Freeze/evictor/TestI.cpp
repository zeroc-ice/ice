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

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

Test::ServantI::ServantI()
{
}

Test::ServantI::ServantI(const RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor, Ice::Int val) :
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
    Lock sync(*this);
    return value;
}

void
Test::ServantI::setValue(Int val, const Current&)
{
    Lock sync(*this);
    value = val;
}


void
Test::ServantI::setValueAsync_async(const AMD_Servant_setValueAsyncPtr& __cb, Int value, const Current&)
{
    Lock sync(*this);
    _setValueAsyncCB = __cb;
    _setValueAsyncValue = value;
}

void
Test::ServantI::releaseAsync(const Current& current) const
{
    if(_setValueAsyncCB)
    {
	Lock sync(*this);
        const_cast<Int&>(value) = _setValueAsyncValue;
        _setValueAsyncCB->ice_response();
        const_cast<AMD_Servant_setValueAsyncPtr&>(_setValueAsyncCB) = 0;
    }
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

void
Test::ServantI::destroy(const Current& current)
{
    _evictor->remove(current.id);
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
    Lock sync(*this);
    return data;
}

void
Test::FacetI::setData(const string& d, const Current&)
{
    Lock sync(*this);
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
    _evictorAdapter = communicator->createObjectAdapterWithEndpoints(IceUtil::generateUUID(), "default -p 9999");
 
    Initializer* initializer = new Initializer;
    
    _evictor = Freeze::createEvictor(_evictorAdapter, envName, category, initializer);
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
Test::RemoteEvictorI::createServant(Int id, Int value, const Current&)
{
    Identity ident;
    ident.category = _category;
    ostringstream ostr;
    ostr << id;
    ident.name = ostr.str();
    ServantPtr servant = new ServantI(this, _evictor, value);
    _evictor->add(servant, ident);
    return ServantPrx::uncheckedCast(_evictorAdapter->createProxy(ident));
}

Test::ServantPrx
Test::RemoteEvictorI::getServant(Int id, const Current&)
{
    Identity ident;
    ident.category = _category;
    ostringstream ostr;
    ostr << id;
    ident.name = ostr.str();
    return ServantPrx::uncheckedCast(_evictorAdapter->createProxy(ident));
}


void
Test::RemoteEvictorI::deactivate(const Current& current)
{
    _evictorAdapter->deactivate();
    _evictorAdapter->waitForDeactivate();
    _adapter->remove(stringToIdentity(_category));
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
    return RemoteEvictorPrx::uncheckedCast(_adapter->add(remoteEvictor, stringToIdentity(name)));
}

void
Test::RemoteEvictorFactoryI::shutdown(const Current&)
{
    _adapter->getCommunicator()->shutdown();
}
