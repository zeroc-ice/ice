//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_NODE_H
#define DATASTORM_NODE_H

#include "Config.h"
#include "InternalI.h"

namespace DataStorm
{
    template<typename, typename, typename> class Topic;

    /**
     * The exception NodeShutdownException. It's raised by methods which might block waiting for a condition to
     * occur and after the node has been shutdown. It informs the application that the condition won't occur
     * because the DataStorm node is being shutdown and will disconnect from other nodes.
     *
     * @headerfile DataStorm/DataStorm.h
     */
    class DATASTORM_API NodeShutdownException : public std::exception
    {
    public:
        virtual const char* what() const noexcept;
    };

    /**
     * The Node class allows creating topic readers and writers.
     *
     * A Node is the main DataStorm object which allows creating topic readers or writers.
     *
     * @headerfile DataStorm/DataStorm.h
     */
    class DATASTORM_API Node
    {
    public:
        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics.
         *
         * @param argc The number of arguments in argv.
         * @param argv The configuration arguments.
         * @param configFile The path to an optional Ice configuration file.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         *
         */
        Node(
            int& argc,
            const char* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics.
         *
         * @param argc The number of arguments in argv.
         * @param argv The configuration arguments.
         * @param configFile The path to an optional Ice configuration file.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         *
         */
        Node(
            int& argc,
            char* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr)
            : Node(argc, const_cast<const char**>(argv), configFile, customExecutor)
        {
        }

#ifdef _WIN32
        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics.
         *
         * @param argc The number of arguments in argv.
         * @param argv The configuration arguments.
         * @param configFile The path to an optional Ice configuration file.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         *
         */
        Node(
            int& argc,
            const wchar_t* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics.
         *
         * @param argc The number of arguments in argv.
         * @param argv The configuration arguments.
         * @param configFile The path to an optional Ice configuration file.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         *
         */
        Node(
            int& argc,
            wchar_t* argv[],
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr)
            : Node(argc, const_cast<const wchar_t**>(argv), configFile, customExecutor)
        {
        }
#endif

        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics.
         *
         * @param configFile The path to an optional Ice configuration file.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         */
        Node(
            std::optional<std::string_view> configFile = std::nullopt,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /**
         * Construct a DataStorm node.
         *
         * A node is the main DataStorm object. It is required to construct topics. The node uses the given Ice
         * communicator.
         *
         * @param communicator The Ice communicator used by the topic factory for its configuration and communications.
         * This communicator must be initialized with a property set to use the "DataStorm" opt-in prefix.
         * @param customExecutor An optional executor used to execute user callbacks, if no callback executor is
         * provided the Node will use the default callback executor that executes callback in a dedicated thread.
         */
        Node(
            Ice::CommunicatorPtr communicator,
            std::function<void(std::function<void()> call)> customExecutor = nullptr);

        /**
         * Construct a new Node by taking ownership of the given node.
         *
         * @param node The node to transfer ownership from.
         */
        Node(Node&& node) noexcept;

        /**
         * Node destructor. The node destruction releases associated resources. If the node created the Ice
         * communicator, the communicator is destroyed.
         */
        ~Node();

        /**
         * Shutdown the node. The shutdown interrupts calls which are waiting for events, writers or readers.
         **/
        void shutdown() noexcept;

        /**
         * Return whether or not the node shutdown has been initiated.
         *
         * @return True if the node is shutdown, false otherwise.
         */
        bool isShutdown() const noexcept;

        /**
         * Wait for shutdown to be called.
         */
        void waitForShutdown() const noexcept;

        /**
         * Move assignment operator.
         *
         * @param node The node.
         **/
        Node& operator=(Node&& node) noexcept;

        /**
         * Returns the Ice communicator associated with the node.
         */
        Ice::CommunicatorPtr getCommunicator() const noexcept;

        /**
         * Returns the Ice connection associated with a session given a session identifier. Session identifiers are
         * provided with the sample origin data member as the first tuple element.
         *
         * @param ident The session identifier.
         * @return The connection associated with the given session
         * @see DataStorm::Sample::ElementId DataStorm::Sample::getSession
         */
        Ice::ConnectionPtr getSessionConnection(std::string_view ident) const noexcept;

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
