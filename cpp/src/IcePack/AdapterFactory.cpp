// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <Freeze/Evictor.h>
#include <Freeze/Initialize.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/AdapterI.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::AdapterFactory::AdapterFactory(const Ice::ObjectAdapterPtr& adapter, 
					const TraceLevelsPtr& traceLevels, 
					const string& envName) :
    _adapter(adapter),
    _traceLevels(traceLevels)
{
    Ice::PropertiesPtr properties = _adapter->getCommunicator()->getProperties();

    //
    // Create and install the freeze evictor for standalone adapter objects.
    //
    properties->setProperty("Freeze.Evictor." + envName + ".adapter.SaveSizeTrigger", "1");
    _evictor = Freeze::createEvictor(_adapter, envName, "adapter");
    _evictor->setSize(1000);

    //
    // Install the server object factory.
    // 
    _adapter->getCommunicator()->addObjectFactory(this, "::IcePack::StandaloneAdapter");

    //
    // Install the evictors.
    //
    _adapter->addServantLocator(_evictor, "IcePackStandaloneAdapter");
}

//
// Ice::ObjectFactory::create method implementation
//
Ice::ObjectPtr
IcePack::AdapterFactory::create(const string& type)
{
    if(type == "::IcePack::StandaloneAdapter")
    {
	return new StandaloneAdapterI(this);
    }
    else
    {
	assert(false);
	return 0; // Keep the compiler happy.
    }
}

//
// Ice::ObjectFactory::destroy method implementation
//
void 
IcePack::AdapterFactory::destroy()
{
    _adapter = 0;
    _evictor = 0;
    _traceLevels = 0;
}

//
// Create a new adapter servant with the given name and add
// it the evictor database.
//
AdapterPrx
IcePack::AdapterFactory::createStandaloneAdapter(const string& name)
{
    StandaloneAdapterPtr adapterI = new StandaloneAdapterI(this);

    Ice::Identity id;
    id.category = "IcePackStandaloneAdapter";
    id.name = name + "-" + IceUtil::generateUUID();

    _evictor->add(adapterI, id);

    return AdapterPrx::uncheckedCast(_adapter->createProxy(id));
}

void
IcePack::AdapterFactory::destroy(const Ice::Identity& id)
{
    try
    {
	_evictor->remove(id);
    }
    catch(const Freeze::DatabaseException& ex)
    {
	ostringstream os;
	os << "couldn't destroy standalone adapter:\n" << ex;
	_traceLevels->logger->warning(os.str());
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
	assert(false);
    }
}
