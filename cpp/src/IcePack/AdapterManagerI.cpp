// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <Freeze/Evictor.h>
#include <IcePack/AdapterManagerI.h>
#include <IcePack/ServerManager.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

namespace IcePack
{

class AdapterNameToAdapter
{
public:

    AdapterNameToAdapter(const ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    AdapterPrx
    operator()(const string& name)
    {
	Identity ident;
	ident.category = "adapter";
	ident.name = name;
	return AdapterPrx::uncheckedCast(_adapter->createProxy(ident));
    }

private:

    ObjectAdapterPtr _adapter;
};

class AdapterFactory : public ObjectFactory
{
public:

    AdapterFactory(const TraceLevelsPtr& traceLevels, int waitTime) : 
	_traceLevels(traceLevels),
	_waitTime(waitTime)
    {
    }

    //
    // Operations from ObjectFactory
    //
    virtual Ice::ObjectPtr 
    create(const std::string& type)
    {
	assert(type == "::IcePack::Adapter");
	return new AdapterI(_traceLevels, _waitTime);
    }

    virtual void 
    destroy()
    {
    }

private:
    
    TraceLevelsPtr _traceLevels;
    int _waitTime;
};

}

IcePack::AdapterI::AdapterI(const TraceLevelsPtr& traceLevels, Int waitTime) :
    _traceLevels(traceLevels),
    _waitTime(waitTime),
    _active(false)
{
}

IcePack::AdapterI::~AdapterI()
{
}

AdapterDescription
IcePack::AdapterI::getAdapterDescription(const Current&)
{
    return description;
}

ObjectPrx
IcePack::AdapterI::getDirectProxy(bool activate, const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(!activate || !description.server || _active)
    {
	return proxy;
    }

    //
    // If there's a server associated to this adapter, try to start
    // the server and wait for the adapter state to change.
    //
    if(description.server)
    {
	if(_traceLevels->adapterMgr > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	    out << "waiting for activation of adapter `" << description.name << "'";
	}

	if(description.server->start())
	{
	    //
	    // Wait for this adapter to be marked as active or the
	    // activation timeout.
	    //
	    while(!_active)
	    {
		bool notify = timedWait(IceUtil::Time::seconds(_waitTime));
		if(!notify)
		{
		    if(_traceLevels->adapterMgr > 1)
		    {
			Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
			out << "adapter `" << description.name << "' activation timed out";
		    }
		    throw AdapterActivationTimeoutException();
		}
	    }
	}
	else if(_traceLevels->adapterMgr > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	    out << "adapter `" << description.name << "' activation failed, couldn't start the server";
	}
    }
    
    return proxy;
}

void
IcePack::AdapterI::setDirectProxy(const ObjectPrx& proxy, const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    this->proxy = proxy;
}

void
IcePack::AdapterI::markAsActive(const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _active = true;

    if(_traceLevels->adapterMgr > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	out << "adapter `" << description.name << "' activated";
    }

    notifyAll();
}

void
IcePack::AdapterI::markAsInactive(const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _active = false;

    if(_traceLevels->adapterMgr > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	out << "adapter `" << description.name << "' deactivated";
    }

    notifyAll();
}

IcePack::AdapterManagerI::AdapterManagerI(const ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels,
					  const Freeze::DBEnvironmentPtr& dbEnv) :
    _adapter(adapter),
    _traceLevels(traceLevels)
{
    Ice::PropertiesPtr properties = adapter->getCommunicator()->getProperties();
    _waitTime = properties->getPropertyAsIntWithDefault("IcePack.Activation.WaitTime", 60);

    ObjectFactoryPtr adapterFactory = new AdapterFactory(_traceLevels, _waitTime);
    adapter->getCommunicator()->addObjectFactory(adapterFactory, "::IcePack::Adapter");

    Freeze::DBPtr dbAdapters = dbEnv->openDB("adapters", true);
    _evictor = dbAdapters->createEvictor(Freeze::SaveUponEviction);
    _evictor->setSize(1000);
    _adapter->addServantLocator(_evictor, "adapter");

    //
    // Cache the server names for getAll(). This will load all the
    // server objects at the begining and might cause slow startup.
    //
    Freeze::EvictorIteratorPtr p = _evictor->getIterator();
    while(p->hasNext())
    {
	AdapterPrx a = AdapterPrx::checkedCast(_adapter->createProxy(p->next()));
	assert(a);
	AdapterDescription desc = a->getAdapterDescription();
	_adapterNames.insert(desc.name);
    }
}

IcePack::AdapterManagerI::~AdapterManagerI()
{
}

AdapterPrx
IcePack::AdapterManagerI::create(const AdapterDescription& description, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    AdapterPrx adapter = AdapterNameToAdapter(_adapter)(description.name);
    try
    {
	adapter->ice_ping();
	
	//
	// The adapter already exists.
	//
	throw AdapterExistsException();
    }
    catch(const ObjectNotExistException&)
    {
    }
    
    AdapterPtr adapterI = new AdapterI(_traceLevels, _waitTime);
    adapterI->description = description;
    adapterI->proxy = 0;

    //
    // Add this adapter name to our adapter names internal set.
    //
    _adapterNames.insert(description.name);

    _evictor->createObject(adapter->ice_getIdentity(), adapterI);

    if(_traceLevels->adapterMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	out << "added adapter `" << description.name << "'";
    }

    return adapter;
}

AdapterPrx
IcePack::AdapterManagerI::findByName(const string& name, const Current&)
{
    AdapterPrx adapter = AdapterNameToAdapter(_adapter)(name);
    try
    {
	adapter->ice_ping();
	return adapter;
    }
    catch(const ObjectNotExistException&)
    {
	return 0;
    }
}

void
IcePack::AdapterManagerI::remove(const string& name, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    AdapterPrx adapter = AdapterNameToAdapter(_adapter)(name);
    try
    {
	adapter->ice_ping();
    }
    catch(const ObjectNotExistException&)
    {
	throw AdapterNotExistException();
    }

    _evictor->destroyObject(adapter->ice_getIdentity());

    //
    // Remove the adapter name from our internal name set.
    //
    _adapterNames.erase(_adapterNames.find(name));

    if(_traceLevels->adapterMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterMgrCat);
	out << "removed adapter `" << name << "'";
    }
}

AdapterNames
IcePack::AdapterManagerI::getAll(const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    AdapterNames names;
    names.reserve(_adapterNames.size());
    copy(_adapterNames.begin(), _adapterNames.end(), back_inserter(names));

    return names;
}
