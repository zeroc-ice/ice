// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include <IceUtil/Timer.h>
#include <Ice/CommunicatorF.h>
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

namespace Ice
{

ICE_API StringSeq argsToStringSeq(int, char*[]);

#ifdef _WIN32

ICE_API StringSeq argsToStringSeq(int, wchar_t*[]);

#endif

//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICE_API void stringSeqToArgs(const StringSeq&, int&, char*[]);

ICE_API PropertiesPtr createProperties();
ICE_API PropertiesPtr createProperties(StringSeq&, const PropertiesPtr& = 0);
ICE_API PropertiesPtr createProperties(int&, char*[], const PropertiesPtr& = 0);

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

ICE_API CommunicatorPtr initialize(int&, char*[], const InitializationData& = InitializationData(),
                                   Int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(Ice::StringSeq&, const InitializationData& = InitializationData(),
                                   Int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(const InitializationData& = InitializationData(),
                                           Int = ICE_INT_VERSION);

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

#ifdef ICE_CPP11_MAPPING
    CommunicatorHolder(std::shared_ptr<Communicator>);

    CommunicatorHolder(const CommunicatorHolder&) = delete;

    CommunicatorHolder(CommunicatorHolder&&) = default;
    CommunicatorHolder& operator=(CommunicatorHolder&&) = default;

#else
    CommunicatorHolder(const CommunicatorPtr&);

    // Required for successful copy-initialization, but not
    // defined as it should always be elided by compiler
    CommunicatorHolder(const CommunicatorHolder&);

#endif

    ~CommunicatorHolder();

    const CommunicatorPtr& communicator() const;
    CommunicatorPtr release();
    const CommunicatorPtr& operator->() const;

private:

    CommunicatorPtr  _communicator;
};

ICE_API Identity stringToIdentity(const std::string&);
ICE_API std::string identityToString(const Identity&);
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
