//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    Resolver(const ApplicationDescriptor&, const std::shared_ptr<Ice::Communicator>&, bool);
    Resolver(const Resolver&, const std::map<std::string, std::string>&, bool);
    Resolver(const std::shared_ptr<InternalNodeInfo>&, const std::shared_ptr<Ice::Communicator>&);

    std::string operator()(const std::string&, const std::string& = std::string(), bool = true) const;
    Ice::StringSeq operator()(const Ice::StringSeq&, const std::string&) const;
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

    std::shared_ptr<Ice::Communicator> getCommunicator() const { return _communicator; }
    bool warningEnabled() const { return _enableWarning; }

private:

    std::string substitute(const std::string&, bool, bool) const;
    std::string getVariable(const std::string&, bool, bool&) const;
    PropertyDescriptorSeq getProperties(const Ice::StringSeq&, std::set<std::string>&) const;

    static std::map<std::string, std::string> getReserved();
    void checkReserved(const std::string&, const std::map<std::string, std::string>&) const;
    void checkDeprecated(const std::string&) const;

    const ApplicationDescriptor* _application;
    const std::shared_ptr<Ice::Communicator> _communicator;
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

    CommunicatorHelper() : _ignoreProps(false) { }
    CommunicatorHelper(const std::shared_ptr<CommunicatorDescriptor>&, bool = false);

    virtual bool operator==(const CommunicatorHelper&) const;
    bool operator!=(const CommunicatorHelper&) const;

    virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    virtual void getReplicaGroups(std::set<std::string>&) const;

    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&) const;

protected:

    void printObjectAdapter(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&, const AdapterDescriptor&) const;
    void printPropertySet(IceUtilInternal::Output&, const PropertySetDescriptor&) const;
    virtual std::string getProperty(const std::string&) const;

    void instantiateImpl(const std::shared_ptr<CommunicatorDescriptor>&, const Resolver&) const;

private:

    std::shared_ptr<CommunicatorDescriptor> _desc;
    bool _ignoreProps;
};

class ServiceHelper final : public CommunicatorHelper
{
public:

    ServiceHelper() = default;
    ServiceHelper(const std::shared_ptr<ServiceDescriptor>&, bool = false);

    bool operator==(const CommunicatorHelper&) const override;

    std::shared_ptr<ServiceDescriptor> getDescriptor() const;
    std::shared_ptr<ServiceDescriptor> instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                                   const PropertySetDescriptorDict&) const;

    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&) const;

protected:

    using CommunicatorHelper::instantiateImpl;

    void instantiateImpl(const std::shared_ptr<ServiceDescriptor>&, const Resolver&, const PropertyDescriptorSeq&,
                         const PropertySetDescriptorDict&) const;

private:

    std::shared_ptr<ServiceDescriptor> _desc;
};

class ServerHelper : public CommunicatorHelper
{
public:

    ServerHelper() = default;
    ServerHelper(const std::shared_ptr<ServerDescriptor>&, bool = false);
    virtual ~ServerHelper() = default;

    bool operator==(const CommunicatorHelper&) const override;

    std::shared_ptr<ServerDescriptor> getDescriptor() const;
    virtual std::shared_ptr<ServerDescriptor> instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                                          const PropertySetDescriptorDict&) const;

    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&) const;
    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&, const ServerInfo&) const;

protected:

    using CommunicatorHelper::instantiateImpl;

    void printImpl(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&, const ServerInfo&) const;
    void instantiateImpl(const std::shared_ptr<ServerDescriptor>&, const Resolver&, const PropertyDescriptorSeq&) const;

private:

    std::shared_ptr<ServerDescriptor> _desc;
};

class InstanceHelper
{
protected:

    std::map<std::string, std::string> instantiateParams(const Resolver&,
                                                         const std::string&,
                                                         const std::map<std::string, std::string>&,
                                                         const std::vector<std::string>&,
                                                         const std::map<std::string, std::string>&) const;
};

class ServiceInstanceHelper final : public InstanceHelper
{
public:

    ServiceInstanceHelper(const ServiceInstanceDescriptor&, bool);

    bool operator==(const ServiceInstanceHelper&) const;
    bool operator!=(const ServiceInstanceHelper&) const;

    ServiceInstanceDescriptor instantiate(const Resolver&, const PropertySetDescriptorDict&) const;
    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;

    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&) const;

private:

    ServiceInstanceDescriptor _def;
    mutable ServiceHelper _service;
};

class IceBoxHelper final : public ServerHelper
{
public:

    IceBoxHelper() = default;
    IceBoxHelper(const std::shared_ptr<IceBoxDescriptor>&, bool = false);

    bool operator==(const CommunicatorHelper&) const override;

    std::shared_ptr<ServerDescriptor> instantiate(const Resolver&, const PropertyDescriptorSeq&,
                                            const PropertySetDescriptorDict&) const override;

    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const override;
    void getReplicaGroups(std::set<std::string>&) const override;

    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&) const;
    void print(const std::shared_ptr<Ice::Communicator>&, IceUtilInternal::Output&, const ServerInfo&) const;

protected:

    void instantiateImpl(const std::shared_ptr<IceBoxDescriptor>&, const Resolver&, const PropertyDescriptorSeq&,
                         const PropertySetDescriptorDict&) const;

private:

    std::shared_ptr<IceBoxDescriptor> _desc;

    std::vector<ServiceInstanceHelper> _services;
};

class ServerInstanceHelper final : public InstanceHelper
{
public:

    ServerInstanceHelper(const ServerInstanceDescriptor&, const Resolver&, bool);
    ServerInstanceHelper(const std::shared_ptr<ServerDescriptor>&, const Resolver&, bool);

    bool operator==(const ServerInstanceHelper&) const;
    bool operator!=(const ServerInstanceHelper&) const;

    std::string getId() const;
    ServerInstanceDescriptor getDefinition() const;
    ServerInstanceDescriptor getInstance() const;

    std::shared_ptr<ServerDescriptor> getServerDefinition() const;
    std::shared_ptr<ServerDescriptor> getServerInstance() const;

    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;

private:

    void init(const std::shared_ptr<ServerDescriptor>&, const Resolver&, bool);

    ServerInstanceDescriptor _def;
    std::string _id;
    ServerInstanceDescriptor _instance;

    std::shared_ptr<ServerHelper> _serverDefinition;
    std::shared_ptr<ServerHelper> _serverInstance;
};

class NodeHelper final
{
public:

    NodeHelper(const std::string&, const NodeDescriptor&, const Resolver&, bool);

    bool operator==(const NodeHelper&) const;
    bool operator!=(const NodeHelper&) const;

    NodeUpdateDescriptor diff(const NodeHelper&) const;
    NodeDescriptor update(const NodeUpdateDescriptor&, const Resolver&) const;

    void getIds(std::multiset<std::string>&, std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&) const;
    const NodeDescriptor& getDefinition() const;
    const NodeDescriptor& getInstance() const;
    void getServerInfos(const std::string&, const std::string&, int, std::map<std::string, ServerInfo>&) const;
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

class ApplicationHelper final
{
public:

    ApplicationHelper(const std::shared_ptr<Ice::Communicator>&, const ApplicationDescriptor&, bool = false, bool = true);

    ApplicationUpdateDescriptor diff(const ApplicationHelper&) const;
    ApplicationDescriptor update(const ApplicationUpdateDescriptor&) const;
    ApplicationDescriptor instantiateServer(const std::string&, const ServerInstanceDescriptor&) const;

    void getIds(std::set<std::string>&, std::set<std::string>&, std::set<Ice::Identity>&) const;
    void getReplicaGroups(std::set<std::string>&, std::set<std::string>&) const;
    const ApplicationDescriptor& getDefinition() const;
    const ApplicationDescriptor& getInstance() const;

    void print(IceUtilInternal::Output&, const ApplicationInfo&) const;
    void printDiff(IceUtilInternal::Output&, const ApplicationHelper&) const;
    std::map<std::string, ServerInfo> getServerInfos(const std::string&, int) const;

private:

    std::shared_ptr<Ice::Communicator> _communicator;
    ApplicationDescriptor _def;
    ApplicationDescriptor _instance;

    using NodeHelperDict = std::map<std::string, NodeHelper>;
    NodeHelperDict _nodes;
};

bool descriptorEqual(const std::shared_ptr<ServerDescriptor>&, const std::shared_ptr<ServerDescriptor>&, bool = false);
std::shared_ptr<ServerHelper> createHelper(const std::shared_ptr<ServerDescriptor>&);
bool isServerUpdated(const ServerInfo&, const ServerInfo&, bool = false);

}

#endif
