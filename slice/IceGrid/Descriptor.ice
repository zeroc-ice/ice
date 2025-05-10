// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceGrid"]]

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    /// A mapping of string to string.
    dictionary<string, string> StringStringDict;

    /// Describes an Ice property.
    struct PropertyDescriptor
    {
        /// The name of the property.
        string name;

        /// The value of the property.
        string value;
    }

    /// A sequence of PropertyDescriptor.
    ["java:type:java.util.LinkedList<PropertyDescriptor>"]
    sequence<PropertyDescriptor> PropertyDescriptorSeq;

    /// A property set descriptor.
    struct PropertySetDescriptor
    {
        /// References to named property sets.
        Ice::StringSeq references;

        /// The property set properties.
        ["matlab:identifier:propertyDescriptors"]
        PropertyDescriptorSeq properties;
    }

    /// A mapping of property set name to property set descriptor.
    dictionary<string, PropertySetDescriptor> PropertySetDescriptorDict;

    /// Describes a well-known Ice object.
    struct ObjectDescriptor
    {
        /// The identity of the object.
        Ice::Identity id;

        /// The object type.
        string type;

        /// The proxy options to use when creating a proxy for this well-known object. If empty, the proxy is created
        /// with the proxy options specified on the object adapter or replica group.
        string proxyOptions;
    }

    /// A sequence of ObjectDescriptor.
    ["java:type:java.util.LinkedList<ObjectDescriptor>"]
    sequence<ObjectDescriptor> ObjectDescriptorSeq;

    /// Describes an indirect object adapter.
    struct AdapterDescriptor
    {
        /// The object adapter name.
        string name;

        /// A description of this object adapter.
        string description;

        /// The adapter ID.
        string id;

        /// The replica group ID. It's empty when the adapter is not part of a replica group.
        string replicaGroupId;

        /// The adapter priority. Only relevant when the adapter is in a replica group.
        string priority;

        /// When `true`, the object adapter registers a process object.
        bool registerProcess;

        /// When `true`, the lifetime of this object adapter is the same of the server lifetime. This information is
        /// used by the IceGrid node to figure out the server state: the server is active when all its "server lifetime"
        /// adapters are active.
        bool serverLifetime;

        /// The descriptors of well-known objects.
        ObjectDescriptorSeq objects;

        /// The descriptors of allocatable objects
        ObjectDescriptorSeq allocatables;
    }

    /// A sequence of AdapterDescriptor.
    ["java:type:java.util.LinkedList<AdapterDescriptor>"]
    sequence<AdapterDescriptor> AdapterDescriptorSeq;

    /// Describes an Ice communicator.
    class CommunicatorDescriptor
    {
        /// The indirect object adapters.
        AdapterDescriptorSeq adapters;

        /// The property set.
        PropertySetDescriptor propertySet;

        /// The path of each log file.
        Ice::StringSeq logs;

        /// A description of this descriptor.
        string description;
    }

    /// Describes a distribution.
    ["deprecated:This descriptor is provided for schema compatibility. It is no longer used as of Ice 3.8."]
    struct DistributionDescriptor
    {
        /// The proxy of the IcePatch2 server.
        string icepatch;

        /// The source directories.
        ["java:type:java.util.LinkedList<String>"]
        Ice::StringSeq directories;
    }

    /// Describes an Ice server.
    class ServerDescriptor extends CommunicatorDescriptor
    {
        /// The server ID.
        string id;

        /// The path of the server executable.
        string exe;

        /// The Ice version used by this server. This is only required if backward compatibility with servers using old
        /// Ice versions is needed (otherwise the registry assumes the server is using the same Ice version as the
        /// registry itself). For example "3.7.5".
        string iceVersion;

        /// The path to the server working directory.
        string pwd;

        /// The command line options to pass to the server executable.
        ["java:type:java.util.LinkedList<String>"]
        Ice::StringSeq options;

        /// The server environment variables.
        ["java:type:java.util.LinkedList<String>"]
        Ice::StringSeq envs;

        //// The server activation mode. Possible values are "on-demand" and "manual".
        string activation;

        /// The activation timeout. It's an integer (in string format) that represents the number of seconds to wait for
        /// activation.
        string activationTimeout;

        /// The deactivation timeout. It's an integer (in string format) that represents the number of seconds to wait
        /// for deactivation.
        string deactivationTimeout;

        /// Specifies if the server depends on the application distribution.
        ["deprecated"]
        bool applicationDistrib = false;

        /// The distribution descriptor.
        ["deprecated"]
        DistributionDescriptor distrib;

        /// Specifies if the server is allocatable.
        bool allocatable;

        /// The user account used to run the server.
        string user;
    }

    /// A sequence of ServerDescriptor.
    ["java:type:java.util.LinkedList<ServerDescriptor>"]
    sequence<ServerDescriptor> ServerDescriptorSeq;

    /// Describes an IceBox service.
    class ServiceDescriptor extends CommunicatorDescriptor
    {
        /// The service name.
        string name;

        /// The entry point of the IceBox service.
        string entry;
    }

    /// A sequence of ServiceDescriptor.
    ["java:type:java.util.LinkedList<ServiceDescriptor>"]
    sequence<ServiceDescriptor> ServiceDescriptorSeq;

    /// Describes a template instantiation that creates a server.
    struct ServerInstanceDescriptor
    {
        /// The template used by this instance. It's never empty.
        ["cpp:identifier:templateName"]
        string template;

        /// The template parameter values.
        StringStringDict parameterValues;

        /// The property set.
        PropertySetDescriptor propertySet;

        /// The services property sets. It's only valid to set these property sets when the template is an IceBox server
        /// template.
        PropertySetDescriptorDict servicePropertySets;
    }

    /// A sequence of ServerInstanceDescriptor.
    ["java:type:java.util.LinkedList<ServerInstanceDescriptor>"]
    sequence<ServerInstanceDescriptor> ServerInstanceDescriptorSeq;

    /// Describes a template for a server or an IceBox service.
    struct TemplateDescriptor
    {
        /// The communicator.
        CommunicatorDescriptor descriptor;

        /// The parameters required to instantiate the template.
        ["java:type:java.util.LinkedList<String>"]
        Ice::StringSeq parameters;

        /// The parameters default values.
        StringStringDict parameterDefaults;
    }

    /// A mapping of template identifier to template descriptor.
    dictionary<string, TemplateDescriptor> TemplateDescriptorDict;

    /// Describes an IceBox service.
    struct ServiceInstanceDescriptor
    {
        /// The template used by this instance. It's empty when this instance does not use a template.
        ["cpp:identifier:templateName"]
        string template;

        /// The template parameter values.
        StringStringDict parameterValues;

        /// The service definition if the instance isn't a template instance (i.e.: if the template attribute is empty).
        ServiceDescriptor descriptor;

        /// The property set.
        PropertySetDescriptor propertySet;
    }

    /// A sequence of ServiceInstanceDescriptor.
    ["java:type:java.util.LinkedList<ServiceInstanceDescriptor>"]
    sequence<ServiceInstanceDescriptor> ServiceInstanceDescriptorSeq;

    /// Describes an IceBox server.
    class IceBoxDescriptor extends ServerDescriptor
    {
        /// The service instances.
        ServiceInstanceDescriptorSeq services;
    }

    /// Describes an IceGrid node.
    struct NodeDescriptor
    {
        /// The variables defined for the node.
        ["java:type:java.util.TreeMap<String, String>"]
        StringStringDict variables;

        /// The server instances (template instances).
        ServerInstanceDescriptorSeq serverInstances;

        /// Servers that are not template instances.
        ServerDescriptorSeq servers;

        /// Load factor of the node.
        string loadFactor;

        /// The description of this node.
        string description;

        /// Property set descriptors.
        PropertySetDescriptorDict propertySets;
    }

    /// Mapping of node name to node descriptor.
    dictionary<string, NodeDescriptor> NodeDescriptorDict;

    /// The base class for load balancing policies.
    class LoadBalancingPolicy
    {
        /// The number of replicas that will be used to gather the endpoints of a replica group.
        string nReplicas;
    }

    /// The load balancing policy that returns endpoints in a random order.
    class RandomLoadBalancingPolicy extends LoadBalancingPolicy
    {
    }

    /// The load balancing policy that returns endpoints in order.
    class OrderedLoadBalancingPolicy extends LoadBalancingPolicy
    {
    }

    /// The load balancing policy that returns endpoints using round-robin.
    class RoundRobinLoadBalancingPolicy extends LoadBalancingPolicy
    {
    }

    /// The load balancing policy that returns the endpoints of the server(s) with the lowest load average.
    class AdaptiveLoadBalancingPolicy extends LoadBalancingPolicy
    {
        /// The load sample to use for the load balancing. The allowed values for this attribute are "1", "5" and "15",
        /// representing respectively the load average over the past minute, the past 5 minutes and the past 15 minutes.
        string loadSample;
    }

    /// Describes a replica group.
    struct ReplicaGroupDescriptor
    {
        /// The replica group ID.
        string id;

        /// The load balancing policy.
        LoadBalancingPolicy loadBalancing;

        /// Default options for proxies created for the replica group.
        string proxyOptions;

        /// The descriptors for the well-known objects.
        ObjectDescriptorSeq objects;

        /// The description of this replica group.
        string description;

        /// The filter to use for this replica group.
        string filter;
    }

    /// A sequence of ReplicaGroupDescriptor.
    ["java:type:java.util.LinkedList<ReplicaGroupDescriptor>"]
    sequence<ReplicaGroupDescriptor> ReplicaGroupDescriptorSeq;

    /// Describes an application.
    struct ApplicationDescriptor
    {
        /// The application name.
        string name;

        /// The variables defined in the application descriptor.
        ["java:type:java.util.TreeMap<String, String>"]
        StringStringDict variables;

        /// The replica groups.
        ReplicaGroupDescriptorSeq replicaGroups;

        /// The server templates.
        TemplateDescriptorDict serverTemplates;

        /// The service templates.
        TemplateDescriptorDict serviceTemplates;

        /// The node descriptors.
        NodeDescriptorDict nodes;

        /// The application distribution.
        ["deprecated"]
        DistributionDescriptor distrib;

        /// The description of this application.
        string description;

        /// Property set descriptors.
        PropertySetDescriptorDict propertySets;
    }

    /// A sequence of ApplicationDescriptor.
    ["java:type:java.util.LinkedList<ApplicationDescriptor>"]
    sequence<ApplicationDescriptor> ApplicationDescriptorSeq;

    /// A "boxed" string.
    class BoxedString
    {
        /// The value of the boxed string.
        string value;
    }

    /// Describes the updates to apply to a node in a deployed application.
    struct NodeUpdateDescriptor
    {
        /// The name of the node to update.
        string name;

        /// The updated description (or null if the description wasn't updated.)
        BoxedString description;

        /// The variables to update.
        ["java:type:java.util.TreeMap<String, String>"]
        StringStringDict variables;

        /// The variables to remove.
        Ice::StringSeq removeVariables;

        /// The property sets to update.
        PropertySetDescriptorDict propertySets;

        /// The property sets to remove.
        Ice::StringSeq removePropertySets;

        /// The server instances to update.
        ServerInstanceDescriptorSeq serverInstances;

        /// The servers which are not template instances to update.
        ServerDescriptorSeq servers;

        /// The IDs of the servers to remove.
        Ice::StringSeq removeServers;

        /// The updated load factor of the node (or null if the load factor was not updated).
        BoxedString loadFactor;
    }

    /// A sequence of NodeUpdateDescriptor.
    ["java:type:java.util.LinkedList<NodeUpdateDescriptor>"]
    sequence<NodeUpdateDescriptor> NodeUpdateDescriptorSeq;

    /// A "boxed" distribution descriptor.
    ["deprecated"]
    class BoxedDistributionDescriptor
    {
        /// The value of the boxed distribution descriptor.
        DistributionDescriptor value;
    }

    /// An application update descriptor to describe the updates to apply to a deployed application.
    struct ApplicationUpdateDescriptor
    {
        /// The name of the application to update.
        string name;

        /// The updated description (or null if the description wasn't updated.)
        BoxedString description;

        /// The updated distribution application descriptor.
        BoxedDistributionDescriptor distrib;

        /// The variables to update.
        ["java:type:java.util.TreeMap<String, String>"]
        StringStringDict variables;

        /// The variables to remove.
        Ice::StringSeq removeVariables;

        /// The property sets to update.
        PropertySetDescriptorDict propertySets;

        /// The property sets to remove.
        Ice::StringSeq removePropertySets;

        /// The replica groups to update.
        ReplicaGroupDescriptorSeq replicaGroups;

        /// The replica groups to remove.
        Ice::StringSeq removeReplicaGroups;

        /// The server templates to update.
        TemplateDescriptorDict serverTemplates;

        /// The IDs of the server template to remove.
        Ice::StringSeq removeServerTemplates;

        /// The service templates to update.
        TemplateDescriptorDict serviceTemplates;

        /// The IDs of the service template to remove.
        Ice::StringSeq removeServiceTemplates;

        /// The application nodes to update.
        NodeUpdateDescriptorSeq nodes;

        /// The nodes to remove.
        Ice::StringSeq removeNodes;
    }
}
