// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include <IceUtil/Timer.h>
#include <Ice/Communicator.h>
#include <Ice/PropertiesF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerF.h>
#include <Ice/InstrumentationF.h>
#include <Ice/Dispatcher.h>
#include <Ice/FactoryTable.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Version.h>
#include <Ice/Plugin.h>
#include <Ice/BatchRequestInterceptor.h>

#ifdef ICE_CPP11_MAPPING
#   define ICE_CONFIG_FILE_STRING const std::string&
#else
#   define ICE_CONFIG_FILE_STRING const char*
#endif

namespace Ice
{

ICE_API StringSeq argsToStringSeq(int, const char* const[]);

#ifdef _WIN32
ICE_API StringSeq argsToStringSeq(int, const wchar_t* const[]);
#endif

//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICE_API void stringSeqToArgs(const StringSeq&, int&, const char*[]);
inline void stringSeqToArgs(const StringSeq& seq, int& argc, char* argv[])
{
    return stringSeqToArgs(seq, argc, const_cast<const char**>(argv));
}

#ifdef _WIN32
ICE_API void stringSeqToArgs(const StringSeq&, int&, const wchar_t*[]);
inline void stringSeqToArgs(const StringSeq& seq, int& argc, wchar_t* argv[])
{
    return stringSeqToArgs(seq, argc, const_cast<const wchar_t**>(argv));
}
#endif

ICE_API PropertiesPtr createProperties();
ICE_API PropertiesPtr createProperties(StringSeq&, const PropertiesPtr& = 0);
ICE_API PropertiesPtr createProperties(int&, const char*[], const PropertiesPtr& = 0);
inline PropertiesPtr createProperties(int& argc, char* argv[], const PropertiesPtr& props = 0)
{
    return createProperties(argc, const_cast<const char**>(argv), props);
}

#ifdef _WIN32
ICE_API PropertiesPtr createProperties(int&, const wchar_t*[], const PropertiesPtr& = 0);
inline PropertiesPtr createProperties(int& argc, wchar_t* argv[], const PropertiesPtr& props = 0)
{
    return createProperties(argc, const_cast<const wchar_t**>(argv), props);
}
#endif

//
// This class is used to notify user of when Ice threads are started
// and stopped.
//
class ICE_API ThreadNotification : public IceUtil::Shared
{
public:

    virtual void start() = 0;
    virtual void stop() = 0;
};

typedef IceUtil::Handle<ThreadNotification> ThreadNotificationPtr;

//
// A special plug-in that installs thread hook during a communicator's initialization.
// Both initialize and destroy are no-op. See Ice::InitializationData.
//
class ICE_API ThreadHookPlugin : public Ice::Plugin
{
public:

#ifdef ICE_CPP11_MAPPING
    ThreadHookPlugin(const CommunicatorPtr& communicator, std::function<void()>, std::function<void()>);
#else
    ThreadHookPlugin(const CommunicatorPtr& communicator, const ThreadNotificationPtr&);
#endif
    virtual void initialize();

    virtual void destroy();
};

//
// Communicator initialization info
//
struct InitializationData
{
    PropertiesPtr properties;
    LoggerPtr logger;
    Instrumentation::CommunicatorObserverPtr observer;
#ifdef ICE_CPP11_MAPPING
    std::function<void()> threadStart;
    std::function<void()> threadStop;
    std::function<void(std::function<void()>, const std::shared_ptr<Ice::Connection>&)> dispatcher;
    std::function<std::string(int)> compactIdResolver;
    std::function<void(const Ice::BatchRequest&, int, int)> batchRequestInterceptor;
#else
    ThreadNotificationPtr threadHook;
    DispatcherPtr dispatcher;
    CompactIdResolverPtr compactIdResolver;
    BatchRequestInterceptorPtr batchRequestInterceptor;
#endif
    ValueFactoryManagerPtr valueFactoryManager;
};

ICE_API CommunicatorPtr initialize(int&, const char*[], const InitializationData& = InitializationData(),
                                   int = ICE_INT_VERSION);
inline CommunicatorPtr initialize(int& argc, char* argv[], const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const char**>(argv), initData, version);
}


ICE_API CommunicatorPtr initialize(int&, const char*[], ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);
inline CommunicatorPtr initialize(int& argc, char* argv[], ICE_CONFIG_FILE_STRING configFile,
                                  int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const char**>(argv), configFile, version);
}

#ifdef _WIN32
ICE_API CommunicatorPtr initialize(int&, const wchar_t*[], const InitializationData& = InitializationData(),
                                   int = ICE_INT_VERSION);
inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const wchar_t**>(argv), initData, version);
}

ICE_API CommunicatorPtr initialize(int&, const wchar_t*[], ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);
inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], ICE_CONFIG_FILE_STRING configFile,
                                  int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const wchar_t**>(argv), configFile, version);
}
#endif

ICE_API CommunicatorPtr initialize(StringSeq&, const InitializationData& = InitializationData(),
                                   int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(StringSeq&, ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(const InitializationData& = InitializationData(),
                                   int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);


ICE_API LoggerPtr getProcessLogger();
ICE_API void setProcessLogger(const LoggerPtr&);

typedef Ice::Plugin* (*PluginFactory)(const ::Ice::CommunicatorPtr&, const std::string&, const ::Ice::StringSeq&);
ICE_API void registerPluginFactory(const std::string&, PluginFactory, bool);

//
// RAII helper class
//
class ICE_API CommunicatorHolder
{
public:

    //
    // Empty holder
    //
    CommunicatorHolder();

#ifdef ICE_CPP11_MAPPING

    //
    // Call initialize to create communicator with the provided args
    // (all except default ctor above)
    //
    //
    template<class... T>
    explicit CommunicatorHolder(T&&... args) :
        _communicator(std::move(initialize(std::forward<T>(args)...)))
    {
    }

    //
    // Adopt communicator
    //
    explicit CommunicatorHolder(std::shared_ptr<Communicator>);
    CommunicatorHolder& operator=(std::shared_ptr<Communicator>);

    CommunicatorHolder(const CommunicatorHolder&) = delete;

    CommunicatorHolder(CommunicatorHolder&&) = default;
    CommunicatorHolder& operator=(CommunicatorHolder&&);

    explicit operator bool() const;

#else // C++98 mapping

    //
    // Call initialize to create communicator with the provided args
    //
    CommunicatorHolder(int&, const char*[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    CommunicatorHolder(int&, char*[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    CommunicatorHolder(int&, const char*[], const char*, int = ICE_INT_VERSION);
    CommunicatorHolder(int&, char*[], const char*, int = ICE_INT_VERSION);

#   ifdef _WIN32
    CommunicatorHolder(int&, const wchar_t*[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    CommunicatorHolder(int&, wchar_t*[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    CommunicatorHolder(int&, const wchar_t*[], const char*, int = ICE_INT_VERSION);
    CommunicatorHolder(int&, wchar_t*[], const char*, int = ICE_INT_VERSION);
#   endif

    explicit CommunicatorHolder(StringSeq&, const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    CommunicatorHolder(StringSeq&, const char*, int = ICE_INT_VERSION);

    explicit CommunicatorHolder(const InitializationData&, int = ICE_INT_VERSION);
    explicit CommunicatorHolder(const char*, int = ICE_INT_VERSION);

    //
    // Adopt communicator
    //
    explicit CommunicatorHolder(const CommunicatorPtr&);
    CommunicatorHolder& operator=(const CommunicatorPtr&);

    operator bool() const;

    //
    // Required for successful copy-initialization, but not
    // defined as it should always be elided by compiler
    CommunicatorHolder(const CommunicatorHolder&);

#endif

    ~CommunicatorHolder();

    const CommunicatorPtr& communicator() const;
    const CommunicatorPtr& operator->() const;
    CommunicatorPtr release();

private:

    CommunicatorPtr  _communicator;
};

ICE_API Identity stringToIdentity(const std::string&);
ICE_API std::string identityToString(const Identity&, ToStringMode = ICE_ENUM(ToStringMode, Unicode));

}

namespace IceInternal
{

//
// Some Ice extensions need access to the Ice internal instance. Do
// not use this operation for regular application code! It is intended
// to be used by modules such as Freeze.
//
ICE_API InstancePtr getInstance(const ::Ice::CommunicatorPtr&);
ICE_API IceUtil::TimerPtr getInstanceTimer(const ::Ice::CommunicatorPtr&);

}

#endif
