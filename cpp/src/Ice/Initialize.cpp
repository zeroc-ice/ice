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

#include <IceUtil/GC.h>
#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{

extern IceUtil::Handle<IceUtil::GC> theCollector;

void collectGarbage()
{
    if(theCollector)
    {
	theCollector->collectGarbage();
    }
}

}

CommunicatorPtr
Ice::initialize(int& argc, char* argv[], Int version)
{
#ifndef ICE_IGNORE_VERSION
    if(version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    PropertiesPtr defaultProperties = getDefaultProperties(argc, argv);
    CommunicatorI* communicatorI = new CommunicatorI(argc, argv, defaultProperties);
    CommunicatorPtr result = communicatorI; // For exception safety.
    communicatorI->finishSetup(argc, argv);
    return result;
}

CommunicatorPtr
Ice::initializeWithProperties(int& argc, char* argv[], const PropertiesPtr& properties, Int version)
{
#ifndef ICE_IGNORE_VERSION
    if(version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    CommunicatorI* communicatorI = new CommunicatorI(argc, argv, properties);
    CommunicatorPtr result = communicatorI; // For exception safety.
    communicatorI->finishSetup(argc, argv);
    return result;
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
Ice::getDefaultProperties()
{
    if(!defaultProperties)
    {
	defaultProperties = createProperties();
    }
    return defaultProperties;
}

PropertiesPtr
Ice::getDefaultProperties(StringSeq& args)
{
    if(!defaultProperties)
    {
	defaultProperties = createProperties(args);
    }
    return defaultProperties;
}

PropertiesPtr
Ice::getDefaultProperties(int& argc, char* argv[])
{
    if(!defaultProperties)
    {
	defaultProperties = createProperties(argc, argv);
    }
    return defaultProperties;
}

PropertiesPtr
Ice::createProperties()
{
    return new PropertiesI();
}

PropertiesPtr
Ice::createProperties(StringSeq& args)
{
    return new PropertiesI(args);
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[])
{
    return new PropertiesI(argc, argv);
}

StringSeq
Ice::argsToStringSeq(int argc, char* argv[])
{
    StringSeq result;
    for(int i = 0; i < argc; i++)
    {
        result.push_back(argv[i]);
    }
    return result;
}

void
Ice::stringSeqToArgs(const StringSeq& args, int& argc, char* argv[])
{
    //
    // Shift all elements in argv which are present in args to the
    // beginning of argv.
    //
    int i = 0;
    while(i < argc)
    {
        if(find(args.begin(), args.end(), argv[i]) == args.end())
        {
            for(int j = i; j < argc - 1; j++)
            {
                argv[j] = argv[j + 1];
            }
            --argc;
        }
        else
        {
            ++i;
        }
    }

    //
    // Make sure that argv[argc] == 0, the ISO C++ standard requires this.
    //
    if(argv)
    {
	argv[argc] = 0;
    }
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}
