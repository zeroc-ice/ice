// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <IceUtil/DisableWarnings.h>
#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/StreamI.h>
#include <Ice/LoggerI.h>
#include <Ice/Instance.h>
#include <Ice/PluginManagerI.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringConverter.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* globalMutex = 0;
Ice::LoggerPtr processLogger;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

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
Ice::argsToStringSeq(int /*argc*/, wchar_t* argv[])
{
    //
    // Don't need to use a wide string converter argv is expected to
    // come from Windows API.
    //
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
    StringSeq args;
    for(int i=0; argv[i] != 0; i++)
    {
        args.push_back(IceUtil::wstringToString(argv[i], converter));
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
Ice::createProperties()
{
    return PropertiesPtr(new PropertiesI(IceUtil::getProcessStringConverter()));
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults)
{
    return PropertiesPtr(new PropertiesI(args, defaults, IceUtil::getProcessStringConverter()));
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

#ifdef ICE_CPP11_MAPPING
Ice::ThreadHookPlugin::ThreadHookPlugin(const CommunicatorPtr& communicator,
                                        function<void()> threadStart,
                                        function<void()> threadStop)
{
    if(communicator == nullptr)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    instance->setThreadHook(move(threadStart), move(threadStop));
}
#else
Ice::ThreadHookPlugin::ThreadHookPlugin(const CommunicatorPtr& communicator, const ThreadNotificationPtr& threadHook)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    instance->setThreadHook(threadHook);
}
#endif
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


Ice::CommunicatorPtr
Ice::initialize(int& argc, char* argv[], const InitializationData& initializationData, Int version)
{
    checkIceVersion(version);

    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);

    CommunicatorIPtr communicator = CommunicatorI::create(initData);
    communicator->finishSetup(argc, argv);
    return communicator;
}

Ice::CommunicatorPtr
Ice::initialize(StringSeq& args, const InitializationData& initializationData, Int version)
{
    IceUtilInternal::ArgVector av(args);
    CommunicatorPtr communicator = initialize(av.argc, av.argv, initializationData, version);
    args = argsToStringSeq(av.argc, av.argv);
    return communicator;
}

Ice::CommunicatorPtr
Ice::initialize(const InitializationData& initData, Int version)
{
    //
    // We can't simply call the other initialize() because this one does NOT read
    // the config file, while the other one always does.
    //
    checkIceVersion(version);

    CommunicatorIPtr communicator = CommunicatorI::create(initData);
    int argc = 0;
    char* argv[] = { 0 };
    communicator->finishSetup(argc, argv);
    return communicator;
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

LoggerPtr
Ice::getProcessLogger()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);

    if(processLogger == ICE_NULLPTR)
    {
       //
       // TODO: Would be nice to be able to use process name as prefix by default.
       //
       processLogger = ICE_MAKE_SHARED(Ice::LoggerI, "", "", true, IceUtil::getProcessStringConverter());
    }
    return processLogger;
}

void
Ice::setProcessLogger(const LoggerPtr& logger)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    processLogger = logger;
}

void
Ice::registerPluginFactory(const std::string& name, PluginFactory factory, bool loadOnInitialize)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    PluginManagerI::registerPluginFactory(name, factory, loadOnInitialize);
}

#ifdef ICE_CPP11_MAPPING
Ice::CommunicatorHolder::CommunicatorHolder(shared_ptr<Communicator> communicator) :
    _communicator(std::move(communicator))
{
}

#else

Ice::CommunicatorHolder::CommunicatorHolder(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

#endif

Ice::CommunicatorHolder::~CommunicatorHolder()
{
    if(_communicator)
    {
        _communicator->destroy();
    }
}

const Ice::CommunicatorPtr&
Ice::CommunicatorHolder::communicator() const
{
    return _communicator;
}

Ice::CommunicatorPtr
Ice::CommunicatorHolder::release()
{
#ifdef ICE_CPP11_MAPPING
    return std::move(_communicator);
#else
    CommunicatorPtr result;
    result.swap(_communicator);
    return result;
#endif
}

const Ice::CommunicatorPtr&
Ice::CommunicatorHolder::operator->() const
{
    return _communicator;
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorIPtr p = ICE_DYNAMIC_CAST(::Ice::CommunicatorI, communicator);
    assert(p);
    return p->_instance;
}

IceUtil::TimerPtr
IceInternal::getInstanceTimer(const CommunicatorPtr& communicator)
{
    CommunicatorIPtr p = ICE_DYNAMIC_CAST(::Ice::CommunicatorI, communicator);
    assert(p);
    return p->_instance->timer();
}

