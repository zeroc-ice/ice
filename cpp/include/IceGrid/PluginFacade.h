//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_PLUGIN_FACADE_H
#define ICEGRID_PLUGIN_FACADE_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/GCObject.h>
#include <Ice/Value.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/FactoryTableInit.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>
#include <Ice/ExceptionHelpers.h>
#include <Ice/SlicedDataF.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Current.h>
#include <IceGrid/Admin.h>
#include <IceUtil/UndefSysMacros.h>
#include <IceGrid/Config.h>

#ifndef ICEGRID_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEGRID_API /**/
#   elif defined(ICEGRID_API_EXPORTS)
#       define ICEGRID_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEGRID_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace IceGrid
{

class ReplicaGroupFilter;
class TypeFilter;
class RegistryPluginFacade;

}

namespace IceGrid
{

/**
 * The ReplicaGroupFilter is used by IceGrid to filter adapters
 * returned to the client when it resolves a filtered replica group.
 *
 * IceGrid provides the list of available adapters. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of adapters.
 * \headerfile IceGrid/IceGrid.h
 */
class ICE_CLASS(ICEGRID_API) ReplicaGroupFilter
{
public:

    ICE_MEMBER(ICEGRID_API) virtual ~ReplicaGroupFilter();

    /**
     * Filter the the given set of adapters.
     * @param replicaGroupId The replica group ID.
     * @param adapterIds The adpater IDs to filter.
     * @param con The connection from the Ice client which is
     * resolving the replica group endpoints.
     * @param ctx The context from the Ice client which is resolving
     * the replica group endpoints.
     * @return The filtered adapter IDs.
     */
    virtual ::Ice::StringSeq filter(const ::std::string& replicaGroupId, const ::Ice::StringSeq& adapterIds, const ::std::shared_ptr<::Ice::Connection>& con, const ::Ice::Context& ctx) = 0;
};

/**
 * The TypeFilter is used by IceGrid to filter well-known proxies
 * returned to the client when it searches a well-known object by
 * type.
 *
 * IceGrid provides the list of available proxies. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of proxies.
 * \headerfile IceGrid/IceGrid.h
 */
class ICE_CLASS(ICEGRID_API) TypeFilter
{
public:

    ICE_MEMBER(ICEGRID_API) virtual ~TypeFilter();

    /**
     * Filter the the given set of proxies.
     * @param type The type.
     * @param proxies The proxies to filter.
     * @param con The connection from the Ice client which is
     * looking up well-known objects by type.
     * @param ctx The context from the Ice client which is looking up
     * well-known objects by type.
     * @return The filtered proxies.
     */
    virtual ::Ice::ObjectProxySeq filter(const ::std::string& type, const ::Ice::ObjectProxySeq& proxies, const ::std::shared_ptr<::Ice::Connection>& con, const ::Ice::Context& ctx) = 0;
};

/**
 * The RegistryPluginFacade is implemented by IceGrid and can be used
 * by plugins and filter implementations to retrieve information from
 * IceGrid about the well-known objects or adapters. It's also used to
 * register/unregister replica group and type filters.
 * \headerfile IceGrid/IceGrid.h
 */
class ICE_CLASS(ICEGRID_API) RegistryPluginFacade
{
public:

    ICE_MEMBER(ICEGRID_API) virtual ~RegistryPluginFacade();

    /**
     * Get an application descriptor.
     * @param name The application name.
     * @return The application descriptor.
     * @throws IceGrid::ApplicationNotExistException Raised if the application
     * doesn't exist.
     */
    virtual ::IceGrid::ApplicationInfo getApplicationInfo(const ::std::string& name) const = 0;

    /**
     * Get the server information for the server with the given id.
     * @param id The server id.
     * @return The server information.
     * @throws IceGrid::ServerNotExistException Raised if the server doesn't exist.
     */
    virtual ::IceGrid::ServerInfo getServerInfo(const ::std::string& id) const = 0;

    /**
     * Get the ID of the server to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The server ID or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterServer(const ::std::string& adapterId) const = 0;

    /**
     * Get the name of the application to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The application name or the empty string if the given
     * identifier is not associated to a replica group or object
     * adapter defined with an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterApplication(const ::std::string& adapterId) const = 0;

    /**
     * Get the name of the node to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The node name or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterNode(const ::std::string& adapterId) const = 0;

    /**
     * Get the adapter information for the replica group or adapter
     * with the given id.
     * @param id The adapter id.
     * @return A sequence of adapter information structures. If the
     * given id refers to an adapter, this sequence will contain only
     * one element. If the given id refers to a replica group, the
     * sequence will contain the adapter information of each member of
     * the replica group.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter or
     * replica group doesn't exist.
     */
    virtual ::IceGrid::AdapterInfoSeq getAdapterInfo(const ::std::string& id) const = 0;

    /**
     * Get the object info for the object with the given identity.
     * @param id The identity of the object.
     * @return The object info.
     * @throws IceGrid::ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     */
    virtual ::IceGrid::ObjectInfo getObjectInfo(const ::Ice::Identity& id) const = 0;

    /**
     * Get the node information for the node with the given name.
     * @param name The node name.
     * @return The node information.
     * @throws IceGrid::NodeNotExistException Raised if the node doesn't exist.
     * @throws IceGrid::NodeUnreachableException Raised if the node could not be
     * reached.
     */
    virtual ::IceGrid::NodeInfo getNodeInfo(const ::std::string& name) const = 0;

    /**
     * Get the load averages of the node.
     * @param name The node name.
     * @return The node load information.
     * @throws IceGrid::NodeNotExistException Raised if the node doesn't exist.
     * @throws IceGrid::NodeUnreachableException Raised if the node could not be
     * reached.
     */
    virtual ::IceGrid::LoadInfo getNodeLoad(const ::std::string& name) const = 0;

    /**
     * Get the property value for the given property and adapter. The
     * property is looked up in the server or service descriptor where
     * the adapter is defined.
     * @param adapterId The adapter ID
     * @param name The name of the property.
     * @return The property value.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't exist.
     */
    virtual ::std::string getPropertyForAdapter(const ::std::string& adapterId, const ::std::string& name) const = 0;

    /**
     * Add a replica group filter.
     * @param id The identifier of the filter. This identifier must
     * match the value of the "filter" attribute specified in the
     * replica group descriptor. To filter dynamically registered
     * replica groups, you should use the empty filter id.
     * @param filter The filter implementation.
     */
    virtual void addReplicaGroupFilter(const ::std::string& id, const ::std::shared_ptr<ReplicaGroupFilter>& filter) noexcept = 0;

    /**
     * Remove a replica group filter.
     * @param id The identifier of the filter.
     * @param filter The filter implementation.
     * @return True of the filter was removed, false otherwise.
     */
    virtual bool removeReplicaGroupFilter(const ::std::string& id, const ::std::shared_ptr<ReplicaGroupFilter>& filter) noexcept = 0;

    /**
     * Add a type filter.
     * @param type The type to register this filter with.
     * @param filter The filter implementation.
     */
    virtual void addTypeFilter(const ::std::string& type, const ::std::shared_ptr<TypeFilter>& filter) noexcept = 0;

    /**
     * Remove a type filter.
     * @param type The type to register this filter with.
     * @param filter The filter implementation.
     * @return True of the filter was removed, false otherwise.
     */
    virtual bool removeTypeFilter(const ::std::string& type, const ::std::shared_ptr<TypeFilter>& filter) noexcept = 0;
};

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace IceGrid
{

using ReplicaGroupFilterPtr = ::std::shared_ptr<ReplicaGroupFilter>;

using TypeFilterPtr = ::std::shared_ptr<TypeFilter>;

using RegistryPluginFacadePtr = ::std::shared_ptr<RegistryPluginFacade>;

}
/// \endcond

#else // C++98 mapping

namespace IceProxy
{

}

namespace IceGrid
{

class ReplicaGroupFilter;
/// \cond INTERNAL
ICEGRID_API ::Ice::LocalObject* upCast(ReplicaGroupFilter*);
/// \endcond
typedef ::IceInternal::Handle< ReplicaGroupFilter> ReplicaGroupFilterPtr;

class TypeFilter;
/// \cond INTERNAL
ICEGRID_API ::Ice::LocalObject* upCast(TypeFilter*);
/// \endcond
typedef ::IceInternal::Handle< TypeFilter> TypeFilterPtr;

class RegistryPluginFacade;
/// \cond INTERNAL
ICEGRID_API ::Ice::LocalObject* upCast(RegistryPluginFacade*);
/// \endcond
typedef ::IceInternal::Handle< RegistryPluginFacade> RegistryPluginFacadePtr;

}

/// \cond INTERNAL
namespace IceAsync
{

}
/// \endcond

namespace IceProxy
{

}

namespace IceGrid
{

/**
 * The ReplicaGroupFilter is used by IceGrid to filter adapters
 * returned to the client when it resolves a filtered replica group.
 *
 * IceGrid provides the list of available adapters. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of adapters.
 * \headerfile IceGrid/IceGrid.h
 */
class ICEGRID_API ReplicaGroupFilter : public virtual ::Ice::LocalObject
{
public:

    typedef ReplicaGroupFilterPtr PointerType;

    virtual ~ReplicaGroupFilter();

    /**
     * Filter the the given set of adapters.
     * @param replicaGroupId The replica group ID.
     * @param adapterIds The adpater IDs to filter.
     * @param con The connection from the Ice client which is
     * resolving the replica group endpoints.
     * @param ctx The context from the Ice client which is resolving
     * the replica group endpoints.
     * @return The filtered adapter IDs.
     */
    virtual ::Ice::StringSeq filter(const ::std::string& replicaGroupId, const ::Ice::StringSeq& adapterIds, const ::Ice::ConnectionPtr& con, const ::Ice::Context& ctx) = 0;
};

/// \cond INTERNAL
inline bool operator==(const ReplicaGroupFilter& lhs, const ReplicaGroupFilter& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) == static_cast<const ::Ice::LocalObject&>(rhs);
}

inline bool operator<(const ReplicaGroupFilter& lhs, const ReplicaGroupFilter& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) < static_cast<const ::Ice::LocalObject&>(rhs);
}
/// \endcond

/**
 * The TypeFilter is used by IceGrid to filter well-known proxies
 * returned to the client when it searches a well-known object by
 * type.
 *
 * IceGrid provides the list of available proxies. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of proxies.
 * \headerfile IceGrid/IceGrid.h
 */
class ICEGRID_API TypeFilter : public virtual ::Ice::LocalObject
{
public:

    typedef TypeFilterPtr PointerType;

    virtual ~TypeFilter();

    /**
     * Filter the the given set of proxies.
     * @param type The type.
     * @param proxies The proxies to filter.
     * @param con The connection from the Ice client which is
     * looking up well-known objects by type.
     * @param ctx The context from the Ice client which is looking up
     * well-known objects by type.
     * @return The filtered proxies.
     */
    virtual ::Ice::ObjectProxySeq filter(const ::std::string& type, const ::Ice::ObjectProxySeq& proxies, const ::Ice::ConnectionPtr& con, const ::Ice::Context& ctx) = 0;
};

/// \cond INTERNAL
inline bool operator==(const TypeFilter& lhs, const TypeFilter& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) == static_cast<const ::Ice::LocalObject&>(rhs);
}

inline bool operator<(const TypeFilter& lhs, const TypeFilter& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) < static_cast<const ::Ice::LocalObject&>(rhs);
}
/// \endcond

/**
 * The RegistryPluginFacade is implemented by IceGrid and can be used
 * by plugins and filter implementations to retrieve information from
 * IceGrid about the well-known objects or adapters. It's also used to
 * register/unregister replica group and type filters.
 * \headerfile IceGrid/IceGrid.h
 */
class ICEGRID_API RegistryPluginFacade : public virtual ::Ice::LocalObject
{
public:

    typedef RegistryPluginFacadePtr PointerType;

    virtual ~RegistryPluginFacade();

    /**
     * Get an application descriptor.
     * @param name The application name.
     * @return The application descriptor.
     * @throws IceGrid::ApplicationNotExistException Raised if the application
     * doesn't exist.
     */
    virtual ApplicationInfo getApplicationInfo(const ::std::string& name) const = 0;

    /**
     * Get the server information for the server with the given id.
     * @param id The server id.
     * @return The server information.
     * @throws IceGrid::ServerNotExistException Raised if the server doesn't exist.
     */
    virtual ServerInfo getServerInfo(const ::std::string& id) const = 0;

    /**
     * Get the ID of the server to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The server ID or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterServer(const ::std::string& adapterId) const = 0;

    /**
     * Get the name of the application to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The application name or the empty string if the given
     * identifier is not associated to a replica group or object
     * adapter defined with an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterApplication(const ::std::string& adapterId) const = 0;

    /**
     * Get the name of the node to which the given adapter belongs.
     * @param adapterId The adapter ID.
     * @return The node name or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't
     * exist.
     */
    virtual ::std::string getAdapterNode(const ::std::string& adapterId) const = 0;

    /**
     * Get the adapter information for the replica group or adapter
     * with the given id.
     * @param id The adapter id.
     * @return A sequence of adapter information structures. If the
     * given id refers to an adapter, this sequence will contain only
     * one element. If the given id refers to a replica group, the
     * sequence will contain the adapter information of each member of
     * the replica group.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter or
     * replica group doesn't exist.
     */
    virtual AdapterInfoSeq getAdapterInfo(const ::std::string& id) const = 0;

    /**
     * Get the object info for the object with the given identity.
     * @param id The identity of the object.
     * @return The object info.
     * @throws IceGrid::ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     */
    virtual ObjectInfo getObjectInfo(const ::Ice::Identity& id) const = 0;

    /**
     * Get the node information for the node with the given name.
     * @param name The node name.
     * @return The node information.
     * @throws IceGrid::NodeNotExistException Raised if the node doesn't exist.
     * @throws IceGrid::NodeUnreachableException Raised if the node could not be
     * reached.
     */
    virtual NodeInfo getNodeInfo(const ::std::string& name) const = 0;

    /**
     * Get the load averages of the node.
     * @param name The node name.
     * @return The node load information.
     * @throws IceGrid::NodeNotExistException Raised if the node doesn't exist.
     * @throws IceGrid::NodeUnreachableException Raised if the node could not be
     * reached.
     */
    virtual LoadInfo getNodeLoad(const ::std::string& name) const = 0;

    /**
     * Get the property value for the given property and adapter. The
     * property is looked up in the server or service descriptor where
     * the adapter is defined.
     * @param adapterId The adapter ID
     * @param name The name of the property.
     * @return The property value.
     * @throws IceGrid::AdapterNotExistException Raised if the adapter doesn't exist.
     */
    virtual ::std::string getPropertyForAdapter(const ::std::string& adapterId, const ::std::string& name) const = 0;

    /**
     * Add a replica group filter.
     * @param id The identifier of the filter. This identifier must
     * match the value of the "filter" attribute specified in the
     * replica group descriptor. To filter dynamically registered
     * replica groups, you should use the empty filter id.
     * @param filter The filter implementation.
     */
    virtual void addReplicaGroupFilter(const ::std::string& id, const ReplicaGroupFilterPtr& filter) ICE_NOEXCEPT = 0;

    /**
     * Remove a replica group filter.
     * @param id The identifier of the filter.
     * @param filter The filter implementation.
     * @return True of the filter was removed, false otherwise.
     */
    virtual bool removeReplicaGroupFilter(const ::std::string& id, const ReplicaGroupFilterPtr& filter) ICE_NOEXCEPT = 0;

    /**
     * Add a type filter.
     * @param type The type to register this filter with.
     * @param filter The filter implementation.
     */
    virtual void addTypeFilter(const ::std::string& type, const TypeFilterPtr& filter) ICE_NOEXCEPT = 0;

    /**
     * Remove a type filter.
     * @param type The type to register this filter with.
     * @param filter The filter implementation.
     * @return True of the filter was removed, false otherwise.
     */
    virtual bool removeTypeFilter(const ::std::string& type, const TypeFilterPtr& filter) ICE_NOEXCEPT = 0;
};

/// \cond INTERNAL
inline bool operator==(const RegistryPluginFacade& lhs, const RegistryPluginFacade& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) == static_cast<const ::Ice::LocalObject&>(rhs);
}

inline bool operator<(const RegistryPluginFacade& lhs, const RegistryPluginFacade& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) < static_cast<const ::Ice::LocalObject&>(rhs);
}
/// \endcond

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

#endif

#include <IceUtil/PopDisableWarnings.h>
#endif
