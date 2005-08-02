// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_DESCRIPTOR_ICE
#define ICE_GRID_DESCRIPTOR_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

module IceGrid
{

["java:type:java.util.TreeMap"] dictionary<string, string> StringStringDict;

/**
 *
 * Property descriptor.
 * 
 **/
struct PropertyDescriptor
{
    string name;
    string value;
};
["java:type:java.util.LinkedList"] sequence<PropertyDescriptor> PropertyDescriptorSeq;

/**
 *
 * An &Ice; object descriptor.
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
};

/**
 *
 * A sequence of object descriptors.
 *
 **/
["java:type:java.util.LinkedList"] sequence<ObjectDescriptor> ObjectDescriptorSeq;

/**
 *
 * An &Ice; object adapter descriptor.
 *
 **/
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
     * The object adapter id.
     *
     **/
    string id;

    /**
     *
     * Flag to specify if the object adapter will register a process object.
     *
     **/
    bool registerProcess;

    /**
     *
     * If true the activator will wait for this object adapter
     * activation to mark the server as active.
     *
     **/
    bool waitForActivation;

    /**
     *
     * The object descriptors associated to this object adapter.
     *
     **/
    ObjectDescriptorSeq objects;
};

/**
 *
 * A sequence of adapter descriptors.
 *
 **/
["java:type:java.util.LinkedList"] sequence<AdapterDescriptor> AdapterDescriptorSeq;

/**
 *
 * A &Freeze; database environment descriptor.
 *
 **/
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
     * The home of the database environment (i.e.: the directory where
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
["java:type:java.util.LinkedList"] sequence<DbEnvDescriptor> DbEnvDescriptorSeq;

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
     * The configuration properties.
     *
     **/
    PropertyDescriptorSeq properties;

    /**
     *
     * The database environments.
     *
     **/
    DbEnvDescriptorSeq dbEnvs;

    /**
     *
     * A description of this descriptor.
     *
     **/
    string description;
};

/**
 *
 * An &Ice; server descriptor.
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
     * The path to the server working directory.
     *
     **/
    string pwd;

    /**
     *
     * The command line options to pass to the server executable.
     *
     **/
    Ice::StringSeq options;

    /**
     *
     * The server environment variables.
     *
     **/
    Ice::StringSeq envs;

    /**
     *
     * The server initial activation mode (possible values are
     * "on-demand" or "manual").
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
};
dictionary<string, ServerDescriptor> ServerDescriptorDict;
["java:type:java.util.LinkedList"] sequence<ServerDescriptor> ServerDescriptorSeq;


/**
 *
 * An &IceBox; service descriptor.
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
     * The entry point of the &IceBox; service.
     * 
     **/
    string entry;
};
dictionary<string, ServiceDescriptor> ServiceDescriptorDict;
["java:type:java.util.LinkedList"] sequence<ServiceDescriptor> ServiceDescriptorSeq;

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
};
["java:type:java.util.LinkedList"] sequence<ServerInstanceDescriptor> ServerInstanceDescriptorSeq;
dictionary<string, ServerInstanceDescriptor> ServerInstanceDescriptorDict;

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
     * The name of the parameters required to instantiate the template.
     *
     **/
    ["java:type:java.util.LinkedList"] Ice::StringSeq parameters;
};
dictionary<string, TemplateDescriptor> TemplateDescriptorDict;

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
};
["java:type:java.util.LinkedList"] sequence<ServiceInstanceDescriptor> ServiceInstanceDescriptorSeq;

/**
 *
 * An &IceBox; server descriptor.
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

struct NodeDescriptor
{
    /**
     *
     * The variables defined for the node.
     *
     **/
    StringStringDict variables;

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
};
dictionary<string, NodeDescriptor> NodeDescriptorDict;

/**
 *
 * A replicated object adapter descriptor.
 * 
 **/
enum LoadBalancingPolicy
{
    Random,
    RoundRobin,
    Adaptive
};

struct ReplicatedAdapterDescriptor
{
    /**
     *
     * The id of the replicated object adapter.
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
     * The object descriptors associated to this object adapter.
     *
     **/
    ObjectDescriptorSeq objects;
};
sequence<ReplicatedAdapterDescriptor> ReplicatedAdapterDescriptorSeq; 

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
    StringStringDict variables;

    /**
     *
     * The replicated adapters.
     *
     **/
    ReplicatedAdapterDescriptorSeq replicatedAdapters;

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
     * A description of the application;
     *
     **/ 
    string description;
};
["java:type:java.util.LinkedList"] sequence<ApplicationDescriptor> ApplicationDescriptorSeq;

class BoxedDescription
{
    string value;
};

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
     * The variables to update.
     *
     **/
    StringStringDict variables;

    /**
     *
     * The variables to remove.
     *
     **/
    Ice::StringSeq removeVariables;

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
};
["java:type:java.util.LinkedList"] sequence<NodeUpdateDescriptor> NodeUpdateDescriptorSeq;

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
     * The updated description (or null if the description wasn't updated).
     *
     **/
    BoxedDescription description;

    /**
     *
     * The variables to update.
     *
     **/
    StringStringDict variables;

    /**
     *
     * The variables to remove.
     *
     **/
    Ice::StringSeq removeVariables;

    /**
     *
     * The replicated adapters to update.
     *
     **/
    ReplicatedAdapterDescriptorSeq replicatedAdapters;

    /**
     *
     * The replicated adapters to remove.
     *
     **/
    Ice::StringSeq removeReplicatedAdapters;

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
     * The ids of the service tempate to remove.
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

/**
 *
 * An enumeration representing the state of the server.
 *
 **/
enum ServerState
{
    /**
     *
     * The server is not running.
     *
     **/
    Inactive,

    /**
     *
     * The server is being activated and will change to the active
     * state when the registered server object adapters are activated.
     *
     **/
    Activating,

    /**
     *
     * The server is running.
     *
     **/
    Active,
    
    /**
     *
     * The server is being deactivated.
     *
     **/
    Deactivating,

    /**
     *
     * The server is being destroyed.
     *
     **/
    Destroying,

    /**
     *
     * The server is destroyed.
     *
     **/
    Destroyed
};

};

#endif
