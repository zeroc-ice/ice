// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:IceGrid/Config.h"]]

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * A mapping of string to string.
 *
 **/
dictionary<string, string> StringStringDict;

/**
 *
 * Property descriptor.
 *
 **/
struct PropertyDescriptor
{
    /**
     *
     * The name of the property.
     *
     **/
    string name;

    /**
     *
     * The value of the property.
     *
     **/
    string value;
};

/**
 *
 * A sequence of property descriptors.
 *
 **/
["java:type:java.util.LinkedList<PropertyDescriptor>"] sequence<PropertyDescriptor> PropertyDescriptorSeq;

/**
 *
 * A property set descriptor.
 *
 **/
["cpp:comparable"]
struct PropertySetDescriptor
{
    /**
     *
     * References to named property sets.
     *
     **/
    Ice::StringSeq references;

    /**
     *
     * The property set properties.
     *
     **/
    PropertyDescriptorSeq properties;
};

/**
 *
 * A mapping of property set name to property set descriptor.
 *
 **/
dictionary<string, PropertySetDescriptor> PropertySetDescriptorDict;

/**
 *
 * An Ice object descriptor.
 *
 **/
struct ObjectDescriptor
{
    /**
     *
     * The identity of the object.
     *
     **/
    Ice::Identity id;

    /**
     *
     * The object type.
     *
     **/
    string type;

    /**
     *
     * Proxy options to use with the proxy created for this Ice object. If empty,
     * the proxy will be created with the proxy options specified on the object
     * adapter or replica group.
     *
     **/
    string proxyOptions;
};

/**
 *
 * A sequence of object descriptors.
 *
 **/
["java:type:java.util.LinkedList<ObjectDescriptor>"] sequence<ObjectDescriptor> ObjectDescriptorSeq;

/**
 *
 * An Ice object adapter descriptor.
 *
 **/
["cpp:comparable"]
struct AdapterDescriptor
{
    /**
     *
     * The object adapter name.
     *
     **/
    string name;

    /**
     *
     * The description of this object adapter.
     *
     **/
    string description;

    /**
     *
     * The object adapter id.
     *
     **/
    string id;

    /**
     *
     * The replica id of this adapter.
     *
     **/
    string replicaGroupId;

    /**
     *
     * The adapter priority. This is eventually used when the adapter
     * is member of a replica group to sort the adapter endpoints by
     * priority.
     *
     **/
    string priority;

    /**
     *
     * Flag to specify if the object adapter will register a process object.
     *
     **/
    bool registerProcess;

    /**
     *
     * If true the lifetime of this object adapter is the same of the
     * server lifetime. This information is used by the IceGrid node
     * to figure out the server state: the server is active only if
     * all its "server lifetime" adapters are active.
     *
     **/
    bool serverLifetime;

    /**
     *
     * The well-known object descriptors associated with this object adapter.
     *
     **/
    ObjectDescriptorSeq objects;

    /**
     *
     * The allocatable object descriptors associated with this object adapter.
     *
     **/
    ObjectDescriptorSeq allocatables;
};

/**
 *
 * A sequence of adapter descriptors.
 *
 **/
["java:type:java.util.LinkedList<AdapterDescriptor>"] sequence<AdapterDescriptor> AdapterDescriptorSeq;

/**
 *
 * A Freeze database environment descriptor.
 *
 **/
["cpp:comparable"]
struct DbEnvDescriptor
{
    /**
     *
     * The name of the database environment.
     *
     **/
    string name;

    /**
     *
     * The description of this database environment.
     *
     **/
    string description;

    /**
     *
     * The home of the database environment (i.e., the directory where
     * the database files will be stored). If empty, the node will
     * provide a default database directory, otherwise the directory
     * must exist.
     *
     **/
    string dbHome;

    /**
     *
     * The configuration properties of the database environment.
     *
     **/
    PropertyDescriptorSeq properties;
};

/**
 *
 * A sequence of database environment descriptors.
 *
 **/
["java:type:java.util.LinkedList<DbEnvDescriptor>"] sequence<DbEnvDescriptor> DbEnvDescriptorSeq;

/**
 *
 * A communicator descriptor.
 *
 **/
class CommunicatorDescriptor
{
    /**
     *
     * The object adapters.
     *
     **/
    AdapterDescriptorSeq adapters;

    /**
     *
     * The property set.
     *
     **/
    PropertySetDescriptor propertySet;

    /**
     *
     * The database environments.
     *
     **/
    DbEnvDescriptorSeq dbEnvs;

    /**
     *
     * The path of each log file.
     *
     **/
    Ice::StringSeq logs;

    /**
     *
     * A description of this descriptor.
     *
     **/
    string description;
};

/**
 *
 * A distribution descriptor defines an IcePatch2 server and the
 * directories to retrieve from the patch server.
 *
 **/
["cpp:comparable"]
struct DistributionDescriptor
{
    /** The proxy of the IcePatch2 server. */
    string icepatch;

    /** The source directories. */
    ["java:type:java.util.LinkedList<String>"] Ice::StringSeq directories;
};

/**
 *
 * An Ice server descriptor.
 *
 **/
class ServerDescriptor extends CommunicatorDescriptor
{
    /**
     *
     * The server id.
     *
     **/
    string id;

    /**
     *
     * The path of the server executable.
     *
     **/
    string exe;

    /**
     *
     * The Ice version used by this server. This is only required if
     * backward compatibility with servers using old Ice versions is
     * needed (otherwise the registry will assume the server is using
     * the same Ice version).
     * For example "3.1.1", "3.2", "3.3.0".
     *
     **/
    string iceVersion;

    /**
     *
     * The path to the server working directory.
     *
     **/
    string pwd;

    /**
     *
     * The command line options to pass to the server executable.
     *
     **/
    ["java:type:java.util.LinkedList<String>"] Ice::StringSeq options;

    /**
     *
     * The server environment variables.
     *
     **/
    ["java:type:java.util.LinkedList<String>"] Ice::StringSeq envs;

    /**
     *
     * The server activation mode (possible values are "on-demand" or
     * "manual").
     *
     **/
    string activation;

    /**
     *
     * The activation timeout (an integer value representing the
     * number of seconds to wait for activation).
     *
     **/
    string activationTimeout;

    /**
     *
     * The deactivation timeout (an integer value representing the
     * number of seconds to wait for deactivation).
     *
     **/
    string deactivationTimeout;

    /**
     *
     * Specifies if the server depends on the application
     * distribution.
     *
     **/
    bool applicationDistrib;

    /**
     *
     * The distribution descriptor.
     *
     **/
    DistributionDescriptor distrib;

    /**
     *
     * Specifies if the server is allocatable.
     *
     **/
    bool allocatable;

    /**
     *
     * The user account used to run the server.
     *
     **/
    string user;
};

/**
 *
 * A sequence of server descriptors.
 *
 **/
["java:type:java.util.LinkedList<ServerDescriptor>"] sequence<ServerDescriptor> ServerDescriptorSeq;

/**
 *
 * An IceBox service descriptor.
 *
 **/
class ServiceDescriptor extends CommunicatorDescriptor
{
    /**
     *
     * The service name.
     *
     **/
    string name;

    /**
     *
     * The entry point of the IceBox service.
     *
     **/
    string entry;
};

/**
 *
 * A sequence of service descriptors.
 *
 **/
["java:type:java.util.LinkedList<ServiceDescriptor>"] sequence<ServiceDescriptor> ServiceDescriptorSeq;

/**
 *
 * A server template instance descriptor.
 *
 **/
["cpp:comparable"]
struct ServerInstanceDescriptor
{
    /**
     *
     * The template used by this instance.
     *
     **/
    string template;

    /**
     *
     * The template parameter values.
     *
     **/
    StringStringDict parameterValues;

    /**
     *
     * The property set.
     *
     **/
    PropertySetDescriptor propertySet;

    /**
     *
     * The services property sets. It's only valid to set these
     * property sets if the template is an IceBox server template.
     *
     **/
    PropertySetDescriptorDict servicePropertySets;
};

/**
 *
 * A sequence of server instance descriptors.
 *
 **/
["java:type:java.util.LinkedList<ServerInstanceDescriptor>"]
sequence<ServerInstanceDescriptor> ServerInstanceDescriptorSeq;

/**
 *
 * A template descriptor for server or service templates.
 *
 **/
struct TemplateDescriptor
{
    /**
     *
     * The template.
     *
     **/
    CommunicatorDescriptor descriptor;

    /**
     *
     * The parameters required to instantiate the template.
     *
     **/
    ["java:type:java.util.LinkedList<String>"] Ice::StringSeq parameters;

    /**
     *
     * The parameters default values.
     *
     **/
    StringStringDict parameterDefaults;
};

/**
 *
 * A mapping of template identifier to template descriptor.
 *
 **/
dictionary<string, TemplateDescriptor> TemplateDescriptorDict;

/**
 *
 * A service template instance descriptor.
 *
 **/
struct ServiceInstanceDescriptor
{
    /**
     *
     * The template used by this instance.
     *
     **/
    string template;

    /**
     *
     * The template parameter values.
     *
     **/
    StringStringDict parameterValues;

    /**
     *
     * The service definition if the instance isn't a template
     * instance (i.e.: if the template attribute is empty).
     *
     **/
    ServiceDescriptor descriptor;

    /**
     *
     * The property set.
     *
     **/
    PropertySetDescriptor propertySet;
};

/**
 *
 * A sequence of service instance descriptors.
 *
 **/
["java:type:java.util.LinkedList<ServiceInstanceDescriptor>"]
sequence<ServiceInstanceDescriptor> ServiceInstanceDescriptorSeq;

/**
 *
 * An IceBox server descriptor.
 *
 **/
class IceBoxDescriptor extends ServerDescriptor
{
    /**
     *
     * The service instances.
     *
     **/
    ServiceInstanceDescriptorSeq services;
};

/**
 *
 * A node descriptor.
 *
 **/
struct NodeDescriptor
{
    /**
     *
     * The variables defined for the node.
     *
     **/
    ["java:type:java.util.TreeMap<String, String>"] StringStringDict variables;

    /**
     *
     * The server instances.
     *
     **/
    ServerInstanceDescriptorSeq serverInstances;

    /**
     *
     * Servers (which are not template instances).
     *
     **/
    ServerDescriptorSeq servers;

    /**
     *
     * Load factor of the node.
     *
     **/
    string loadFactor;

    /**
     *
     * The description of this node.
     *
     **/
    string description;

    /**
     *
     * Property set descriptors.
     *
     **/
    PropertySetDescriptorDict propertySets;
};

/**
 *
 * Mapping of node name to node descriptor.
 *
 **/
dictionary<string, NodeDescriptor> NodeDescriptorDict;

/**
 *
 * A base class for load balancing policies.
 *
 **/
class LoadBalancingPolicy
{
    /**
     *
     * The number of replicas that will be used to gather the
     * endpoints of a replica group.
     *
     **/
    string nReplicas;
};

/**
 *
 * Random load balancing policy.
 *
 **/
class RandomLoadBalancingPolicy extends LoadBalancingPolicy
{
};

/**
 *
 * Ordered load balancing policy.
 *
 **/
class OrderedLoadBalancingPolicy extends LoadBalancingPolicy
{
};

/**
 *
 * Round robin load balancing policy.
 *
 **/
class RoundRobinLoadBalancingPolicy extends LoadBalancingPolicy
{
};

/**
 *
 * Adaptive load balancing policy.
 *
 **/
class AdaptiveLoadBalancingPolicy extends LoadBalancingPolicy
{
    /**
     *
     * The load sample to use for the load balancing. The allowed
     * values for this attribute are "1", "5" and "15", representing
     * respectively the load average over the past minute, the past 5
     * minutes and the past 15 minutes.
     *
     **/
    string loadSample;
};

/**
 *
 * A replica group descriptor.
 *
 **/
struct ReplicaGroupDescriptor
{
    /**
     *
     * The id of the replica group.
     *
     **/
    string id;

    /**
     *
     * The load balancing policy.
     *
     **/
    LoadBalancingPolicy loadBalancing;

    /**
     *
     * Default options for proxies created for the replica group.
     *
     **/
    string proxyOptions;

    /**
     *
     * The object descriptors associated with this object adapter.
     *
     **/
    ObjectDescriptorSeq objects;

    /**
     *
     * The description of this replica group.
     *
     **/
    string description;

    /**
     *
     * The filter to use for this replica group.
     *
     **/
    string filter;
};

/**
 *
 * A sequence of replica groups.
 *
 **/
["java:type:java.util.LinkedList<ReplicaGroupDescriptor>"] sequence<ReplicaGroupDescriptor> ReplicaGroupDescriptorSeq;

/**
 *
 * An application descriptor.
 *
 **/
struct ApplicationDescriptor
{
    /**
     *
     * The application name.
     *
     **/
    string name;

    /**
     *
     * The variables defined in the application descriptor.
     *
     **/
    ["java:type:java.util.TreeMap<String, String>"] StringStringDict variables;

    /**
     *
     * The replica groups.
     *
     **/
    ReplicaGroupDescriptorSeq replicaGroups;

    /**
     *
     * The server templates.
     *
     **/
    TemplateDescriptorDict serverTemplates;

    /**
     *
     * The service templates.
     *
     **/
    TemplateDescriptorDict serviceTemplates;

    /**
     *
     * The application nodes.
     *
     **/
    NodeDescriptorDict nodes;

    /**
     *
     * The application distribution.
     *
     **/
    DistributionDescriptor distrib;

    /**
     *
     * The description of this application.
     *
     **/
    string description;

    /**
     *
     * Property set descriptors.
     *
     **/
    PropertySetDescriptorDict propertySets;
};

/**
 *
 * A sequence of application descriptors.
 *
 **/
["java:type:java.util.LinkedList<ApplicationDescriptor>"] sequence<ApplicationDescriptor> ApplicationDescriptorSeq;

/**
 *
 * A "boxed" string.
 *
 **/
class BoxedString
{
    /** The value of the boxed string. */
    string value;
};

/**
 *
 * A node update descriptor to describe the updates to apply to a
 * node of a deployed application.
 *
 **/
struct NodeUpdateDescriptor
{
    /**
     *
     * The name of the node to update.
     *
     **/
    string name;

    /**
     *
     * The updated description (or null if the description wasn't
     * updated.)
     *
     **/
    BoxedString description;

    /**
     *
     * The variables to update.
     *
     **/
    ["java:type:java.util.TreeMap<String, String>"] StringStringDict variables;

    /**
     *
     * The variables to remove.
     *
     **/
    Ice::StringSeq removeVariables;

    /**
     *
     * The property sets to update.
     *
     **/
    PropertySetDescriptorDict propertySets;

    /**
     *
     * The property sets to remove.
     *
     **/
    Ice::StringSeq removePropertySets;

    /**
     *
     * The server instances to update.
     *
     **/
    ServerInstanceDescriptorSeq serverInstances;

    /**
     *
     * The servers which are not template instances to update.
     *
     **/
    ServerDescriptorSeq servers;

    /**
     *
     * The ids of the servers to remove.
     *
     **/
    Ice::StringSeq removeServers;

    /**
     *
     * The updated load factor of the node (or null if the load factor
     * was not updated).
     *
     **/
    BoxedString loadFactor;
};

/**
 *
 * A sequence of node update descriptors.
 *
 **/
["java:type:java.util.LinkedList<NodeUpdateDescriptor>"] sequence<NodeUpdateDescriptor> NodeUpdateDescriptorSeq;

/**
 *
 * A "boxed" distribution descriptor.
 *
 **/
class BoxedDistributionDescriptor
{
    /** The value of the boxed distribution descriptor. */
    DistributionDescriptor value;
};

/**
 *
 * An application update descriptor to describe the updates to apply
 * to a deployed application.
 *
 **/
struct ApplicationUpdateDescriptor
{
    /**
     *
     * The name of the application to update.
     *
     **/
    string name;

    /**
     *
     * The updated description (or null if the description wasn't
     * updated.)
     *
     **/
    BoxedString description;

    /**
     *
     * The updated distribution application descriptor.
     *
     **/
    BoxedDistributionDescriptor distrib;

    /**
     *
     * The variables to update.
     *
     **/
    ["java:type:java.util.TreeMap<String, String>"] StringStringDict variables;

    /**
     *
     * The variables to remove.
     *
     **/
    Ice::StringSeq removeVariables;

    /**
     *
     * The property sets to update.
     *
     **/
    PropertySetDescriptorDict propertySets;

    /**
     *
     * The property sets to remove.
     *
     **/
    Ice::StringSeq removePropertySets;

    /**
     *
     * The replica groups to update.
     *
     **/
    ReplicaGroupDescriptorSeq replicaGroups;

    /**
     *
     * The replica groups to remove.
     *
     **/
    Ice::StringSeq removeReplicaGroups;

    /**
     *
     * The server templates to update.
     *
     **/
    TemplateDescriptorDict serverTemplates;

    /**
     *
     * The ids of the server template to remove.
     *
     **/
    Ice::StringSeq removeServerTemplates;

    /**
     *
     * The service templates to update.
     *
     **/
    TemplateDescriptorDict serviceTemplates;

    /**
     *
     * The ids of the service template to remove.
     *
     **/
    Ice::StringSeq removeServiceTemplates;

    /**
     *
     * The application nodes to update.
     *
     **/
    NodeUpdateDescriptorSeq nodes;

    /**
     *
     * The nodes to remove.
     *
     **/
    Ice::StringSeq removeNodes;
};

};

