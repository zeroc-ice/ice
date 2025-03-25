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
    /// Converts an argument vector into a string sequence.
    /// @param argc The number of arguments in argv.
    /// @param argv The arguments.
    /// @return A string sequence containing the arguments.
    ICE_API StringSeq argsToStringSeq(int argc, const char* const argv[]);

#ifdef _WIN32
    /// @copydoc argsToStringSeq(int, const char* const[])
    ICE_API StringSeq argsToStringSeq(int argc, const wchar_t* const argv[]);
#endif

    /// Updates @p argv to match the contents of @p seq. This function assumes that @p seq contains only elements of
    /// @p argv. The function shifts the argument vector elements so that the vector matches the contents of the
    /// sequence.
    /// @param seq The string sequence returned from a call to #argsToStringSeq.
    /// @param[in,out] argc The number of arguments, updated to reflect the size of the sequence.
    /// @param argv The arguments, shifted to match @p seq.
    ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const char* argv[]);

    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    inline void stringSeqToArgs(const StringSeq& seq, int& argc, char* argv[])
    {
        return stringSeqToArgs(seq, argc, const_cast<const char**>(argv));
    }

#ifdef _WIN32
    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const wchar_t* argv[]);

    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    inline void stringSeqToArgs(const StringSeq& seq, int& argc, wchar_t* argv[])
    {
        return stringSeqToArgs(seq, argc, const_cast<const wchar_t**>(argv));
    }
#endif

    /// A special plug-in that installs a thread hook during a communicator's initialization.
    /// Both initialize and destroy are no-op. See InitializationData.
    /// @headerfile Ice/Ice.h
    class ICE_API ThreadHookPlugin : public Ice::Plugin
    {
    public:
        /// Installs the thread hooks.
        /// @param communicator The communicator in which to install the thread hooks.
        /// @param start The start callback.
        /// @param stop The stop callback.
        ThreadHookPlugin(const CommunicatorPtr& communicator, std::function<void()> start, std::function<void()> stop);

        void initialize() override;

        void destroy() override;
    };

    /// Represents a set of options that you can specify when initializing a communicator.
    /// @headerfile Ice/Ice.h
    struct InitializationData
    {
        /// The properties for the communicator.
        PropertiesPtr properties{};

        /// The logger for the communicator.
        LoggerPtr logger{};

        /// The communicator observer used by the Ice runtime.
        Instrumentation::CommunicatorObserverPtr observer{};

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdocumentation" // param/return is not recognized for std::function data members
#endif

        /// A function that the communicator calls when it starts a new thread.
        std::function<void()> threadStart{};

        /// A function that the communicator calls when it destroys a thread.
        std::function<void()> threadStop{};

        /// A function that the communicator calls to execute dispatches and async invocation callbacks.
        /// @param call Represents the function to execute. The executor must eventually execute this function.
        /// @param con The connection associated with this call. May be null.
        std::function<void(std::function<void()> call, const Ice::ConnectionPtr& con)> executor{};

        /// @private
        std::function<std::string(int id)> compactIdResolver{};

        /// The batch request interceptor, which is called by the Ice runtime to enqueue a batch request.
        /// @param req An object representing the batch request.
        /// @param count The number of requests currently in the queue.
        /// @param size The number of bytes consumed by the requests currently in the queue.
        std::function<void(const Ice::BatchRequest& req, int count, int size)> batchRequestInterceptor{};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

        /// The value factory manager.
        ValueFactoryManagerPtr valueFactoryManager{};

        /// The authentication options for %SSL client connections. When set, the %SSL transport ignores all IceSSL
        /// configuration properties and uses these options. The `SSL::ClientAuthenticationOptions` type is an alias for
        /// the platform specific %SSL client authentication options.
        /// @see SSL::OpenSSLClientAuthenticationOptions
        /// @see SSL::SecureTransportClientAuthenticationOptions
        /// @see SSL::SchannelClientAuthenticationOptions
        std::optional<SSL::ClientAuthenticationOptions> clientAuthenticationOptions{};
    };

    /// @name Communicator
    /// Communicator initialization functions.
    /// @{

    /// Creates a new communicator.
    /// @param[in,out] argc The number of arguments in @p argv. When this function parses properties from @p argv, it
    /// reshuffles the arguments so that the remaining arguments start at the beginning of @p argv, and updates @p argc
    /// accordingly.
    /// @param argv The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL etc.) as properties for the new communicator. If there is an argument starting
    /// with `--Ice.Config`, this function loads the specified configuration file. When the same property is set in a
    /// configuration file and through a command-line argument, the command-line setting takes precedence.
    /// @param initData Options for the new communicator.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(int& argc, const char* argv[], InitializationData initData = {});

    /// @copydoc initialize(int&, const char*[], InitializationData)
    inline CommunicatorPtr initialize(int& argc, char* argv[], InitializationData initData = {})
    {
        return initialize(argc, const_cast<const char**>(argv), std::move(initData));
    }

    /// Creates a new communicator.
    /// @param[in,out] argc The number of arguments in @p argv. When this function parses properties from @p argv, it
    /// reshuffles the arguments so that the remaining arguments start at the beginning of @p argv, and updates @p argc.
    /// @param argv The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator.
    /// @param configFile The name of an Ice configuration file.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(int& argc, const char* argv[], std::string_view configFile);

    /// @copydoc initialize(int&, const char*[], std::string_view)
    inline CommunicatorPtr initialize(int& argc, char* argv[], std::string_view configFile)
    {
        return initialize(argc, const_cast<const char**>(argv), configFile);
    }

#ifdef _WIN32
    /// @copydoc initialize(int&, const char*[], InitializationData)
    ICE_API CommunicatorPtr initialize(int& argc, const wchar_t* argv[], InitializationData initData = {});

    /// @copydoc initialize(int&, const char*[], InitializationData)
    inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], InitializationData initData = {})
    {
        return initialize(argc, const_cast<const wchar_t**>(argv), std::move(initData));
    }

    // @copydoc initialize(int&, const char*[], std::string_view)
    ICE_API CommunicatorPtr initialize(int& argc, const wchar_t* argv[], std::string_view configFile);

    // @copydoc initialize(int&, const char*[], std::string_view)
    inline CommunicatorPtr initialize(int& argc, wchar_t* argv[], std::string_view configFile)
    {
        return initialize(argc, const_cast<const wchar_t**>(argv), configFile);
    }
#endif

    /// Creates a new communicator.
    /// @param[in,out] seq The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator and removes these elements from the
    /// list. If there is an argument starting with `--Ice.Config`, this function loads the specified configuration
    /// file. When the same property is set in a configuration file and through a command-line argument, the
    /// command-line setting takes precedence.
    /// @param initData Options for the new communicator.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(StringSeq& seq, InitializationData initData = {});

    /// Creates a new communicator.
    /// @param[in,out] seq The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator and removes these elements from the
    /// list.
    /// @param configFile The name of an Ice configuration file.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(StringSeq& seq, std::string_view configFile);

    /// Creates a new communicator.
    /// @param initData Options for the new communicator.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(InitializationData initData = {});

    /// Creates a new communicator.
    /// @param configFile The name of an Ice configuration file.
    /// @return The new communicator.
    ICE_API CommunicatorPtr initialize(std::string_view configFile);

    /// @}

    /// Gets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    /// configured at the time the communicator is created.
    /// @return The current per-process logger instance.
    ICE_API LoggerPtr getProcessLogger();

    /// Sets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    /// configured at the time the communicator is created.
    /// @param logger The new per-process logger instance.
    ICE_API void setProcessLogger(const LoggerPtr& logger);

    /// A plug-in factory function is responsible for creating an Ice plug-in.
    /// @param communicator The communicator in which the plug-in will be installed.
    /// @param name The name assigned to the plug-in.
    /// @param args Additional arguments included in the plug-in's configuration.
    /// @return The new plug-in object.
    using PluginFactory = Ice::Plugin* (*)(const Ice::CommunicatorPtr& communicator,
                                           const std::string& name,
                                           const Ice::StringSeq& args);

    /// Registers a plug-in factory function.
    /// @param name The name assigned to the plug-in.
    /// @param factory The factory function.
    /// @param loadOnInit If `true`, the plug-in is always loaded (created) during communicator initialization, even if
    /// `Ice.Plugin.name` is not set. When `false`, the plug-in is loaded (created) during communicator initialization
    /// only when `Ice.Plugin.name` is set to a non-empty value.
    ICE_API void registerPluginFactory(std::string name, PluginFactory factory, bool loadOnInit);

    /// Converts a stringified identity into an Identity.
    /// @param str The stringified identity.
    /// @return An Identity containing the name and category components.
    ICE_API Identity stringToIdentity(std::string_view str);

    /// Converts an Identity into a string using the specified mode.
    /// @param id The identity.
    /// @param mode Specifies how to handle non-ASCII characters and non-printable ASCII characters.
    /// @return The stringified identity.
    ICE_API std::string identityToString(const Identity& id, ToStringMode mode = ToStringMode::Unicode);
}

namespace IceInternal
{
    ICE_API InstancePtr getInstance(const Ice::CommunicatorPtr&);
    ICE_API TimerPtr getInstanceTimer(const Ice::CommunicatorPtr&);
}

#endif
