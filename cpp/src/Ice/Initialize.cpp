// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <Ice/LoggerI.h>
#include <Ice/Instance.h>
#include <Ice/PluginManagerI.h>
#include <Ice/StringUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Ice/StringConverter.h>

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
    const StringConverterPtr converter = getProcessStringConverter();
    StringSeq args;
    for(int i=0; argv[i] != 0; i++)
    {
        args.push_back(wstringToString(argv[i], converter));
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

#ifdef _WIN32
void
Ice::stringSeqToArgs(const StringSeq& args, int& argc, wchar_t* argv[])
{
    //
    // Don't need to use a wide string converter argv is expected to
    // come from Windows API.
    //
    const StringConverterPtr converter = getProcessStringConverter();

    //
    // Shift all elements in argv which are present in args to the
    // beginning of argv. We record the original value of argc so
    // that we can know later if we've shifted the array.
    //
    const int argcOrig = argc;
    int i = 0;
    while(i < argc)
    {
        if(find(args.begin(), args.end(), wstringToString(argv[i], converter)) == args.end())
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
#endif

PropertiesPtr
Ice::createProperties()
{
    return ICE_MAKE_SHARED(PropertiesI);
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults)
{
    return ICE_MAKE_SHARED(PropertiesI, args, defaults);
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

#ifdef _WIN32
PropertiesPtr
Ice::createProperties(int& argc, wchar_t* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}
#endif

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

#ifdef _WIN32
Ice::CommunicatorPtr
Ice::initialize(int& argc, wchar_t* argv[], const InitializationData& initializationData, Int version)
{
    Ice::StringSeq args = argsToStringSeq(argc, argv);
    CommunicatorPtr communicator = initialize(args, initializationData, version);
    stringSeqToArgs(args, argc, argv);
    return communicator;
}
#endif

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

LoggerPtr
Ice::getProcessLogger()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);

    if(processLogger == ICE_NULLPTR)
    {
       //
       // TODO: Would be nice to be able to use process name as prefix by default.
       //
       processLogger = ICE_MAKE_SHARED(LoggerI, "", "", true);
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

Identity
Ice::stringToIdentity(const string& s)
{
    Identity ident;

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        int escapes = 0;
        while(static_cast<int>(pos) - escapes > 0 && s[pos - escapes - 1] == '\\')
        {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if(escapes % 2 == 0)
        {
            if(slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = "unescaped '/' in identity `" + s + "'";
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        try
        {
            ident.name = unescapeString(s, 0, s.size(), "/");
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid identity name `" + s + "': " + e.reason();
            throw ex;
        }
    }
    else
    {
        try
        {
            ident.category = unescapeString(s, 0, slash, "/");
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid category in identity `" + s + "': " + e.reason();
            throw ex;
        }

        if(slash + 1 < s.size())
        {
            try
            {
                ident.name = unescapeString(s, slash + 1, s.size(), "/");
            }
            catch(const IceUtil::IllegalArgumentException& e)
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = "invalid name in identity `" + s + "': " + e.reason();
                throw ex;
            }
        }
    }

    return ident;
}

string
Ice::identityToString(const Identity& ident, ToStringMode toStringMode)
{
    if(ident.category.empty())
    {
        return escapeString(ident.name, "/", toStringMode);
    }
    else
    {
        return escapeString(ident.category, "/", toStringMode) + '/' + escapeString(ident.name, "/", toStringMode);
    }
}
