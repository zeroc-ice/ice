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

#include <IceGrid/Admin.h>
#include <IceXML/Parser.h>

namespace IceGrid
{

class DescriptorVariables : public IceUtil::SimpleShared
{
public:

    DescriptorVariables();
    DescriptorVariables(const std::map<std::string, std::string>&);

    std::string substitute(const std::string&);
    std::string substituteWithMissing(const std::string&, std::set<std::string>&);
    std::string getVariable(const std::string&);
    void addVariable(const std::string&, const std::string&);
    bool hasVariable(const std::string&) const;
    void remove(const std::string&);
    void dumpVariables() const;

    void reset(const std::map<std::string, std::string>&, const std::vector<std::string>&);
    void push(const std::map<std::string, std::string>&);
    void push();
    void pop();
    std::map<std::string, std::string> getCurrentScopeVariables() const;
    std::vector<std::string> getCurrentScopeParameters() const;
    void addParameter(const std::string&);

    std::vector<std::string> getDeploymentTargets(const std::string&) const;

    void substitution(bool);
    bool substitution() const;

private:

    std::string substituteImpl(const std::string&, std::set<std::string>&);

    struct VariableScope
    {
	std::map<std::string, std::string> variables;
	std::set<std::string> used;
	std::set<std::string> parameters;
	bool substitution;
    };
    std::vector<VariableScope> _scopes;
    std::vector<std::string> _deploymentTargets;
};
typedef IceUtil::Handle<DescriptorVariables> DescriptorVariablesPtr;

class DescriptorHelper;
class ServerDescriptorHelper;
class ServiceDescriptorHelper;

class DescriptorTemplates : public IceUtil::SimpleShared
{
public:

    DescriptorTemplates(const ApplicationDescriptorPtr&);

    ServerDescriptorPtr instantiateServer(const DescriptorHelper&, const std::string&, 
					  const std::map<std::string, std::string>&);

    ServiceDescriptorPtr instantiateService(const DescriptorHelper&, const std::string&,
					    const std::map<std::string, std::string>&);
    
    void addServerTemplate(const std::string&, const ServerDescriptorPtr&, const Ice::StringSeq&);
    void addServiceTemplate(const std::string&, const ServiceDescriptorPtr&, const Ice::StringSeq&);

    ApplicationDescriptorPtr getApplicationDescriptor() const;

private:

    const ApplicationDescriptorPtr _application;
};
typedef IceUtil::Handle<DescriptorTemplates> DescriptorTemplatesPtr;

class XmlAttributesHelper
{
public:

    XmlAttributesHelper(const DescriptorVariablesPtr&, const IceXML::Attributes&);

    bool contains(const std::string&);
    std::string operator()(const std::string&);
    std::string operator()(const std::string&, const std::string&);

private:
    
    const DescriptorVariablesPtr& _variables;
    const IceXML::Attributes& _attributes;
};

class DescriptorHelper
{
public:

    const DescriptorVariablesPtr& getVariables() const;

protected:

    DescriptorHelper(const Ice::CommunicatorPtr&, const DescriptorVariablesPtr&, const DescriptorTemplatesPtr&);
    DescriptorHelper(const DescriptorHelper&);

    virtual ~DescriptorHelper();

    const Ice::CommunicatorPtr _communicator;
    const DescriptorVariablesPtr _variables;
    const DescriptorTemplatesPtr _templates;
};

class ServerDescriptorHelper;
class ServiceDescriptorHelper;

class ApplicationDescriptorHelper : public DescriptorHelper
{
public:

    ApplicationDescriptorHelper(const Ice::CommunicatorPtr&, const ApplicationDescriptorPtr&);
    ApplicationDescriptorHelper(const Ice::CommunicatorPtr&, const DescriptorVariablesPtr&, const IceXML::Attributes&);

    void endParsing();

    const ApplicationDescriptorPtr& getDescriptor() const;
    void setComment(const std::string&);

    void addNode(const IceXML::Attributes&);
    void endNodeParsing();

    void addServer(const std::string&, const IceXML::Attributes&);
    void addServer(const ServerDescriptorPtr&);
    std::auto_ptr<ServerDescriptorHelper> addServerTemplate(const std::string&, const IceXML::Attributes&);
    std::auto_ptr<ServiceDescriptorHelper> addServiceTemplate(const std::string&, const IceXML::Attributes&);

private:

    ApplicationDescriptorPtr _descriptor;
};

class ComponentDescriptorHelper : public DescriptorHelper
{
public:
    
    ComponentDescriptorHelper(const DescriptorHelper&);
    ComponentDescriptorHelper(const Ice::CommunicatorPtr&, const DescriptorVariablesPtr&, 
			      const DescriptorTemplatesPtr&);

    bool operator==(const ComponentDescriptorHelper&) const;
    bool operator!=(const ComponentDescriptorHelper&) const;

    void setComment(const std::string&);
    void addProperty(const IceXML::Attributes&);
    void addAdapter(const IceXML::Attributes&);
    void addObject(const IceXML::Attributes&);
    void addDbEnv(const IceXML::Attributes&);
    void addDbEnvProperty(const IceXML::Attributes&);

protected:
    
    void init(const ComponentDescriptorPtr&, const IceXML::Attributes& = IceXML::Attributes());
    virtual void instantiateImpl(const ComponentDescriptorPtr&, std::set<std::string>&) const;

private:

    ComponentDescriptorPtr _descriptor;
};

class ServerDescriptorHelper : public ComponentDescriptorHelper
{
public:
    
    ServerDescriptorHelper(const DescriptorHelper&, const ServerDescriptorPtr&);
    ServerDescriptorHelper(const DescriptorHelper&, const IceXML::Attributes&, const std::string&);
    ~ServerDescriptorHelper();

    void endParsing();

    bool operator==(const ServerDescriptorHelper&) const;
    bool operator!=(const ServerDescriptorHelper&) const;

    virtual ServerDescriptorPtr instantiate(std::set<std::string>&) const;
    const ServerDescriptorPtr& getDescriptor() const;
    const std::string& getTemplateId() const;

    void addService(const std::string&, const IceXML::Attributes&);
    void addService(const ServiceDescriptorPtr&);
    std::auto_ptr<ServiceDescriptorHelper> addServiceTemplate(const std::string&, const IceXML::Attributes&);

    void addOption(const std::string&);
    void addEnv(const std::string&);
    void addInterpreterOption(const std::string&);

private:

    virtual void instantiateImpl(const ServerDescriptorPtr&, std::set<std::string>&) const;

    ServerDescriptorPtr _descriptor;
    std::string _templateId;
};

class ServiceDescriptorHelper : public ComponentDescriptorHelper
{
public:

    ServiceDescriptorHelper(const DescriptorHelper&, const ServiceDescriptorPtr&);
    ServiceDescriptorHelper(const DescriptorHelper&, const IceXML::Attributes&, const std::string&);
    ~ServiceDescriptorHelper();

    void endParsing();

    bool operator==(const ServiceDescriptorHelper&) const;
    bool operator!=(const ServiceDescriptorHelper&) const;

    virtual ServiceDescriptorPtr instantiate(std::set<std::string>&) const;
    const ServiceDescriptorPtr& getDescriptor() const;
    const std::string& getTemplateId() const;

private:

    virtual void instantiateImpl(const ServiceDescriptorPtr&, std::set<std::string>&) const;

    ServiceDescriptorPtr _descriptor;
    std::string _templateId;
};

}

#endif
