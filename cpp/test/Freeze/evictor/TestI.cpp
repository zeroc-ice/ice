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

    vector<string> facets = ice_facets();
    for(size_t i = 0; i < facets.size(); i++)
    {
	dynamic_cast<Test::ServantI*>(ice_findFacet(facets[i]).get())->init(remoteEvictor, evictor);
    }
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
    FacetPath facetPath(current.facet);
    facetPath.push_back(name);

    FacetPtr facet = new FacetI(_remoteEvictor, _evictor, value, data);

    try
    {
	_evictor->addFacet(current.id, facetPath, facet);
    }
    catch(const Ice::AlreadyRegisteredException&)
    {
	throw Test::AlreadyRegisteredException();
    }
}

void
Test::ServantI::removeFacet(const string& name, const Current& current) const
{
    FacetPath facetPath(current.facet);
    facetPath.push_back(name);
    try
    {
	_evictor->removeFacet(current.id, facetPath);
    }
     catch(const Ice::NotRegisteredException&)
    {
	throw Test::NotRegisteredException();
    }
   
}

void
Test::ServantI::removeAllFacets(const Current& current) const
{
    _evictor->removeAllFacets(current.id);
}

void
Test::ServantI::destroy(const Current& current)
{
    _evictor->destroyObject(current.id);
}

void
Test::ServantI::__write(IceInternal::BasicStream* os, bool marshalFacets) const
{
    assert(_remoteEvictor);
    _remoteEvictor->setLastSavedValue(value);
    Servant::__write(os, marshalFacets);
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

void 
Test::FacetI::__write(::IceInternal::BasicStream* os, bool marshalFacets) const
{
    assert(_remoteEvictor);
    _remoteEvictor->setLastSavedValue(value);
    Facet::__write(os, marshalFacets);
}

Test::RemoteEvictorI::RemoteEvictorI(const ObjectAdapterPtr& adapter, const string& category,
				     const Freeze::EvictorPtr& evictor) :
    _adapter(adapter),
    _category(category),
    _evictor(evictor),
    _lastSavedValue(-1)
{
    CommunicatorPtr communicator = adapter->getCommunicator();
    _evictorAdapter = communicator->createObjectAdapterWithEndpoints(IceUtil::generateUUID(), "default");
    _evictorAdapter->addServantLocator(evictor, category);
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
    _evictor->createObject(ident, servant);
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

Int
Test::RemoteEvictorI::getLastSavedValue(const Current&) const
{
    Int result = _lastSavedValue;
    return result;
}

void
Test::RemoteEvictorI::clearLastSavedValue(const Current&)
{
    _lastSavedValue = -1;
}


void
Test::RemoteEvictorI::saveNow(const Current&)
{
    _evictor->saveNow();
}

void
Test::RemoteEvictorI::deactivate(const Current& current)
{
    _evictorAdapter->deactivate();
    _evictorAdapter->waitForDeactivate();
    _adapter->remove(stringToIdentity(_category));
}

void
Test::RemoteEvictorI::destroyAllServants(const Current&)
{
    //
    // Don't use such a small value in real applications!
    //
    Ice::Int batchSize = 1;

    Freeze::EvictorIteratorPtr p = _evictor->getIterator(batchSize, true);
    while(p->hasNext())
    {
	_evictor->destroyObject(p->next());
	_evictor->saveNow();
    }
}

void
Test::RemoteEvictorI::setLastSavedValue(Int value)
{
    _lastSavedValue = value;
}

class Initializer : public Freeze::ServantInitializer
{
public:

    Initializer(const Test::RemoteEvictorIPtr& remoteEvictor, const Freeze::EvictorPtr& evictor) :
        _remoteEvictor(remoteEvictor),
        _evictor(evictor)
    {
    }

    virtual void
    initialize(const ObjectAdapterPtr& adapter, const Identity& ident, const ObjectPtr& servant)
    {
        Test::ServantI* servantI = dynamic_cast<Test::ServantI*>(servant.get());
        servantI->init(_remoteEvictor, _evictor);
    }

private:

    Test::RemoteEvictorIPtr _remoteEvictor;
    Freeze::EvictorPtr _evictor;
};

Test::RemoteEvictorFactoryI::RemoteEvictorFactoryI(const ObjectAdapterPtr& adapter,
                                                   const std::string& envName) :
    _adapter(adapter),
    _envName(envName)
{
}

::Test::RemoteEvictorPrx
Test::RemoteEvictorFactoryI::createEvictor(const string& name, const Current& current)
{
    Freeze::EvictorPtr evictor = Freeze::createEvictor(_adapter->getCommunicator(), _envName, name);

    RemoteEvictorIPtr remoteEvictor = new RemoteEvictorI(_adapter, name, evictor);
    Freeze::ServantInitializerPtr initializer = new Initializer(remoteEvictor, evictor);
    evictor->installServantInitializer(initializer);
    return RemoteEvictorPrx::uncheckedCast(_adapter->add(remoteEvictor, stringToIdentity(name)));
}

void
Test::RemoteEvictorFactoryI::shutdown(const Current&)
{
    _adapter->getCommunicator()->shutdown();
}
