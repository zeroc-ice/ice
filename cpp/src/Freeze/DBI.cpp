// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Freeze/DBI.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

DBPtr
Freeze::DBFactoryI::createDB()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    return 0;
}

void
Freeze::DBFactoryI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroy = true;
}

Freeze::DBFactoryI::DBFactoryI(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _communicator(communicator),
    _properties(properties),
    _destroy(false)
{
}

Freeze::DBFactoryI::~DBFactoryI()
{
    if (!_destroy)
    {
	_communicator->getLogger()->warning("database factory object has not been destroyed");
    }
}

DBFactoryPtr
Freeze::initialize(const CommunicatorPtr& communicator)
{
    return new DBFactoryI(communicator, communicator->getProperties());
}

DBFactoryPtr
Freeze::initializeWithProperties(const CommunicatorPtr& communicator, const PropertiesPtr& properties)
{
    return new DBFactoryI(communicator, properties);
}
