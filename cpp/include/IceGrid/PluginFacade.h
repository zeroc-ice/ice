// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_PLUGIN_FACADE_H
#define ICEGRID_PLUGIN_FACADE_H

#include "Ice/Ice.h"
#include "IceGrid/Admin.h"

namespace IceGrid
{
    /// The ReplicaGroupFilter is used by IceGrid to filter adapters returned to the client when it resolves a filtered
    /// replica group.
    /// IceGrid provides the list of available adapters. The implementation of this method can use the provided context
    /// and connection to filter and return the filtered set of adapters.
    /// @headerfile IceGrid/IceGrid.h
    class ReplicaGroupFilter
    {
    public:
        virtual ~ReplicaGroupFilter() = default;

        /// Filters adapter IDs.
        /// @param replicaGroupId The replica group ID.
        /// @param adapterIds The adapter IDs to filter.
        /// @param con The connection from the Ice client which is resolving the replica group endpoints.
        /// @param ctx The request context from the Ice client which is resolving the replica group endpoints.
        /// @return The filtered adapter IDs.
        virtual Ice::StringSeq filter(
            const std::string& replicaGroupId,
            const Ice::StringSeq& adapterIds,
            const Ice::ConnectionPtr& con,
            const Ice::Context& ctx) = 0;
    };

    /// The TypeFilter is used by IceGrid to filter well-known proxies returned to the client when it searches a
    /// well-known object by type. IceGrid provides the list of available proxies. The implementation of this method can
    /// use the provided context and connection to filter and return the filtered set of proxies.
    /// @headerfile IceGrid/IceGrid.h
    class TypeFilter
    {
    public:
        virtual ~TypeFilter() = default;

        /// Filters the given set of proxies.
        /// @param type The type.
        /// @param proxies The proxies to filter.
        /// @param con The connection from the Ice client which is looking up well-known objects by type.
        /// @param ctx The context from the Ice client which is looking up well-known objects by type.
        /// @return The filtered proxies.
        virtual Ice::ObjectProxySeq filter(
            const std::string& type,
            const Ice::ObjectProxySeq& proxies,
            const Ice::ConnectionPtr& con,
            const Ice::Context& ctx) = 0;
    };

    /// The RegistryPluginFacade is implemented by IceGrid and can be used by plugins and filter implementations to
    /// retrieve information from IceGrid about the well-known objects or adapters. It's also used to
    /// register/unregister replica group and type filters.
    /// @headerfile IceGrid/IceGrid.h
    class RegistryPluginFacade
    {
    public:
        virtual ~RegistryPluginFacade();

        /// Gets an application descriptor.
        /// @param name The application name.
        /// @return The application descriptor.
        /// @throws IceGrid::ApplicationNotExistException Thrown when the application doesn't exist.
        [[nodiscard]] virtual ApplicationInfo getApplicationInfo(const std::string& name) const = 0;

        /// Gets the server information for the server with the given id.
        /// @param id The server id.
        /// @return The server information.
        /// @throws IceGrid::ServerNotExistException Thrown when the server doesn't exist.
        [[nodiscard]] virtual ServerInfo getServerInfo(const std::string& id) const = 0;

        /// Gets the ID of the server to which the given adapter belongs.
        /// @param adapterId The adapter ID.
        /// @return The server ID or the empty string if the given identifier is not associated to an object adapter
        /// defined with an application descriptor.
        /// @throws IceGrid::AdapterNotExistException Thrown when the adapter doesn't exist.
        [[nodiscard]] virtual std::string getAdapterServer(const std::string& adapterId) const = 0;

        /// Gets the name of the application to which the given adapter belongs.
        /// @param adapterId The adapter ID.
        /// @return The application name or the empty string if the given identifier is not associated to a replica
        /// group or object adapter defined with an application descriptor.
        /// @throws IceGrid::AdapterNotExistException Thrown when the adapter doesn't exist.
        [[nodiscard]] virtual std::string getAdapterApplication(const std::string& adapterId) const = 0;

        /// Gets the name of the node to which the given adapter belongs.
        /// @param adapterId The adapter ID.
        /// @return The node name or the empty string if the given identifier is not associated to an object adapter
        /// defined with an application descriptor.
        /// @throws IceGrid::AdapterNotExistException Thrown when the adapter doesn't exist.
        [[nodiscard]] virtual std::string getAdapterNode(const std::string& adapterId) const = 0;

        /// Gets the adapter information for the replica group or adapter with the given id.
        /// @param id The adapter id.
        /// @return A sequence of adapter information structures. If the given id refers to an adapter, this sequence
        /// will contain only one element. If the given id refers to a replica group, the sequence will contain the
        /// adapter information of each member of the replica group.
        /// @throws IceGrid::AdapterNotExistException Thrown when the adapter or replica group doesn't exist.
        [[nodiscard]] virtual AdapterInfoSeq getAdapterInfo(const std::string& id) const = 0;

        /// Gets the object info for the object with the given identity.
        /// @param id The identity of the object.
        /// @return The object info.
        /// @throws IceGrid::ObjectNotRegisteredException Thrown when the object isn't registered with the registry.
        [[nodiscard]] virtual ObjectInfo getObjectInfo(const Ice::Identity& id) const = 0;

        /// Gets the node information for the node with the given name.
        /// @param name The node name.
        /// @return The node information.
        /// @throws IceGrid::NodeNotExistException Thrown when the node doesn't exist.
        /// @throws IceGrid::NodeUnreachableException Thrown when the node could not be reached.
        [[nodiscard]] virtual NodeInfo getNodeInfo(const std::string& name) const = 0;

        /// Gets the load averages of the node.
        /// @param name The node name.
        /// @return The node load information.
        /// @throws IceGrid::NodeNotExistException Thrown when the node doesn't exist.
        /// @throws IceGrid::NodeUnreachableException Thrown when the node could not be reached.
        [[nodiscard]] virtual LoadInfo getNodeLoad(const std::string& name) const = 0;

        /// Gets the property value for the given property and adapter. The property is looked up in the server or
        /// service descriptor where the adapter is defined.
        /// @param adapterId The adapter ID
        /// @param name The name of the property.
        /// @return The property value.
        /// @throws IceGrid::AdapterNotExistException Thrown when the adapter doesn't exist.
        [[nodiscard]] virtual std::string
        getPropertyForAdapter(const std::string& adapterId, const std::string& name) const = 0;

        /// Adds a replica group filter.
        /// @param id The identifier of the filter. This identifier must match the value of the "filter" attribute
        /// specified in the replica group descriptor. To filter dynamically registered replica groups, you should use
        /// the empty filter id.
        /// @param filter The filter implementation.
        virtual void
        addReplicaGroupFilter(const std::string& id, const std::shared_ptr<ReplicaGroupFilter>& filter) noexcept = 0;

        /// Removes a replica group filter.
        /// @param id The identifier of the filter.
        /// @param filter The filter implementation.
        /// @return `true` of the filter was removed, `false` otherwise.
        virtual bool
        removeReplicaGroupFilter(const std::string& id, const std::shared_ptr<ReplicaGroupFilter>& filter) noexcept = 0;

        /// Adds a type filter.
        /// @param type The type to register this filter with.
        /// @param filter The filter implementation.
        virtual void addTypeFilter(const std::string& type, const std::shared_ptr<TypeFilter>& filter) noexcept = 0;

        /// Removes a type filter.
        /// @param type The type to register this filter with.
        /// @param filter The filter implementation.
        /// @return `true` of the filter was removed, `false` otherwise.
        virtual bool removeTypeFilter(const std::string& type, const std::shared_ptr<TypeFilter>& filter) noexcept = 0;
    };

    /// A shared pointer to a ReplicaGroupFilter.
    using ReplicaGroupFilterPtr = std::shared_ptr<ReplicaGroupFilter>;

    /// A shared pointer to a TypeFilter.
    using TypeFilterPtr = std::shared_ptr<TypeFilter>;

    /// A shared pointer to a RegistryPluginFacade.
    using RegistryPluginFacadePtr = std::shared_ptr<RegistryPluginFacade>;

    /// Gets the plug-in facade for the IceGrid registry.
    /// @return The plug-in facade.
    ICEGRID_API RegistryPluginFacadePtr getRegistryPluginFacade();
}

#endif
