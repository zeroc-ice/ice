// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_DESCRIPTOR_HELPER_H
#define ICE_GRID_DESCRIPTOR_HELPER_H

#include <IceUtil/OutputUtil.h>
#include <IceXML/Parser.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Internal.h>
#include <set>

namespace IceGrid
{

class ApplicationHelper;
class Resolver
{
public:

    Resolver(const ApplicationDescriptor&, const Ice::CommunicatorPtr&, bool);
    Resolver(const Resolver&, const std::map<std::string, std::string>&, bool);
    Resolver(const InternalNodeInfoPtr&, const Ice::CommunicatorPtr&);

    std::string operator()(const std::string&, const std::string& = std::string(), bool = true) const;
    Ice::StringSeq operator()(const Ice::StringSeq&, const std::string&) const;
    DistributionDescriptor operator()(const DistributionDescriptor&) const;
    PropertyDescriptorSeq operator()(const PropertyDescriptorSeq&, const std::string& = std::string("property")) const;
    PropertySetDescriptorDict operator()(const PropertySetDescriptorDict&) const;
    ObjectDescriptorSeq operator()(const ObjectDescriptorSeq&, const std::string&, const std::string&) const;
    Ice::Identity operator()(const Ice::Identity&, const std::string&) const;
    PropertySetDescriptor operator()(const PropertySetDescriptor&) const;

    std::string asInt(const std::string&, const std::string& = std::string()) const;
    std::string asFloat(const std::string&, const std::string& = std::string()) const;
    std::string asId(const std::string&, const std::string& = std::string(), bool = false) const;

    void setReserved(const std::string&, const std::string&);
    void setContext(const std::string&);
    void addPropertySets(const PropertySetDescriptorDict&);
    const PropertySetDescriptor& getPropertySet(const std::string&) const;
    PropertyDescriptorSeq getProperties(const Ice::StringSeq&) const;
    void addIgnored(const std::string&);

    void setVersion(const std::string&);
    int getVersion() const;

    void exception(const std::string&) const;

    TemplateDescriptor getServerTemplate(const std::string&) const;
    TemplateDescriptor getServiceTemplate(const std::string&) const;
    bool hasReplicaGroup(const std::string&) const;

    Ice::CommunicatorPtr getCommunicator() const { return _communicator; }
    bool warningEnabled() const { return _enableWarning; }

private:

    std::string substitute(const std::string&, bool, bool) const;
    std::string getVariable(const std::string&, bool, bool&) const;
    PropertyDescriptorSeq getProperties(const Ice::StringSeq&, std::set<std::string>&) const;

    static std::map<std::string, std::string> getReserved();
    void checkReserved(const std::string&, const std::map<std::string, std::string>&) const;
    void checkDeprecated(const std::string&) const;

    const ApplicationDescriptor* _application;
    const Ice::CommunicatorPtr _communicator;
    const bool _escape;
    const bool _enableWarning;
    std::string _context;
    std::map<std::string, std::string> _variables;
    std::map<std::string, std::string> _parameters;
    PropertySetDescriptorDict _propertySets;
    std::map<std::string, std::string> _reserved;
    std::map<std::string, std::string> _deprecated;
    std::set<std::string> _ignore;
    int _version;
};

class CommunicatorHelper
{
public:

    CommunicatorHelper(const CommunicatorDescriptorPtr&, bool = false);
    CommunicatorHelper() : _ignoreProps(false) { }
    virtual ~CommunicatorHelper() { }

    virtual bool operator==(const CommunicatorHelper&) const;
    virtual bool operator!=(const CommunicatorHelper&) const;

    virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    virtual void getReplicaGroups(std::set<std::string>&) const;

    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&) const;

protected:

    void printDbEnv(IceUtilInternal::Output&, const DbEnvDescriptor&) const;
    void printObjectAdapter(const Ice::CommunicatorPtr&, IceUtilInternal::Output&, const AdapterDescriptor&) const;
    void printPropertySet(IceUtilInternal::Output&, const PropertySetDescriptor&) const;
    virtual std::string getProperty(const std::string&) const;

    void instantiateImpl(const CommunicatorDescriptorPtr&, const Resolver&) const;

private:

    CommunicatorDescriptorPtr _desc;
    bool _ignoreProps;
};

class ServiceHelper : public CommunicatorHelper
{
public:

    ServiceHelper(const ServiceDescriptorPtr&, bool = false);
    ServiceHelper() { }

    virtual bool operator==(const CommunicatorHelper&) const;
    virtual bool operator!=(const CommunicatorHelper&) const;

    ServiceDescriptorPtr getDescriptor() const;
    ServiceDescriptorPtr instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                     const PropertySetDescriptorDict&) const;

    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&) const;

protected:

    using CommunicatorHelper::instantiateImpl;

    void instantiateImpl(const ServiceDescriptorPtr&, const Resolver&, const PropertyDescriptorSeq&,
                         const PropertySetDescriptorDict&) const;

private:

    ServiceDescriptorPtr _desc;
};

class ServerHelper : public CommunicatorHelper, public IceUtil::SimpleShared
{
public:

    ServerHelper(const ServerDescriptorPtr&, bool = false);
    ServerHelper() { }

    virtual bool operator==(const CommunicatorHelper&) const;
    virtual bool operator!=(const CommunicatorHelper&) const;

    ServerDescriptorPtr getDescriptor() const;
    virtual ServerDescriptorPtr instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                            const PropertySetDescriptorDict&) const;

    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&) const;
    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&, const ServerInfo&) const;

protected:

    using CommunicatorHelper::instantiateImpl;

    void printImpl(const Ice::CommunicatorPtr&, IceUtilInternal::Output&, const ServerInfo&) const;
    void instantiateImpl(const ServerDescriptorPtr&, const Resolver&, const PropertyDescriptorSeq&) const;

private:

    ServerDescriptorPtr _desc;
};
typedef IceUtil::Handle<ServerHelper> ServerHelperPtr;

class InstanceHelper
{
protected:

    std::map<std::string, std::string> instantiateParams(const Resolver&,
                                                         const std::string&,
                                                         const std::map<std::string, std::string>&,
                                                         const std::vector<std::string>&,
                                                         const std::map<std::string, std::string>&) const;
};

class ServiceInstanceHelper : public InstanceHelper
{
public:

    ServiceInstanceHelper(const ServiceInstanceDescriptor&, bool);

    bool operator==(const ServiceInstanceHelper&) const;
    bool operator!=(const ServiceInstanceHelper&) const;

    ServiceInstanceDescriptor instantiate(const Resolver&, const PropertySetDescriptorDict&) const;
    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;

    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&) const;

private:

    ServiceInstanceDescriptor _def;
    mutable ServiceHelper _service;
};

class IceBoxHelper : public ServerHelper
{
public:

    IceBoxHelper(const IceBoxDescriptorPtr&, bool = false);
    IceBoxHelper() { }

    virtual bool operator==(const CommunicatorHelper&) const;
    virtual bool operator!=(const CommunicatorHelper&) const;

    virtual ServerDescriptorPtr instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                            const PropertySetDescriptorDict&) const;

    virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    virtual void getReplicaGroups(std::set<std::string>&) const;

    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&) const;
    void print(const Ice::CommunicatorPtr&, IceUtilInternal::Output&, const ServerInfo&) const;

protected:

#ifdef __SUNPRO_CC
    using ServerHelper::instantiateImpl;
#endif

    void instantiateImpl(const IceBoxDescriptorPtr&, const Resolver&, const PropertyDescriptorSeq&,
                         const PropertySetDescriptorDict&) const;

private:

    IceBoxDescriptorPtr _desc;

    std::vector<ServiceInstanceHelper> _services;
};

class ServerInstanceHelper : public InstanceHelper
{
public:

    ServerInstanceHelper(const ServerInstanceDescriptor&, const Resolver&, bool);
    ServerInstanceHelper(const ServerDescriptorPtr&, const Resolver&, bool);

    bool operator==(const ServerInstanceHelper&) const;
    bool operator!=(const ServerInstanceHelper&) const;

    std::string getId() const;
    ServerInstanceDescriptor getDefinition() const;
    ServerInstanceDescriptor getInstance() const;

    ServerDescriptorPtr getServerDefinition() const;
    ServerDescriptorPtr getServerInstance() const;

    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;

private:

    void init(const ServerDescriptorPtr&, const Resolver&, bool);

    ServerInstanceDescriptor _def;
    std::string _id;
    ServerInstanceDescriptor _instance;

    ServerHelperPtr _serverDefinition;
    ServerHelperPtr _serverInstance;
};

class NodeHelper
{
public:

    NodeHelper(const std::string&, const NodeDescriptor&, const Resolver&, bool);
    virtual ~NodeHelper() { }

    virtual bool operator==(const NodeHelper&) const;
    virtual bool operator!=(const NodeHelper&) const;

    NodeUpdateDescriptor diff(const NodeHelper&) const;
    NodeDescriptor update(const NodeUpdateDescriptor&, const Resolver&) const;

    void getIds(std::multiset<std::string>&, std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;
    const NodeDescriptor& getDefinition() const;
    const NodeDescriptor& getInstance() const;
    void getServerInfos(const std::string&, const std::string&, int, std::map<std::string, ServerInfo>&) const;
    bool hasDistributions(const std::string&) const;
    bool hasServers() const;
    bool hasServer(const std::string&) const;
    void print(IceUtilInternal::Output&) const;
    void printDiff(IceUtilInternal::Output&, const NodeHelper&) const;

private:

    NodeDescriptor instantiate(const Resolver&) const;

    std::string _name;
    NodeDescriptor _def;
    NodeDescriptor _instance;
    bool _instantiated;

    typedef std::map<std::string, ServerInstanceHelper> ServerInstanceHelperDict;
    ServerInstanceHelperDict _serverInstances;
    ServerInstanceHelperDict _servers;
};

class ApplicationHelper
{
public:

    ApplicationHelper(const Ice::CommunicatorPtr&, const ApplicationDescriptor&, bool = false, bool = true);

    ApplicationUpdateDescriptor diff(const ApplicationHelper&) const;
    ApplicationDescriptor update(const ApplicationUpdateDescriptor&) const;
    ApplicationDescriptor instantiateServer(const std::string&, const ServerInstanceDescriptor&) const;

    void getIds(std::set<std::string>&, std::set<std::string>&, std::set<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&, std::set<std::string>&) const;
    const ApplicationDescriptor& getDefinition() const;
    const ApplicationDescriptor& getInstance() const;
    void getDistributions(DistributionDescriptor&, std::vector<std::string>&,const std::string& = std::string()) const;

    void print(IceUtilInternal::Output&, const ApplicationInfo&) const;
    void printDiff(IceUtilInternal::Output&, const ApplicationHelper&) const;
    std::map<std::string, ServerInfo> getServerInfos(const std::string&, int) const;

private:

    Ice::CommunicatorPtr _communicator;
    ApplicationDescriptor _def;
    ApplicationDescriptor _instance;

    typedef std::map<std::string, NodeHelper> NodeHelperDict;
    NodeHelperDict _nodes;
};

bool descriptorEqual(const ServerDescriptorPtr&, const ServerDescriptorPtr&, bool = false);
ServerHelperPtr createHelper(const ServerDescriptorPtr&);
bool isServerUpdated(const ServerInfo&, const ServerInfo&, bool = false);

}

#endif
