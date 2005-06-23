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

dictionary<string, string> StringStringDict;

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
sequence<PropertyDescriptor> PropertyDescriptorSeq;

/**
 *
 * An &Ice; object descriptor.
 * 
 **/
struct ObjectDescriptor
{
    /**
     *
     * The object proxy.
     *
     **/
    Object* proxy;

    /**
     *
     * The object type.
     *
     **/
    string type;
    
    /**
     *
     * The object adapter id.
     *
     **/
    string adapterId;
};

/**
 *
 * A sequence of object descriptors.
 *
 **/
sequence<ObjectDescriptor> ObjectDescriptorSeq;

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
     * The object adapter endpoints.
     *
     **/
    string endpoints;

    /**
     *
     * Flag to specify if the object adapter will register a process object.
     *
     **/
    bool registerProcess;

    /**
     *
     * The object descriptor associated to this object adapter descriptor.
     *
     **/
    ObjectDescriptorSeq objects;
};

/**
 *
 * A sequence of adapter descriptors.
 *
 **/
sequence<AdapterDescriptor> AdapterDescriptorSeq;

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
     * The home of the database environment (i.e.: the directory where the database file
     * will be stored).
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
sequence<DbEnvDescriptor> DbEnvDescriptorSeq;

/**
 *
 * A component descriptor. A component is either an &Ice; server or
 * an &IceBox; service.
 *
 **/
class ComponentDescriptor
{
    /**
     *
     * The component nane.
     *
     **/
    string name;

    /**
     *
     * The variables defined in the component.
     *
     **/
    StringStringDict variables;

    /**
     *
     * The component object adapters.
     *
     **/
    AdapterDescriptorSeq adapters;

    /**
     *
     * The component configuration properties.
     *
     **/
    PropertyDescriptorSeq properties;

    /**
     *
     * The component database environments.
     *
     **/
    DbEnvDescriptorSeq dbEnvs;

    /**
     *
     * Some comments on the component.
     *
     **/
    string comment;
};

/**
 *
 * An &Ice; server descriptor.
 *
 **/
class ServerDescriptor extends ComponentDescriptor
{
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
     * The name of the interpreter or empty if the executable isn't
     * interpreted.
     *
     **/
    string interpreter;

    /**
     *
     * The command line options to pass to the interpreter.
     *
     **/
    Ice::StringSeq interpreterOptions;

    /**
     *
     * The server environment variables.
     *
     **/
    Ice::StringSeq envs;

    /**
     *
     * The server initial activation mode.
     *
     **/
    string activation;
};
dictionary<string, ServerDescriptor> ServerDescriptorDict;


/**
 *
 * An &IceBox; service descriptor.
 *
 **/
class ServiceDescriptor extends ComponentDescriptor
{
    /**
     *
     * The entry point of the &IceBox; service.
     * 
     **/
    string entry;
};
dictionary<string, ServiceDescriptor> ServiceDescriptorDict;

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
     * The node hosting the server.
     *
     **/
    string node;    

    /**
     *
     * The template parameter values.
     *
     **/
    StringStringDict parameterValues;

    /**
     *
     * The instantiated component descriptor (NOTE: this is provided
     * as a convenience. This descriptor can also be computed from the
     * template and the instance variables.)
     *
     **/
    ServerDescriptor descriptor;
    
    /**
     *
     * The targets used to deploy this instance.
     *
     **/
    Ice::StringSeq targets;
};
sequence<ServerInstanceDescriptor> ServerInstanceDescriptorSeq;

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
     * The instantiated component descriptor (NOTE: this is provided
     * as a convenience. This descriptor can also be computed from the
     * template and the instance variables.)
     *
     **/
    ServiceDescriptor descriptor;
    
    /**
     *
     * The targets used to deploy this instance.
     *
     **/
    Ice::StringSeq targets;
};
sequence<ServiceInstanceDescriptor> ServiceInstanceDescriptorSeq;

struct TemplateDescriptor
{
    /**
     *
     * The template.
     *
     **/
    ComponentDescriptor descriptor;

    /**
     *
     * The name of the parameters required to instantiate the template.
     *
     **/
    Ice::StringSeq parameters;
};
dictionary<string, TemplateDescriptor> TemplateDescriptorDict;

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

    /**
     *
     * The endpoints of the &IceBox; service manager interface.
     *
     **/
    string endpoints;
};

struct NodeDescriptor
{
    /**
     *
     * The node name.
     * 
     **/
    string name;

    /**
     *
     * The variables defined for the node.
     *
     **/
    StringStringDict variables;
};
sequence<NodeDescriptor> NodeDescriptorSeq;

/**
 *
 * An application descriptor.
 *
 **/
class ApplicationDescriptor
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
    NodeDescriptorSeq nodes;
    
    /**
     *
     * The server instances.
     *
     **/
    ServerInstanceDescriptorSeq servers;

    /**
     *
     * The targets used to deploy the application.
     *
     **/
    Ice::StringSeq targets;

    /**
     *
     * Some comments on the application.
     *
     **/ 
    string comment;
};
sequence<ApplicationDescriptor> ApplicationDescriptorSeq;

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
    NodeDescriptorSeq nodes;

    /**
     *
     * The nodes to remove.
     *
     **/
    Ice::StringSeq removeNodes;
    
    /**
     *
     * The server instances to update.
     *
     **/
    ServerInstanceDescriptorSeq servers;

    /**
     *
     * The name of the server instances to remove.
     *
     **/
    Ice::StringSeq removeServers;
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
     * state if the server fork succeeded or to the Inactive state if
     * it failed.
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