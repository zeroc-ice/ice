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

/**
 * Converts an argument vector into a string sequence.
 * @param argc The number of arguments in argv.
 * @param argv The arguments.
 * @return A string sequence containing the arguments.
 */
ICE_API StringSeq argsToStringSeq(int argc, const char* const argv[]);

#ifdef _WIN32
/**
 * Converts an argument vector into a string sequence.
 * @param argc The number of arguments in argv.
 * @param argv The arguments.
 * @return A string sequence containing the arguments.
 */
ICE_API StringSeq argsToStringSeq(int argc, const wchar_t* const argv[]);
#endif

/**
 * Updates the argument vector to match the contents of the string sequence.
 * This function assumes that the string sequence only contains
 * elements of the argument vector. The function shifts the
 * the argument vector elements so that the vector matches the
 * contents of the sequence.
 * @param seq The string sequence returned from a call to argsToStringSeq.
 * @param argc Updated to reflect the size of the sequence.
 * @param argv Elements are shifted to match the sequence.
 */
ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const char* argv[]);

/**
 * Updates the argument vector to match the contents of the string sequence.
 * This function assumes that the string sequence only contains
 * elements of the argument vector. The function shifts the
 * the argument vector elements so that the vector matches the
 * contents of the sequence.
 * @param seq The string sequence returned from a call to argsToStringSeq.
 * @param argc Updated to reflect the size of the sequence.
 * @param argv Elements are shifted to match the sequence.
 */
inline void stringSeqToArgs(const StringSeq& seq, int& argc, char* argv[])
{
    return stringSeqToArgs(seq, argc, const_cast<const char**>(argv));
}

#ifdef _WIN32
/**
 * Updates the argument vector to match the contents of the string sequence.
 * This function assumes that the string sequence only contains
 * elements of the argument vector. The function shifts the
 * the argument vector elements so that the vector matches the
 * contents of the sequence.
 * @param seq The string sequence returned from a call to argsToStringSeq.
 */
ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const wchar_t* argv[]);

/**
 * Updates the argument vector to match the contents of the string sequence.
 * This function assumes that the string sequence only contains
 * elements of the argument vector. The function shifts the
 * the argument vector elements so that the vector matches the
 * contents of the sequence.
 * @param seq The string sequence returned from a call to argsToStringSeq.
 */
inline void stringSeqToArgs(const StringSeq& seq, int& argc, wchar_t* argv[])
{
    return stringSeqToArgs(seq, argc, const_cast<const wchar_t**>(argv));
}
#endif

/**
 * Creates a new empty property set.
 *
 * @return A new empty property set.
 */
ICE_API PropertiesPtr createProperties();

/**
 * Creates a property set initialized from command-line arguments
 * and a default property set.
 *
 * @param seq Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this container upon return.
 *
 * @param defaults Default values for the property set. Settings in
 * configuration files and the arguments override these defaults.
 *
 * @return A new property set initialized with the property settings
 * that were removed from the argument vector.
 */
ICE_API PropertiesPtr createProperties(StringSeq& seq, const PropertiesPtr& defaults = 0);

/**
 * Creates a property set initialized from command-line arguments
 * and a default property set.
 *
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 *
 * @param defaults Default values for the property set. Settings in
 * configuration files and the arguments override these defaults.
 *
 * @return A new property set initialized with the property settings
 * that were removed from the argument vector.
 */
ICE_API PropertiesPtr createProperties(int& argc, const char* argv[], const PropertiesPtr& defaults = 0);

/**
 * Creates a property set initialized from command-line arguments
 * and a default property set.
 *
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 *
 * @param defaults Default values for the property set. Settings in
 * configuration files and the arguments override these defaults.
 *
 * @return A new property set initialized with the property settings
 * that were removed from the argument vector.
 */
inline PropertiesPtr createProperties(int& argc, char* argv[], const PropertiesPtr& defaults = 0)
{
    return createProperties(argc, const_cast<const char**>(argv), defaults);
}

#ifdef _WIN32
/**
 * Creates a property set initialized from command-line arguments
 * and a default property set.
 *
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 *
 * @param defaults Default values for the property set. Settings in
 * configuration files and the arguments override these defaults.
 *
 * @return A new property set initialized with the property settings
 * that were removed from the argument vector.
 */
ICE_API PropertiesPtr createProperties(int& argc, const wchar_t* argv[], const PropertiesPtr& defaults = 0);

/**
 * Creates a property set initialized from command-line arguments
 * and a default property set.
 *
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 *
 * @param defaults Default values for the property set. Settings in
 * configuration files and the arguments override these defaults.
 *
 * @return A new property set initialized with the property settings
 * that were removed from the argument vector.
 */
inline PropertiesPtr createProperties(int& argc, wchar_t* argv[], const PropertiesPtr& defaults = 0)
{
    return createProperties(argc, const_cast<const wchar_t**>(argv), defaults);
}
#endif

/**
 * Base class for a thread notification hook. An application can subclass this class,
 * implement start and stop, and install an instance in InitializationData in order
 * to receive notifications when Ice threads are started and stopped.
 * \headerfile Ice/Ice.h
 */
class ICE_API ThreadNotification : public IceUtil::Shared
{
public:

    /**
     * Called from the new Ice thread at startup.
     */
    virtual void start() = 0;

    /**
     * Called from an Ice thread that is about to stop.
     */
    virtual void stop() = 0;
};

typedef IceUtil::Handle<ThreadNotification> ThreadNotificationPtr;

/**
 * A special plug-in that installs a thread hook during a communicator's initialization.
 * Both initialize and destroy are no-op. See InitializationData.
 * \headerfile Ice/Ice.h
 */
class ICE_API ThreadHookPlugin : public Ice::Plugin
{
public:

#ifdef ICE_CPP11_MAPPING
    /**
     * Installs the thread hooks.
     * @param communicator The communicator in which to install the thread hooks.
     * @param start The start callback.
     * @param stop The stop callback.
     */
    ThreadHookPlugin(const CommunicatorPtr& communicator, std::function<void()> start, std::function<void()> stop);
#else
    /**
     * Installs the thread hooks.
     * @param communicator The communicator in which to install the thread hooks.
     * @param hook The thread notification callback object.
     */
    ThreadHookPlugin(const CommunicatorPtr& communicator, const ThreadNotificationPtr& hook);
#endif

    /** Not used. */
    virtual void initialize();

    /** Not used. */
    virtual void destroy();
};

/**
 * Encapsulates data to initialize a communicator.
 * \headerfile Ice/Ice.h
 */
struct InitializationData
{
    /**
     * The properties for the communicator.
     */
    PropertiesPtr properties;

    /**
     * The logger for the communicator.
     */
    LoggerPtr logger;

    /**
     * The communicator observer used by the Ice run-time.
     */
    Instrumentation::CommunicatorObserverPtr observer;

#ifdef ICE_CPP11_MAPPING
    /**
     * Called whenever the communicator starts a new thread.
     */
    std::function<void()> threadStart;

    /**
     * Called whenever a thread created by the communicator is about to be destroyed.
     */
    std::function<void()> threadStop;

    /**
      * You can control which thread receives operation invocations and AMI
      * callbacks by supplying a dispatcher.
      *
      * For example, you can use this dispatching facility to ensure that
      * all invocations and callbacks are dispatched in a GUI event loop
      * thread so that it is safe to invoke directly on GUI objects.
      *
      * The dispatcher is responsible for running (dispatching) the
      * invocation or AMI callback on its favorite thread.
      * @param call Represents the invocation. The dispatcher must eventually invoke this function.
      * @param con The connection associated with this dispatch, or nil if no connection is
      * associated with it.
      */
    std::function<void(std::function<void()> call, const std::shared_ptr<Ice::Connection>& con)> dispatcher;

    /**
     * Applications that make use of compact type IDs to conserve space
     * when marshaling class instances, and also use the streaming API to
     * extract such classes, can intercept the translation between compact
     * type IDs and their corresponding string type IDs by installing a
     * compact ID resolver.
     * @param id The compact ID.
     * @return The fully-scoped type ID such as "::Module::Class", or an empty string if
     * the compact ID is unknown.
     */
    std::function<std::string(int id)> compactIdResolver;

    /**
     * The batch request interceptor, which is called by the Ice run time to enqueue a batch request.
     * @param req An object representing the batch request.
     * @param count The number of requests currently in the queue.
     * @param size The number of bytes consumed by the requests currently in the queue.
     */
    std::function<void(const Ice::BatchRequest& req, int count, int size)> batchRequestInterceptor;
#else
    /**
     * The thread hook for the communicator.
     */
    ThreadNotificationPtr threadHook;

    /**
      * You can control which thread receives operation invocations and AMI
      * callbacks by supplying a dispatcher.
      *
      * For example, you can use this dispatching facility to ensure that
      * all invocations and callbacks are dispatched in a GUI event loop
      * thread so that it is safe to invoke directly on GUI objects.
      *
      * The dispatcher is responsible for running (dispatching) the
      * invocation or AMI callback on its favorite thread. It must eventually
      * execute the provided call.
      */
    DispatcherPtr dispatcher;

    /**
     * Applications that make use of compact type IDs to conserve space
     * when marshaling class instances, and also use the streaming API to
     * extract such classes, can intercept the translation between compact
     * type IDs and their corresponding string type IDs by installing a
     * compact ID resolver.
     */
    CompactIdResolverPtr compactIdResolver;

    /**
     * The batch request interceptor.
     */
    BatchRequestInterceptorPtr batchRequestInterceptor;
#endif

    /**
     * The value factory manager.
     */
    ValueFactoryManagerPtr valueFactoryManager;
};

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(int& argc, const char* argv[],
                                   const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
inline CommunicatorPtr initialize(int& argc, char* argv[], const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const char**>(argv), initData, version);
}

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(int& argc, const char* argv[], ICE_CONFIG_FILE_STRING configFile,
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
inline CommunicatorPtr initialize(int& argc, char* argv[], ICE_CONFIG_FILE_STRING configFile,
                                  int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const char**>(argv), configFile, version);
}

#ifdef _WIN32
/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(int& argc, const wchar_t* argv[],
                                   const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const wchar_t**>(argv), initData, version);
}

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(int& argc, const wchar_t* argv[], ICE_CONFIG_FILE_STRING configFile,
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param argc The number of arguments in argv. Upon return, this argument
 * is updated to reflect the arguments still remaining in argv.
 * @param argv Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this vector upon return.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], ICE_CONFIG_FILE_STRING configFile,
                                  int version = ICE_INT_VERSION)
{
    return initialize(argc, const_cast<const wchar_t**>(argv), configFile, version);
}
#endif

/**
 * Initializes a new communicator.
 * @param seq Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this container upon return.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(StringSeq& seq, const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param seq Command-line arguments, possibly containing
 * options to set properties. If the arguments include
 * a <code>--Ice.Config</code> option, the corresponding configuration
 * files are parsed. If the same property is set in a configuration
 * file and in the arguments, the arguments take precedence.
 * Recognized options are removed from this container upon return.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(StringSeq& seq, ICE_CONFIG_FILE_STRING configFile, int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param initData Configuration data for the new Communicator.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(const InitializationData& initData = InitializationData(),
                                   int version = ICE_INT_VERSION);

/**
 * Initializes a new communicator.
 * @param configFile The name of an Ice configuration file.
 * @param version Indicates the Ice version with which the application is compatible. If not
 * specified, the version of the Ice installation is used.
 * @return The new communicator.
 */
ICE_API CommunicatorPtr initialize(ICE_CONFIG_FILE_STRING configFile, int version = ICE_INT_VERSION);

/**
 * Obtains the per-process logger. This logger is used by all communicators that do not have their
 * own specific logger established at the time a communicator is created.
 * @return The current per-process logger instance.
 */
ICE_API LoggerPtr getProcessLogger();

/**
 * Sets the per-process logger. This logger is used by all communicators that do not have their
 * own specific logger established at the time a communicator is created.
 * @param logger The new per-process logger instance.
 */
ICE_API void setProcessLogger(const LoggerPtr& logger);

/**
 * A plug-in factory function is responsible for creating an Ice plug-in.
 * @param communicator The communicator in which the plug-in will be installed.
 * @param name The name assigned to the plug-in.
 * @param args Additional arguments included in the plug-in's configuration.
 * @return The new plug-in object. Returning nil will cause the run time to raise PluginInitializationException.
 */
typedef Ice::Plugin* (*PluginFactory)(const ::Ice::CommunicatorPtr& communicator, const std::string& name,
                                      const ::Ice::StringSeq& args);

/**
 * Manually registers a plug-in factory function.
 * @param name The name assigned to the plug-in.
 * @param factory The factory function.
 * @param loadOnInit If true, the plug-in is always loaded (created) during communicator initialization,
 * even if Ice.Plugin.name is not set. When false, the plug-in is loaded (created) during communication
 * initialization only if Ice.Plugin.name  is set to a non-empty value (e.g.: Ice.Plugin.IceSSL=1).
 */
ICE_API void registerPluginFactory(const std::string& name, PluginFactory factory, bool loadOnInit);

/**
 * A helper class that uses Resource Acquisition Is Initialization (RAII) to initialize and hold a
 * communicator instance, and automatically destroy the communicator when the holder goes out of scope.
 * \headerfile Ice/Ice.h
 */
class ICE_API CommunicatorHolder
{
public:

    /**
     * The holder's initial state is empty.
     */
    CommunicatorHolder();

#ifdef ICE_CPP11_MAPPING

    /**
     * Calls initialize to create a communicator with the provided arguments.
     * This constructor accepts all of the same overloaded argument styles as
     * initialize.
     */
    template<class... T>
    explicit CommunicatorHolder(T&&... args) :
        _communicator(std::move(initialize(std::forward<T>(args)...)))
    {
    }

    /**
     * Adopts the given communicator.
     * @param communicator The new communicator instance to hold.
     */
    explicit CommunicatorHolder(std::shared_ptr<Communicator> communicator);

    /**
     * Adopts the given communicator. If this holder currently holds a communicator,
     * it will be destroyed.
     * @param communicator The new communicator instance to hold.
     */
    CommunicatorHolder& operator=(std::shared_ptr<Communicator> communicator);

    CommunicatorHolder(const CommunicatorHolder&) = delete;
    CommunicatorHolder(CommunicatorHolder&&) = default;

    /**
     * Adopts the communicator in the given holder. If this holder currently holds a communicator,
     * it will be destroyed.
     * @param holder The holder from which to adopt a communicator.
     */
    CommunicatorHolder& operator=(CommunicatorHolder&& holder);

    /**
     * Determines whether the holder contains an instance.
     * @return True if the holder currently holds an instance, false otherwise.
     */
    explicit operator bool() const;

#else // C++98 mapping

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, const char* argv[], const InitializationData& initData = InitializationData(),
                       int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, char* argv[], const InitializationData& initData = InitializationData(),
                       int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, const char* argv[], const char* configFile, int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, char* argv[], const char* configFile, int version = ICE_INT_VERSION);

#   ifdef _WIN32
    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, const wchar_t* argv[], const InitializationData& initData = InitializationData(),
                       int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, wchar_t* argv[], const InitializationData& initData = InitializationData(),
                       int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, const wchar_t* argv[], const char* configFile, int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param argc The number of arguments in argv. Upon return, this argument
     * is updated to reflect the arguments still remaining in argv.
     * @param argv Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this vector upon return.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(int& argc, wchar_t* argv[], const char* configFile, int version = ICE_INT_VERSION);
#   endif

    /**
     * Initializes a new communicator.
     * @param seq Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this container upon return.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    explicit CommunicatorHolder(StringSeq& seq, const InitializationData& initData = InitializationData(),
                                int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param seq Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this container upon return.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    CommunicatorHolder(StringSeq& seq, const char* configFile, int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    explicit CommunicatorHolder(const InitializationData& initData, int version = ICE_INT_VERSION);

    /**
     * Initializes a new communicator.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     */
    explicit CommunicatorHolder(const char* configFile, int version = ICE_INT_VERSION);

    /**
     * Adopts the given communicator.
     * @param communicator The new communicator instance to hold.
     */
    explicit CommunicatorHolder(const CommunicatorPtr& communicator);

    /**
     * Adopts the given communicator. If this holder currently holds a communicator,
     * it will be destroyed.
     * @param communicator The new communicator instance to hold.
     */
    CommunicatorHolder& operator=(const CommunicatorPtr& communicator);

    /**
     * Determines whether the holder contains an instance.
     * @return True if the holder currently holds an instance, false otherwise.
     */
    operator bool() const;

    /// \cond INTERNAL
    //
    // Required for successful copy-initialization, but not
    // defined as it should always be elided by the compiler.
    CommunicatorHolder(const CommunicatorHolder&);
    /// \endcond

#endif

    ~CommunicatorHolder();

    /**
     * Obtains the communicator instance.
     * @return The communicator held by this holder, or nil if the holder is empty.
     */
    const CommunicatorPtr& communicator() const;

    /**
     * Obtains the communicator instance.
     * @return The communicator held by this holder, or nil if the holder is empty.
     */
    const CommunicatorPtr& operator->() const;

    /**
     * Obtains the communicator instance and clears the reference held by the holder.
     * @return The communicator held by this holder, or nil if the holder is empty.
     */
    CommunicatorPtr release();

private:

    CommunicatorPtr  _communicator;
};

/**
 * Converts a stringified identity into an Identity.
 * @param str The stringified identity.
 * @return An Identity structure containing the name and category components.
 */
ICE_API Identity stringToIdentity(const std::string& str);

/**
 * Converts an Identity structure into a string using the specified mode.
 * @param id The identity structure.
 * @param mode Affects the handling of non-ASCII characters and non-printable ASCII characters.
 * @return The stringified identity.
 */
ICE_API std::string identityToString(const Identity& id, ToStringMode mode = ICE_ENUM(ToStringMode, Unicode));

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
