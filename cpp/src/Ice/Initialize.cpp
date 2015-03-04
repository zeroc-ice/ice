// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// We disable deprecation warning here, to allow clean compilation of
// of deprecated methods from StreamI.h.
//
#ifdef _MSC_VER
#   pragma warning( disable : 4996 )
#endif

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/ArgVector.h>
#include <Ice/GC.h>
#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/StreamI.h>
#include <Ice/LoggerI.h>
#include <Ice/Instance.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{

extern IceUtil::Handle<IceInternal::GC> theCollector;

}

namespace
{

pair<const Byte*, const Byte*>
makePair(const vector<Byte>& v)
{
    if(v.empty())
    {
        return pair<const Byte*, const Byte*>(static_cast<Byte*>(0), static_cast<Byte*>(0));
    }
    else
    {
        return pair<const Byte*, const Byte*>(&v[0], &v[0] + v.size());
    }
}

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

#ifdef _WIN32

StringSeq
Ice::argsToStringSeq(int argc, wchar_t* argv[])
{
   return argsToStringSeq(argc, argv, 0);
}

StringSeq
Ice::argsToStringSeq(int /*argc*/, wchar_t* argv[], const StringConverterPtr& converter)
{
    StringSeq args;
    for(int i=0; argv[i] != 0; i++)
    {
        string value = IceUtil::wstringToString(argv[i]);
        value = Ice::UTF8ToNative(converter, value);
        args.push_back(value);
    }
    return args;
}

#endif

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

Ice::ThreadHookPlugin::ThreadHookPlugin(const CommunicatorPtr& communicator, const ThreadNotificationPtr& threadHook)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    instance->setThreadHook(threadHook);
}

void
Ice::ThreadHookPlugin::initialize()
{
}

void
Ice::ThreadHookPlugin::destroy()
{
}

namespace
{

inline void checkIceVersion(Int version)
{
#ifndef ICE_IGNORE_VERSION

#   if ICE_INT_VERSION % 100 > 50
    //
    // Beta version: exact match required
    //
    if(ICE_INT_VERSION != version)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }
#   else

    //
    // Major and minor version numbers must match.
    //
    if(ICE_INT_VERSION / 100 != version / 100)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }

    //
    // Reject beta caller
    //
    if(version % 100 > 50)
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
    
#   endif    
#endif
}

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
Ice::initialize(StringSeq& args, const InitializationData& initializationData, Int version)
{
    CommunicatorPtr communicator;
    IceUtilInternal::ArgVector av(args);
    communicator = initialize(av.argc, av.argv, initializationData, version);
    args = argsToStringSeq(av.argc, av.argv);
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

    CommunicatorI* communicatorI = new CommunicatorI(initData);
    CommunicatorPtr result = communicatorI; // For exception safety.
    int argc = 0;
    char* argv[] = { 0 };
    communicatorI->finishSetup(argc, argv);
    return result;
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes)
{
    return new InputStreamI(communicator, makePair(bytes), true);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes, const EncodingVersion& v)
{
    return new InputStreamI(communicator, makePair(bytes), v, true);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes)
{
    return new InputStreamI(communicator, makePair(bytes), false);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes, const EncodingVersion& v)
{
    return new InputStreamI(communicator, makePair(bytes), v, false);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    return new InputStreamI(communicator, bytes, true);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes,
                       const EncodingVersion& v)
{
    return new InputStreamI(communicator, bytes, v, true);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    return new InputStreamI(communicator, bytes, false);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes,
                     const EncodingVersion& v)
{
    return new InputStreamI(communicator, bytes, v, false);
}

OutputStreamPtr
Ice::createOutputStream(const CommunicatorPtr& communicator)
{
    return new OutputStreamI(communicator);
}

OutputStreamPtr
Ice::createOutputStream(const CommunicatorPtr& communicator, const EncodingVersion& v)
{
    return new OutputStreamI(communicator, v);
}

static IceUtil::Mutex* processLoggerMutex = 0;
static Ice::LoggerPtr processLogger;

namespace
{

class Init
{
public:

    Init()
    {
        processLoggerMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete processLoggerMutex;
        processLoggerMutex = 0;
    }
};

Init init;

}

LoggerPtr
Ice::getProcessLogger()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processLoggerMutex);

    if(processLogger == 0)
    {
       //
       // TODO: Would be nice to be able to use process name as prefix by default.
       //
       processLogger = new Ice::LoggerI("", "");
    }
    return processLogger;
}

void
Ice::setProcessLogger(const LoggerPtr& logger)
{
   IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processLoggerMutex);
   processLogger = logger;
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}

#ifdef ICE_CPP11
void
IceInternal::Cpp11Dispatcher::dispatch(const ::Ice::DispatcherCallPtr& call, const ::Ice::ConnectionPtr& conn)
{
    _cb(call, conn);
}
#endif
