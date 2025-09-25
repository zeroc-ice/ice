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
        Ice::CommunicatorPtr communicator{nullptr};

        /// Specifies whether or not the node owns the communicator.
        /// This option is only meaningful when the communicator field is not nullptr. If true, the node's destructor
        /// destroys the communicator. The default value is false.
        bool nodeOwnsCommunicator{false};

        /// An optional executor used to execute user callbacks.
        /// If no callback executor is set, the node will use the default callback executor that executes callbacks
        /// in a dedicated thread.
        std::function<void(std::function<void()> call)> customExecutor{};
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
        /// @remark This constructor sets the nodeOwnsCommunicator option to false.
        explicit Node(Ice::CommunicatorPtr communicator)
            : Node{NodeOptions{.communicator = std::move(communicator), .nodeOwnsCommunicator = false}}
        {
        }

        /// Constructs a DataStorm node with an Ice communicator initialized from command line arguments.
        /// A node is the main DataStorm object. It is required to construct topics.
        /// @param argc The number of arguments in argv.
        /// @param argv The configuration arguments.
        Node(int& argc, const char* argv[]);

        /// @copydoc Node(int&, const char*[])
        Node(int& argc, char* argv[]) : Node{argc, const_cast<const char**>(argv)} {}

#if defined(_WIN32) || defined(ICE_DOXYGEN)
        /// @copydoc Node(int&, const char*[])
        /// @remark Windows only.
        Node(int& argc, const wchar_t* argv[]);

        /// @copydoc Node(int&, const char*[])
        /// @remark Windows only.
        Node(int& argc, wchar_t* argv[]) : Node{argc, const_cast<const wchar_t**>(argv)} {}
#endif
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
        std::shared_ptr<DataStormI::Instance> _instance;
        std::shared_ptr<DataStormI::TopicFactory> _factory;
        bool _ownsCommunicator{false};

        template<typename, typename, typename> friend class Topic;
    };
}
#endif
