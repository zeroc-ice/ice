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
    return value;
}

void
Test::ServantI::setValue(Int val, const Current&)
{
    value = val;
}

void
Test::ServantI::saveValue(Int val, const Current& current)
{
    value = val;
    _evictor->saveObject(current.id);
}


void
Test::ServantI::setValueAsync_async(const AMD_Servant_setValueAsyncPtr& __cb, Int value, const Current&)
{
    _setValueAsyncCB = __cb;
    _setValueAsyncValue = value;
}

void
Test::ServantI::releaseAsync(const Current& current) const
{
    if(_setValueAsyncCB)
    {
        const_cast<Int&>(value) = _setValueAsyncValue;
        _setValueAsyncCB->ice_response();
        const_cast<AMD_Servant_setValueAsyncPtr&>(_setValueAsyncCB) = 0;
    }
}

void
Test::ServantI::destroy(const Current& current)
{
    _evictor->destroyObject(current.id);
}

void
Test::ServantI::__write(IceInternal::BasicStream* os) const
{
    assert(_remoteEvictor);
    _remoteEvictor->setLastSavedValue(value);
    Servant::__write(os);
}

void
Test::ServantI::__marshal(const StreamPtr& os) const
{
    assert(_remoteEvictor);
    _remoteEvictor->setLastSavedValue(value);
    Servant::__marshal(os);
}

Test::RemoteEvictorI::RemoteEvictorI(const ObjectAdapterPtr& adapter, const string& category, const Freeze::DBPtr& db,
                                     const StrategyIPtr& strategy, const Freeze::EvictorPtr& evictor) :
    _adapter(adapter),
    _category(category),
    _db(db),
    _strategy(strategy),
    _evictor(evictor),
    _lastSavedValue(-1)
{
}

void
Test::RemoteEvictorI::setSize(Int size, const Current&)
{
    _evictor->setSize(size);
}

Test::ServantPrx
Test::RemoteEvictorI::createServant(Int value, const Current&)
{
    Identity id;
    id.category = _category;
    ostringstream ostr;
    ostr << value;
    id.name = ostr.str();
    ServantPtr servant = new ServantI(this, _evictor, value);
    _evictor->createObject(id, servant);
    return ServantPrx::uncheckedCast(_adapter->createProxy(id));
}

Int
Test::RemoteEvictorI::getLastSavedValue(const Current&) const
{
    Int result = _lastSavedValue;
    (const_cast<RemoteEvictorI*>(this))->_lastSavedValue = -1;
    return result;
}

void
Test::RemoteEvictorI::clearLastSavedValue(const Current&)
{
    _lastSavedValue = -1;
}

Int
Test::RemoteEvictorI::getLastEvictedValue(const Current&) const
{
    return _strategy->getLastEvictedValue();
}

void
Test::RemoteEvictorI::clearLastEvictedValue(const Current&)
{
    _strategy->clearLastEvictedValue();
}

void
Test::RemoteEvictorI::deactivate(const Current& current)
{
    _adapter->removeServantLocator(_category);
    _adapter->remove(stringToIdentity(_category));
    _db->close();
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
                                                   const Freeze::DBEnvironmentPtr& dbEnv) :
    _adapter(adapter),
    _dbEnv(dbEnv)
{
}

::Test::RemoteEvictorPrx
Test::RemoteEvictorFactoryI::createEvictor(const string& name,
                                           Test::Strategy mode,
                                           const Current& current)
{
    Freeze::DBPtr db = _dbEnv->openDB(name, true);

    Freeze::PersistenceStrategyPtr delegate;
    if(mode == Test::Eviction)
    {
        delegate = db->createEvictionStrategy();
    }
    else
    {
        delegate = db->createIdleStrategy();
    }
    StrategyIPtr strategy = new StrategyI(delegate);
    Freeze::EvictorPtr evictor = db->createEvictor(strategy);
    _adapter->addServantLocator(evictor, name);

    RemoteEvictorIPtr remoteEvictor = new RemoteEvictorI(_adapter, name, db, strategy, evictor);
    Freeze::ServantInitializerPtr initializer = new Initializer(remoteEvictor, evictor);
    evictor->installServantInitializer(initializer);
    return RemoteEvictorPrx::uncheckedCast(_adapter->add(remoteEvictor, stringToIdentity(name)));
}

void
Test::RemoteEvictorFactoryI::shutdown(const Current&)
{
    _dbEnv->getCommunicator()->shutdown();
}

Test::StrategyI::StrategyI(const Freeze::PersistenceStrategyPtr& delegate) :
    _delegate(delegate), _lastEvictedValue(-1)
{
}

LocalObjectPtr
Test::StrategyI::activatedObject(const Identity& ident,
                                 const ObjectPtr& servant)
{
    return _delegate->activatedObject(ident, servant);
}

void
Test::StrategyI::destroyedObject(const Identity& ident, const LocalObjectPtr& cookie)
{
    _delegate->destroyedObject(ident, cookie);
}

void
Test::StrategyI::evictedObject(const Freeze::ObjectStorePtr& store,
                               const Identity& ident,
                               const ObjectPtr& servant,
                               const LocalObjectPtr& cookie)
{
    ServantIPtr s = ServantIPtr::dynamicCast(servant);
    _lastEvictedValue = s->getValue();

    _delegate->evictedObject(store, ident, servant, cookie);
}

void
Test::StrategyI::savedObject(const Freeze::ObjectStorePtr& store,
			     const Identity& ident,
			     const ObjectPtr& servant,
			     const LocalObjectPtr& cookie,
			     Ice::Int usageCount)
{
    _delegate->savedObject(store, ident, servant, cookie, usageCount);
}


void
Test::StrategyI::preOperation(const Freeze::ObjectStorePtr& store,
                              const Identity& ident,
                              const ObjectPtr& servant,
                              bool mutating,
                              const LocalObjectPtr& cookie)
{
    _delegate->preOperation(store, ident, servant, mutating, cookie);
}

void
Test::StrategyI::postOperation(const Freeze::ObjectStorePtr& store,
                               const Identity& ident,
                               const ObjectPtr& servant,
                               bool mutating,
                               const LocalObjectPtr& cookie)
{
    _delegate->postOperation(store, ident, servant, mutating, cookie);
}

void
Test::StrategyI::destroy()
{
    _delegate->destroy();
}

Int
Test::StrategyI::getLastEvictedValue()
{
    Int result = _lastEvictedValue;
    _lastEvictedValue = -1;
    return result;
}

void
Test::StrategyI::clearLastEvictedValue()
{
    _lastEvictedValue = -1;
}
