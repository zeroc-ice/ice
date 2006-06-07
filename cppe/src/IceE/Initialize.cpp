// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Initialize.h>
#include <IceE/Communicator.h>
#include <IceE/Properties.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

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

PropertiesPtr
Ice::createProperties()
{
    return new Properties();
}

PropertiesPtr
Ice::createProperties(StringSeq& args)
{
    return new Properties(args);
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[])
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args);
    stringSeqToArgs(args, argc, argv);
    return properties;
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
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = getDefaultProperties(args);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

CommunicatorPtr
Ice::initialize(int& argc, char* argv[], const InitializationData& initData, Int version)
{
#ifndef ICE_IGNORE_VERSION
    //
    // Major and minor version numbers must match.
    //
    if(ICEE_INT_VERSION / 100 != version / 100)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
    //
    // The caller's patch level cannot be greater than library's patch level. (Patch level changes are
    // backward-compatible, but not forward-compatible.)
    //
    if(version % 100 > ICEE_INT_VERSION % 100)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    InitializationData tmpData = initData;
    if(!tmpData.properties)
    {
        tmpData.properties = getDefaultProperties(argc, argv);
    }

    StringSeq args = argsToStringSeq(argc, argv);
    args = tmpData.properties->parseIceCommandLineOptions(args);
    stringSeqToArgs(args, argc, argv);

    CommunicatorPtr communicator = new Communicator(tmpData);
    communicator->finishSetup(argc, argv);
    return communicator;
}

CommunicatorPtr
Ice::initializeWithProperties(int& argc, char* argv[], const PropertiesPtr& properties, Int version)
{
    InitializationData initData;
    initData.properties = properties;
    return initialize(argc, argv, initData, version);
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    return communicator->_instance;
}
