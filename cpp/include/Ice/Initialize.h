// Copyright (c) ZeroC, Inc.

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include "BatchRequest.h"
#include "CommunicatorF.h"
#include "Connection.h"
#include "Ice/BuiltinSequences.h"
#include "InstanceF.h"
#include "Instrumentation.h"
#include "Logger.h"
#include "Plugin.h"
#include "PluginFactory.h"
#include "Properties.h"
#include "SSL/ClientAuthenticationOptions.h"
#include "SliceLoader.h"
#include "StringUtil.h"
#include "TimerTask.h"

#include <algorithm>

namespace Ice
{
    /// Represents a set of options that you can specify when initializing a communicator.
    /// @headerfile Ice/Ice.h
    struct InitializationData
    {
        /// The properties for the communicator.
        /// When not-null, this corresponds to the object returned by the Communicator::getProperties function.
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

        /// The batch request interceptor, which is called by the Ice runtime to enqueue a batch request.
        /// @param req An object representing the batch request.
        /// @param count The number of requests currently in the queue.
        /// @param size The number of bytes consumed by the requests currently in the queue.
        std::function<void(const Ice::BatchRequest& req, int count, int size)> batchRequestInterceptor{};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

        /// The authentication options for SSL client connections. When set, the SSL transport ignores all IceSSL
        /// configuration properties and uses these options.
        /// @see SSL::OpenSSLClientAuthenticationOptions
        /// @see SSL::SecureTransportClientAuthenticationOptions
        /// @see SSL::SchannelClientAuthenticationOptions
        std::optional<SSL::ClientAuthenticationOptions> clientAuthenticationOptions{};

        /// A list of plug-in factories. The corresponding plug-ins are created during communicator initialization,
        /// in order, before all other plug-ins.
        std::vector<PluginFactory> pluginFactories{};

        /// The Slice loader, used to unmarshal Slice classes and exceptions.
        SliceLoaderPtr sliceLoader{};
    };

    /// Creates a new communicator.
    /// @param initData Options for the new communicator.
    /// @return The new communicator.
    /// @remark This is the main initialize function. All other overloads are provided for convenience and call this
    /// function.
    ICE_API CommunicatorPtr initialize(InitializationData initData = {});

    /// Creates a new communicator, using Ice properties parsed from the command-line arguments.
    /// @tparam ArgvT The type of the argument vector, such as char**, const char**, or wchar_t** (on Windows).
    /// @param[in,out] argc The number of arguments in @p argv. When this function parses properties from @p argv, it
    /// reshuffles the arguments so that the remaining arguments start at the beginning of @p argv, and updates @p argc
    /// accordingly.
    /// @param argv The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator. If there is an argument starting
    /// with `--Ice.Config`, this function loads the specified configuration file. When the same property is set in a
    /// configuration file and through a command-line argument, the command-line setting takes precedence.
    /// @return The new communicator.
    template<typename ArgvT> inline CommunicatorPtr initialize(int& argc, ArgvT argv)
    {
        auto properties = std::make_shared<Properties>(argc, argv);
        if (properties->getProperty("Ice.ProgramName").empty() && argc > 0)
        {
            StringSeq args = argsToStringSeq(argc, argv);
            std::string programName = args[0];
            // Replace any backslashes in this value with forward slashes, in case this value is used by the event
            // logger.
            std::replace(programName.begin(), programName.end(), '\\', '/');
            properties->setProperty("Ice.ProgramName", std::move(programName));
        }

        InitializationData initData;
        initData.properties = properties;
        return initialize(std::move(initData));
    }

    /// Gets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    /// configured at the time the communicator is created.
    /// @return The current per-process logger instance.
    ICE_API LoggerPtr getProcessLogger();

    /// Sets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    /// configured at the time the communicator is created.
    /// @param logger The new per-process logger instance.
    ICE_API void setProcessLogger(const LoggerPtr& logger);

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
