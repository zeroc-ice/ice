// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/Admin.h>
#include <HelloI.h>

using namespace std;
using namespace Demo;

HelloFactoryI::HelloFactoryI()
{
}

HelloPrx
HelloFactoryI::create(const string& name, const Ice::Current& current)
{
    Ice::ObjectAdapterPtr adapter = current.adapter;
    Ice::CommunicatorPtr communicator = adapter->getCommunicator();

    //
    // Create the servant and add it to the object adapter using the
    // given name as the identity.
    //
    Ice::ObjectPtr hello = new HelloI(name);
    Ice::ObjectPrx object = adapter->add(hello, Ice::stringToIdentity(name));

    //
    // Get the IcePack Admin interface and register the newly created
    // Hello object with the IcePack object registry.
    // 
    try
    {
	IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
	admin->addObject(object);
    }
    catch(const IcePack::ObjectExistsException&)
    {
	//
	// An object with the same identity is already registered with
	// the registry. Remove the object from the object adapter and
	// throw.
	//
	adapter->remove(object->ice_getIdentity());
	throw NameExistsException();
    }

    string id = communicator->getProperties()->getProperty("Identity");

    cout << "HelloFactory-" << id << ": created Hello object named '" << name << "'" << endl;

    return HelloPrx::uncheckedCast(object);
}

HelloPrx
HelloFactoryI::find(const string& name, const Ice::Current& current) const
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    //
    // The object is registered with the IcePack object registry so we
    // just return a proxy containing the identity.
    //
    try
    {
	return HelloPrx::checkedCast(communicator->stringToProxy(name));
    }
    catch(const Ice::NoEndpointException&)
    {
	//
	// The object couldn't be activated. Ignore.
	// 
	return HelloPrx::uncheckedCast(communicator->stringToProxy(name));
    }
    catch(const Ice::NotRegisteredException&)
    {
	//
	// The object is not registered.
	//
	throw NameNotExistException();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// The object doesn't exist anymore. This can occur if the
	// server has been restarted and the server objects haven't
	// been removed from the object registry.
	// 
	IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
	admin->removeObject(Ice::stringToIdentity(name));	
	throw NameNotExistException();
    }
}

HelloI::HelloI(const string& n)
{
    name = n;
}

void
HelloI::sayHello(const Ice::Current&) const
{
    cout << name << " says Hello World!" << endl;
}

void
HelloI::destroy(const Ice::Current& current)
{
    Ice::ObjectAdapterPtr adapter = current.adapter;
    Ice::CommunicatorPtr communicator = adapter->getCommunicator();

    //
    // Get the IcePack Admin interface and remove the Hello object
    // from the IcePack object registry.
    // 
    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
    admin->removeObject(current.id);

    //
    // Remove the Hello object from the object adapter.
    //
    adapter->remove(current.id);

    string id = communicator->getProperties()->getProperty("Identity");

    cout << "HelloFactory-" << id << ": destroyed Hello object named '" << name << "'" << endl;
}
