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
#include <IcePack/AdapterManagerI.h>
#include <IcePack/ServerManager.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::AdapterI::AdapterI(Int waitTime) :
    _waitTime(waitTime)
{
}

IcePack::AdapterI::~AdapterI()
{
}

AdapterDescription
IcePack::AdapterI::getAdapterDescription(const Current&)
{
    return _description;
}

ObjectPrx
IcePack::AdapterI::getDirectProxy(bool activate, const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(!activate || !_description.server || _active)
    {
	return _proxy;
    }

    //
    // If there's a server associated to this adapter, try to start
    // the server and wait for the adapter state to change.
    //
    if(_description.server && _description.server->start())
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
		throw AdapterActivationTimeoutException();
	    }
	}
    }
    
    return _proxy;
}

void
IcePack::AdapterI::setDirectProxy(const ObjectPrx& proxy, const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _proxy = proxy;
}

void
IcePack::AdapterI::markAsActive(const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _active = true;
    notifyAll();
}

void
IcePack::AdapterI::markAsInactive(const Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _active = false;
    notifyAll();
}

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

IcePack::AdapterManagerI::AdapterManagerI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
    Ice::PropertiesPtr properties = adapter->getCommunicator()->getProperties();
    _waitTime = properties->getPropertyAsIntWithDefault("IcePack.Activation.WaitTime", 10);
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
    
    AdapterPtr adapterI = new AdapterI(_waitTime);
    adapterI->_description = description;
    adapterI->_active = false;
    adapterI->_proxy = 0;

    //
    // Add this adapter name to our adapter names internal set.
    //
    _adapterNames.insert(description.name);

    return AdapterPrx::uncheckedCast(_adapter->add(adapterI, adapter->ice_getIdentity()));
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

    _adapter->remove(adapter->ice_getIdentity());

    //
    // Remove the adapter name from our internal name set.
    //
    _adapterNames.erase(_adapterNames.find(name));
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
