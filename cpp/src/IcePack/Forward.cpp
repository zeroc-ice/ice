// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/Forward.h>
#include <IcePack/Admin.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::Forward::Forward(const CommunicatorPtr& communicator, const AdminPtr& admin) :
    _communicator(communicator),
    _admin(admin)
{
    _activator = new Activator(_communicator);
    _activator->start();
}

IcePack::Forward::~Forward()
{
    _activator->destroy();
}

ObjectPtr
IcePack::Forward::locate(const ObjectAdapterPtr& adapter, const string& identity, ObjectPtr&)
{
    ObjectPrx proxy = adapter->identityToProxy(identity);
    ServerDescriptionPtr desc = _admin->find(proxy);
    if (desc)
    {
	assert(desc->object);
	_activator->activate(desc);
	throw LocationForward(desc->object);
    }
    return 0;
}

void
IcePack::Forward::finished(const ObjectAdapterPtr&, const string&, const ObjectPtr&, const ObjectPtr&)
{
    // Nothing to do
}
