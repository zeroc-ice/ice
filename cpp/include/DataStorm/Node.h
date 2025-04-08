// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_NODE_H
#define DATASTORM_NODE_H

#include "Config.h"
#include "InternalI.h"

namespace DataStorm
{
    template<typename, typename, typename> class Topic;

    /// The exception that is thrown when a blocking operation is interrupted by the shutdown of the node.
    /// @headerfile DataStorm/DataStorm.h
    class DATASTORM_API NodeShutdownException final : public std::exception
    {
    public:
        /// Gets the error message of this exception.
        /// @return The error message.
        [[nodiscard]] const char* what() const noexcept final;
    };

    /// The Node class allows creating topic readers and writers.
    /// A Node is the main DataStorm object which allows creating topic readers or writers.
    /// @headerfile DataStorm/DataStorm.h
    class DATASTORM_API Node
    {
    public:
        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param argc The number of arguments in argv.
        /// @param argv The configuration arguments.
        /// @param configFile The path to an optional Ice configuration file.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        Node(
            int& argc,
            const char* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param argc The number of arguments in argv.
        /// @param argv The configuration arguments.
        /// @param configFile The path to an optional Ice configuration file.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        Node(
            int& argc,
            char* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr)
            : Node(argc, const_cast<const char**>(argv), configFile, std::move(customExecutor))
        {
        }

#if defined(_WIN32) || defined(ICE_DOXYGEN)
        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param argc The number of arguments in argv.
        /// @param argv The configuration arguments.
        /// @param configFile The path to an optional Ice configuration file.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        /// @remark Windows only.
        Node(
            int& argc,
            const wchar_t* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param argc The number of arguments in argv.
        /// @param argv The configuration arguments.
        /// @param configFile The path to an optional Ice configuration file.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        //// @remark Windows only.
        Node(
            int& argc,
            wchar_t* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr)
            : Node(argc, const_cast<const wchar_t**>(argv), configFile, customExecutor)
        {
        }
#endif

        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param configFile The path to an optional Ice configuration file.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        Node(
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics. The node uses the given Ice
        /// communicator.
        /// @param communicator The Ice communicator used by the topic factory for its configuration and communications.
        /// This communicator must be initialized with a property set to use the "DataStorm" opt-in prefix.
        /// @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
        /// provided the Node will use the default callback executor that executes callback in a dedicated thread.
        Node(
            Ice::CommunicatorPtr communicator,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /// Move constructor.
        /// @param node The node to move from.
        Node(Node&& node) noexcept;

        /// Destructor.
        /// The node destruction releases associated resources. If the node created the Ice communicator, the
        /// communicator is destroyed.
        ~Node();

        /// Shuts down the node. The shutdown interrupts calls which are waiting for events, writers or readers.
        void shutdown() noexcept;

        /// Returns whether or not the node shutdown has been initiated.
        /// @return `true` if the node is shutdown, `false` otherwise.
        [[nodiscard]] bool isShutdown() const noexcept;

        /// Waits for shutdown to be called.
        void waitForShutdown() const noexcept;

        /// Move assignment operator.
        /// @param node The node to move from.
        /// @return A reference to this node.
        Node& operator=(Node&& node) noexcept;

        /// Returns the Ice communicator associated with the node.
        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const noexcept;

        /// Returns the Ice connection associated with a session given a session identifier. Session identifiers are
        /// provided with the sample origin data member as the first tuple element.
        /// @param ident The session identifier.
        /// @return The connection associated with the given session
        /// @see DataStorm::Sample::ElementId DataStorm::Sample::getSession
        [[nodiscard]] Ice::ConnectionPtr getSessionConnection(std::string_view ident) const noexcept;

    private:
        Node(
            Ice::CommunicatorPtr,
            std::function<void(std::function<void()> call)> customExecutor,
            bool ownsCommunicator);

        std::shared_ptr<DataStormI::Instance> _instance;
        std::shared_ptr<DataStormI::TopicFactory> _factory;
        bool _ownsCommunicator;

        template<typename, typename, typename> friend class Topic;
    };
}
#endif
