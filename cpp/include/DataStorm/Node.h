// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_NODE_H
#define DATASTORM_NODE_H

#include "Config.h"
#include "InternalI.h"

#include <functional>

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

    /// Options to configure a DataStorm node.
    /// @headerfile DataStorm/DataStorm.h
    struct NodeOptions
    {
        /// The Ice communicator used by the node. If nullptr, the node creates its own communicator.
        /// @remark A DataStorm node requires the requests it receives on a connection to be dispatched in the order
        /// they were sent; a partial update is applied to the value left by the preceding sample. A node configures
        /// the communicators it creates accordingly, but it does not change the configuration of a communicator
        /// supplied here. Such a communicator must set `Ice.ThreadPool.Client.Serialize` to 1 before it is created.
        /// An executor set through Ice::InitializationData must likewise preserve the dispatch order.
        Ice::CommunicatorPtr communicator{nullptr};

        /// Specifies whether or not the node owns the communicator.
        /// This option is only meaningful when the communicator field is not nullptr. If true, the node's destructor
        /// destroys the communicator. The default value is false.
        bool nodeOwnsCommunicator{false};

        /// An optional executor used to execute user callbacks.
        /// If no callback executor is set, the node will use the default callback executor that executes callbacks
        /// in a dedicated thread.
        std::function<void(std::function<void()> call)> customExecutor{};

        /// The server authentication options used for SSL connections.
        /// When provided, these options are used to initialize the DataStorm.Node.Server object adapter.
        std::optional<Ice::SSL::ServerAuthenticationOptions> serverAuthenticationOptions{std::nullopt};
    };

    /// The Node class allows creating topic readers and writers.
    /// A node is the main DataStorm object which allows creating topic readers or writers.
    /// @headerfile DataStorm/DataStorm.h
    class DATASTORM_API Node
    {
    public:
        /// Constructs a DataStorm node.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param options The options to configure the node.
        /// @remark This is the main Node constructor. All other constructors are convenience constructors that call
        /// this constructor.
        explicit Node(NodeOptions options = {});

        /// Constructs a DataStorm node with the specified communicator.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param communicator The communicator used by the node. If nullptr, the node creates its own communicator.
        /// @remark This constructor sets the nodeOwnsCommunicator option to false. The communicator must be
        /// configured for ordered dispatch, as described in NodeOptions::communicator.
        explicit Node(Ice::CommunicatorPtr communicator);

        /// Constructs a DataStorm node with an Ice communicator initialized from command-line arguments.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @tparam ArgvT The type of the argument vector, such as char**, const char**, or wchar_t** (on Windows).
        /// @param argc The number of arguments in argv.
        /// @param argv The command-line arguments.
        template<typename ArgvT> Node(int& argc, ArgvT argv) : Node{createNodeOptions(argc, argv)} {}

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
        /// returned by DataStorm::Sample::getSession.
        /// @param ident The session identifier.
        /// @return The connection associated with the given session
        /// @see DataStorm::Sample::getSession
        [[nodiscard]] Ice::ConnectionPtr getSessionConnection(std::string_view ident) const noexcept;

    private:
        /// Returns the properties a node applies to the communicators it creates.
        static Ice::PropertiesPtr defaultProperties();

        template<typename ArgvT> NodeOptions createNodeOptions(int& argc, ArgvT argv)
        {
            // The node defaults have to be in place before the communicator is created: the client thread pool
            // reads its configuration when the communicator creates it.
            Ice::InitializationData initData;
            initData.properties = std::make_shared<Ice::Properties>(argc, argv, defaultProperties());

            NodeOptions options;
            options.communicator = Ice::initialize(std::move(initData));
            options.nodeOwnsCommunicator = true;
            return options;
        }

        std::shared_ptr<DataStormI::Instance> _instance;
        std::shared_ptr<DataStormI::TopicFactory> _factory;
        bool _ownsCommunicator{false};

        template<typename, typename, typename> friend class Topic;
    };
}
#endif
