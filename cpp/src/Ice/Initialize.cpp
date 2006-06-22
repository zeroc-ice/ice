// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/GC.h>
#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/StreamI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{

extern IceUtil::Handle<IceInternal::GC> theCollector;

}

void
Ice::collectGarbage()
{
    if(theCollector)
    {
	theCollector->collectGarbage();
    }
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
    // beginning of argv. We record the original value of argc so
    // that we can know later if we've shifted the array.
    //
    const int argcOrig = argc;
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
    // We can only do this if we've shifted the array, otherwise argv[argc]
    // may point to an invalid address.
    //
    if(argv && argcOrig != argc)
    {
	argv[argc] = 0;
    }
}

PropertiesPtr
Ice::createProperties(const StringConverterPtr& converter)
{
    return new PropertiesI(converter);
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults, const StringConverterPtr& converter)
{
    return new PropertiesI(args, defaults, converter);
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
    if(ICE_INT_VERSION / 100 != version / 100)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
    //
    // The caller's patch level cannot be greater than library's patch level. (Patch level changes are
    // backward-compatible, but not forward-compatible.)
    //
    if(version % 100 > ICE_INT_VERSION % 100)
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
    initData.properties = createProperties(argc, argv, initData.properties, initData.stringConverter);

    CommunicatorI* communicatorI = new CommunicatorI(initData);
    CommunicatorPtr result = communicatorI; // For exception safety.
    communicatorI->finishSetup(argc, argv);
    return result;
}

CommunicatorPtr
Ice::initialize(const InitializationData& initData, Int version)
{
    //
    // We can't simply call the other initialize() because this one does NOT read
    // the config file, while the other one always does.
    //
    checkIceVersion(version);

    CommunicatorI* communicatorI = new CommunicatorI(initData);
    CommunicatorPtr result = communicatorI; // For exception safety.
    int argc = 0;
    char* argv[] = { 0 };
    communicatorI->finishSetup(argc, argv);
    return result;
}


CommunicatorPtr
Ice::initializeWithProperties(int& argc, char* argv[], const PropertiesPtr& properties, Int version)
{
    InitializationData initData;
    initData.properties = properties;
    return initialize(argc, argv, initData, version);
}

CommunicatorPtr
Ice::initializeWithLogger(int& argc, char* argv[], const LoggerPtr& logger, Int version)
{
    InitializationData initData;
    initData.logger = logger;
    return initialize(argc, argv, initData, version);
}

CommunicatorPtr
Ice::initializeWithPropertiesAndLogger(int& argc, char* argv[], const PropertiesPtr& properties,
				       const LoggerPtr& logger, Int version)
{
    InitializationData initData;
    initData.properties = properties;
    initData.logger = logger;
    return initialize(argc, argv, initData, version);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes)
{
    return new InputStreamI(communicator, bytes);
}

OutputStreamPtr
Ice::createOutputStream(const CommunicatorPtr& communicator)
{
    return new OutputStreamI(communicator);
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}
