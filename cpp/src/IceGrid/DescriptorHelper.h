// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_DESCRIPTOR_HELPER_H
#define ICE_GRID_DESCRIPTOR_HELPER_H

#include <IceUtil/OutputUtil.h>
#include <IceGrid/Admin.h>
#include <IceXML/Parser.h>

namespace IceGrid
{

class ApplicationHelper;
class Resolver
{
public:

    Resolver(const ApplicationHelper&, const std::string&, const std::map<std::string, std::string>&);
    Resolver(const Resolver&, const std::map<std::string, std::string>&, bool);

    std::string operator()(const std::string&, const std::string& = std::string(), bool = true, bool = true) const;
    std::string asInt(const std::string&, const std::string& = std::string()) const;
    void setReserved(const std::string&, const std::string&);
    void setContext(const std::string&);

    void exception(const std::string&) const;

    TemplateDescriptor getServerTemplate(const std::string&) const;
    TemplateDescriptor getServiceTemplate(const std::string&) const;
    PatchDescriptor getPatchDescriptor(const std::string&) const;
    
private:

    std::string substitute(const std::string&, bool = false) const;
    std::string getVariable(const std::string&, bool, bool&) const;

    const ApplicationHelper& _application;
    std::string _context;
    std::map<std::string, std::string> _variables;
    std::map<std::string, std::string> _parameters;
    std::map<std::string, std::string> _reserved;
};

class CommunicatorHelper
{
public:

    CommunicatorHelper(const CommunicatorDescriptorPtr&);
    CommunicatorHelper() { }
    virtual ~CommunicatorHelper() { }

    bool operator==(const CommunicatorHelper&) const;
    bool operator!=(const CommunicatorHelper&) const;

    virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    
    virtual void print(IceUtil::Output&) const;

protected:

    void printDbEnv(IceUtil::Output&, const DbEnvDescriptor&) const;
    void printObjectAdapter(IceUtil::Output&, const AdapterDescriptor&) const;
    void printProperties(IceUtil::Output&, const PropertyDescriptorSeq&) const;
    virtual std::string getProperty(const std::string&) const;

    void instantiateImpl(const CommunicatorDescriptorPtr&, const Resolver&) const;
    
private:

    CommunicatorDescriptorPtr _desc;
};

class ServiceHelper : public CommunicatorHelper
{
public:

    ServiceHelper(const ServiceDescriptorPtr&);
    ServiceHelper() { }

    bool operator==(const ServiceHelper&) const;
    bool operator!=(const ServiceHelper&) const;    

    ServiceDescriptorPtr getDescriptor() const;
    ServiceDescriptorPtr instantiate(const Resolver&) const;

    void print(IceUtil::Output&) const;

protected:

    void instantiateImpl(const ServiceDescriptorPtr&, const Resolver&) const;

private:
    
    ServiceDescriptorPtr _desc;
};

class ServerHelper : public CommunicatorHelper, public IceUtil::SimpleShared
{
public:

    ServerHelper(const ServerDescriptorPtr&);
    ServerHelper() { }

    bool operator==(const ServerHelper&) const;
    bool operator!=(const ServerHelper&) const;    

    ServerDescriptorPtr getDescriptor() const;
    virtual ServerDescriptorPtr instantiate(const Resolver&) const;

    void print(IceUtil::Output&, const std::string& = std::string(), const std::string& = std::string()) const;

protected:

    void printImpl(IceUtil::Output&, const std::string&, const std::string&) const;
    void instantiateImpl(const ServerDescriptorPtr&, const Resolver&) const;

private:
    
    ServerDescriptorPtr _desc;
};
typedef IceUtil::Handle<ServerHelper> ServerHelperPtr;

class ServiceInstanceHelper;

class IceBoxHelper : public ServerHelper
{
public:

    IceBoxHelper(const IceBoxDescriptorPtr&, const Resolver&);
    IceBoxHelper(const IceBoxDescriptorPtr&);
    IceBoxHelper() { }

    bool operator==(const IceBoxHelper&) const;
    bool operator!=(const IceBoxHelper&) const;    

    virtual ServerDescriptorPtr instantiate(const Resolver&) const;

    virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;

    void print(IceUtil::Output&, const std::string& = std::string(), const std::string& = std::string()) const;

protected:

    void instantiateImpl(const IceBoxDescriptorPtr&, const Resolver&) const;

private:
    
    IceBoxDescriptorPtr _desc;

    typedef std::vector<ServiceInstanceHelper> ServiceInstanceHelperSeq;
    ServiceInstanceHelperSeq _services;
};

class InstanceHelper
{
protected:

    std::map<std::string, std::string> instantiateParams(const Resolver&, const std::string&, 
							 const std::map<std::string, std::string>&,
							 const std::vector<std::string>&);
};

class ServiceInstanceHelper : public InstanceHelper
{
public:

    ServiceInstanceHelper(const ServiceInstanceDescriptor&, const Resolver&);
    ServiceInstanceHelper(const ServiceInstanceDescriptor&);
    ServiceInstanceHelper() { }

    bool operator==(const ServiceInstanceHelper&) const;
    bool operator!=(const ServiceInstanceHelper&) const;

    ServiceInstanceDescriptor getDescriptor() const;
    ServiceInstanceDescriptor getInstance() const;
    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;

    void print(IceUtil::Output&) const;

private:
    
    std::string _template;
    std::map<std::string, std::string> _parameters;
    ServiceHelper _definition;
    ServiceHelper _instance;
};

class ServerInstanceHelper : InstanceHelper
{
public:

    ServerInstanceHelper(const ServerInstanceDescriptor&, const Resolver&);
    ServerInstanceHelper(const ServerDescriptorPtr&, const Resolver&);
    ServerInstanceHelper() { }
    
    void operator=(const ServerInstanceHelper&);
    bool operator==(const ServerInstanceHelper&) const;
    bool operator!=(const ServerInstanceHelper&) const;

    std::string getId() const;
    bool getPatchDirs(const std::string&, const Resolver&, std::set<std::string>&) const;
    ServerInstanceDescriptor getDescriptor() const;
    ServerDescriptorPtr getDefinition() const;
    ServerDescriptorPtr getInstance() const;
    const ServerHelper& getInstanceHelper() const;

    void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;

private:

    void init(const ServerDescriptorPtr&, const Resolver&);

    const std::string _template;
    const std::map<std::string, std::string> _parameters;
    ServerHelperPtr _definition;
    ServerHelperPtr _instance;
};

class NodeHelper
{
public:

    NodeHelper(const std::string&, const NodeDescriptor&, const Resolver&);
    NodeHelper() { }
    virtual ~NodeHelper() { }

    bool operator==(const NodeHelper&) const;
    bool operator!=(const NodeHelper&) const;

    NodeUpdateDescriptor diff(const NodeHelper&) const;
    void update(const NodeUpdateDescriptor&, const Resolver&);

    void getIds(std::multiset<std::string>&, std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
    const NodeDescriptor& getDescriptor() const;
    void getServerInfos(const std::string&, std::map<std::string, ServerInfo>&) const;
    std::pair<Ice::StringSeq, Ice::StringSeq> getPatchDirs(const std::string&, const Resolver&) const;

    void print(IceUtil::Output&) const;
    void printDiff(IceUtil::Output&, const NodeHelper&) const;
    void validate(const Resolver&) const;

private:

    std::string _name;
    NodeDescriptor _desc;

    typedef std::map<std::string, ServerInstanceHelper> ServerInstanceHelperDict;
    ServerInstanceHelperDict _serverInstances;
    ServerInstanceHelperDict _servers;
};

class ApplicationHelper
{
public:

    ApplicationHelper(const ApplicationDescriptor&);

    ApplicationUpdateDescriptor diff(const ApplicationHelper&);
    void update(const ApplicationUpdateDescriptor&);    

    void getIds(std::set<std::string>&, std::set<std::string>&, std::set<Ice::Identity>&) const;
    const ApplicationDescriptor& getDescriptor() const;
    TemplateDescriptor getServerTemplate(const std::string&) const;
    TemplateDescriptor getServiceTemplate(const std::string&) const;
    PatchDescriptor getPatchDescriptor(const std::string&) const;

    void print(IceUtil::Output&) const;
    void printDiff(IceUtil::Output&, const ApplicationHelper&) const;
    std::map<std::string, ServerInfo> getServerInfos() const;
    std::map<std::string, std::pair<Ice::StringSeq, Ice::StringSeq> > getNodesPatchDirs(const std::string&) const;

private:

    void validate(const Resolver&) const;

    ApplicationDescriptor _desc;

    typedef std::map<std::string, NodeHelper> NodeHelperDict;
    NodeHelperDict _nodes;
};

}

#endif
