// Copyright (c) ZeroC, Inc.

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include "BatchRequest.h"
#include "CommunicatorF.h"
#include "Connection.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/Timer.h"
#include "InstanceF.h"
#include "Instrumentation.h"
#include "Logger.h"
#include "Plugin.h"
#include "PropertiesF.h"
#include "SSL/ClientAuthenticationOptions.h"
#include "StringUtil.h"
#include "ValueFactory.h"

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
    ICE_API PropertiesPtr createProperties(StringSeq& seq, const PropertiesPtr& defaults = nullptr);

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
    ICE_API PropertiesPtr createProperties(int& argc, const char* argv[], const PropertiesPtr& defaults = nullptr);

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
    inline PropertiesPtr createProperties(int& argc, char* argv[], const PropertiesPtr& defaults = nullptr)
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
    ICE_API PropertiesPtr createProperties(int& argc, const wchar_t* argv[], const PropertiesPtr& defaults = nullptr);

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
    inline PropertiesPtr createProperties(int& argc, wchar_t* argv[], const PropertiesPtr& defaults = nullptr)
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
    class ICE_API ThreadNotification
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

    using ThreadNotificationPtr = std::shared_ptr<ThreadNotification>;

    /**
     * A special plug-in that installs a thread hook during a communicator's initialization.
     * Both initialize and destroy are no-op. See InitializationData.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ThreadHookPlugin : public Ice::Plugin
    {
    public:
        /**
         * Installs the thread hooks.
         * @param communicator The communicator in which to install the thread hooks.
         * @param start The start callback.
         * @param stop The stop callback.
         */
        ThreadHookPlugin(const CommunicatorPtr& communicator, std::function<void()> start, std::function<void()> stop);

        /** Not used. */
        void initialize() override;

        /** Not used. */
        void destroy() override;
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
        PropertiesPtr properties{};

        /**
         * The logger for the communicator.
         */
        LoggerPtr logger{};

        /**
         * The communicator observer used by the Ice run-time.
         */
        Instrumentation::CommunicatorObserverPtr observer{};

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdocumentation" // param/return is not recognized for std::function data members
#endif

        /**
         * Called whenever the communicator starts a new thread.
         */
        std::function<void()> threadStart{};

        /**
         * Called whenever a thread created by the communicator is about to be destroyed.
         */
        std::function<void()> threadStop{};

        /**
         * You can control which thread receives operation dispatches and async invocation
         * callbacks by supplying an executor.
         *
         * For example, you can use this execution facility to ensure that
         * all dispatches and invocation callbacks are executed in a GUI event loop
         * thread so that it is safe to invoke directly on GUI objects.
         *
         * The executor is responsible for running the dispatch or async invocation callback on its favorite thread.
         * @param call Represents the function to execute. The execute must eventually execute this function.
         * @param con The connection associated with this call, or null if no connection is associated with it.
         */
        std::function<void(std::function<void()> call, const Ice::ConnectionPtr& con)> executor{};

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
        std::function<std::string(int id)> compactIdResolver{};

        /**
         * The batch request interceptor, which is called by the Ice run time to enqueue a batch request.
         * @param req An object representing the batch request.
         * @param count The number of requests currently in the queue.
         * @param size The number of bytes consumed by the requests currently in the queue.
         */
        std::function<void(const Ice::BatchRequest& req, int count, int size)> batchRequestInterceptor{};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

        /**
         * The value factory manager.
         */
        ValueFactoryManagerPtr valueFactoryManager{};

        /**
         * The authentication options for %SSL client connections. When set, the %SSL transport ignores all IceSSL
         * configuration properties and uses the provided options.
         *
         * The `SSL::ClientAuthenticationOptions` type is an alias to the platform specific %SSL client authentication
         * options.
         *
         * @see SSL::OpenSSLClientAuthenticationOptions
         * @see SSL::SecureTransportClientAuthenticationOptions
         * @see SSL::SchannelClientAuthenticationOptions
         */
        std::optional<SSL::ClientAuthenticationOptions> clientAuthenticationOptions{};
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
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr
    initialize(int& argc, const char* argv[], const InitializationData& initData = InitializationData());

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
     * @return The new communicator.
     */
    inline CommunicatorPtr
    initialize(int& argc, char* argv[], const InitializationData& initData = InitializationData())
    {
        return initialize(argc, const_cast<const char**>(argv), initData);
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
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(int& argc, const char* argv[], std::string_view configFile);

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
     * @return The new communicator.
     */
    inline CommunicatorPtr initialize(int& argc, char* argv[], std::string_view configFile)
    {
        return initialize(argc, const_cast<const char**>(argv), configFile);
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
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr
    initialize(int& argc, const wchar_t* argv[], const InitializationData& initData = InitializationData());

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
     * @return The new communicator.
     */
    inline CommunicatorPtr
    initialize(int& argc, wchar_t* argv[], const InitializationData& initData = InitializationData())
    {
        return initialize(argc, const_cast<const wchar_t**>(argv), initData);
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
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(int& argc, const wchar_t* argv[], std::string_view configFile);

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
     * @return The new communicator.
     */
    inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], std::string_view configFile)
    {
        return initialize(argc, const_cast<const wchar_t**>(argv), configFile);
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
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(StringSeq& seq, const InitializationData& initData = InitializationData());

    /**
     * Initializes a new communicator.
     * @param seq Command-line arguments, possibly containing
     * options to set properties. If the arguments include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the arguments, the arguments take precedence.
     * Recognized options are removed from this container upon return.
     * @param configFile The name of an Ice configuration file.
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(StringSeq& seq, std::string_view configFile);

    /**
     * Initializes a new communicator.
     * @param initData Configuration data for the new Communicator.
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(const InitializationData& initData = InitializationData());

    /**
     * Initializes a new communicator.
     * @param configFile The name of an Ice configuration file.
     * @return The new communicator.
     */
    ICE_API CommunicatorPtr initialize(std::string_view configFile);

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
    using PluginFactory = Ice::Plugin* (*)(const Ice::CommunicatorPtr& communicator,
                                           const std::string& name,
                                           const Ice::StringSeq& args);

    /**
     * Manually registers a plug-in factory function.
     * @param name The name assigned to the plug-in.
     * @param factory The factory function.
     * @param loadOnInit If true, the plug-in is always loaded (created) during communicator initialization,
     * even if Ice.Plugin.name is not set. When false, the plug-in is loaded (created) during communication
     * initialization only if Ice.Plugin.name is set to a non-empty value (e.g.: Ice.Plugin.IceSSL=1).
     */
    ICE_API void registerPluginFactory(std::string name, PluginFactory factory, bool loadOnInit);

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

        /**
         * Calls initialize to create a communicator with the provided arguments.
         * This constructor accepts all of the same overloaded argument styles as
         * initialize.
         */
        template<class... T>
        explicit CommunicatorHolder(T&&... args) : _communicator(std::move(initialize(std::forward<T>(args)...)))
        {
        }

        /**
         * Adopts the given communicator.
         * @param communicator The new communicator instance to hold.
         */
        CommunicatorHolder(CommunicatorPtr communicator);

        /**
         * Adopts the given communicator. If this holder currently holds a communicator,
         * it will be destroyed.
         * @param communicator The new communicator instance to hold.
         */
        CommunicatorHolder& operator=(CommunicatorPtr communicator);

        CommunicatorHolder(const CommunicatorHolder&) = delete;
        CommunicatorHolder(CommunicatorHolder&&) = default;

        /**
         * Adopts the communicator in the given holder. If this holder currently holds a communicator,
         * it will be destroyed.
         * @param holder The holder from which to adopt a communicator.
         */
        CommunicatorHolder& operator=(CommunicatorHolder&& holder) noexcept;

        /**
         * Determines whether the holder contains an instance.
         * @return True if the holder currently holds an instance, false otherwise.
         */
        explicit operator bool() const;

        ~CommunicatorHolder();

        /**
         * Obtains the communicator instance.
         * @return The communicator held by this holder, or nil if the holder is empty.
         */
        [[nodiscard]] const CommunicatorPtr& communicator() const;

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
        CommunicatorPtr _communicator;
    };

    /**
     * Converts a stringified identity into an Identity.
     * @param str The stringified identity.
     * @return An Identity structure containing the name and category components.
     */
    ICE_API Identity stringToIdentity(std::string_view str);

    /**
     * Converts an Identity structure into a string using the specified mode.
     * @param id The identity structure.
     * @param mode Affects the handling of non-ASCII characters and non-printable ASCII characters.
     * @return The stringified identity.
     */
    ICE_API std::string identityToString(const Identity& id, ToStringMode mode = ToStringMode::Unicode);
}

namespace IceInternal
{
    ICE_API InstancePtr getInstance(const Ice::CommunicatorPtr&);
    ICE_API TimerPtr getInstanceTimer(const Ice::CommunicatorPtr&);
}

#endif
