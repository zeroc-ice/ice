// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

CommunicatorPtr
Ice::initialize(int& argc, char* argv[], Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(getDefaultProperties(argc, argv));
}

CommunicatorPtr
Ice::initializeWithProperties(const PropertiesPtr& properties, Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(properties);
}

static PropertiesPtr defaultProperties;
class DefaultPropertiesDestroyer
{
public:

    ~DefaultPropertiesDestroyer()
    {
	defaultProperties = 0;
    }
};
static DefaultPropertiesDestroyer defaultPropertiesDestroyer;

PropertiesPtr
Ice::getDefaultProperties(int& argc, char* argv[])
{
    if (!defaultProperties)
    {
	defaultProperties = createProperties(argc, argv);
    }
    return defaultProperties;
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[])
{
    return new PropertiesI(argc, argv);
}

PropertiesPtr
Ice::createPropertiesFromFile(int& argc, char* argv[], const string& file)
{
    return new PropertiesI(argc, argv, file);
}

void
Ice::addArgumentPrefix(const std::string& prefix)
{
    PropertiesI::addArgumentPrefix(prefix);
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}
