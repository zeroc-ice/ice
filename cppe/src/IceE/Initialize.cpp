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
Ice::createProperties(const StringConverterPtr& converter)
{
    return new Properties(converter);
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults, const StringConverterPtr& converter)
{
    return new Properties(args, defaults, converter);
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[], const PropertiesPtr& defaults, const StringConverterPtr& converter)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults, converter);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

inline void checkIceVersion(Int version)
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
}


CommunicatorPtr
Ice::initialize(int& argc, char* argv[], const InitializationData& initializationData, Int version)
{
    checkIceVersion(version);

    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);

    CommunicatorPtr communicator = new Communicator(initData);
    communicator->finishSetup(argc, argv);
    return communicator;
}

CommunicatorPtr
Ice::initialize(const InitializationData& initData, Int version)
{
    //
    // We can't simply call the other initialize() because this one does NOT read
    // the config file, while the other one always does.
    //
    checkIceVersion(version);

    CommunicatorPtr communicator = new Communicator(initData);
    int argc = 0;
    char* argv[] = { 0 };
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
