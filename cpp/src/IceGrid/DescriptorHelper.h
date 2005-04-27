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

    std::string substitute(const std::string&) const;
    std::string substituteWithMissing(const std::string&, std::vector<std::string>&) const;
    std::string getVariable(const std::string&) const;
    bool hasVariable(const std::string&) const;
    void remove(const std::string&);
    void dumpVariables() const;

    void reset(const std::map<std::string, std::string>&);
    void push(const std::map<std::string, std::string>&);
    void push();
    void pop();
    std::string& operator[](const std::string&);

    void ignoreMissing(bool);

private:

    std::string substituteImpl(const std::string&, bool, std::vector<std::string>&) const;

    std::vector<std::map<std::string, std::string> > _variables;
    bool _ignoreMissing;
};
typedef IceUtil::Handle<DescriptorVariables> DescriptorVariablesPtr;

class DescriptorHelper;
class DescriptorTemplates : public IceUtil::SimpleShared
{
public:

    DescriptorTemplates(const ApplicationDescriptorPtr& = ApplicationDescriptorPtr());

    ServerDescriptorPtr instantiateServer(const DescriptorHelper&, const std::string&, 
					  const std::map<std::string, std::string>&);

    ServiceDescriptorPtr instantiateService(const DescriptorHelper&, const std::string&,
					    const std::map<std::string, std::string>&);
    
    void addServerTemplate(const std::string&, const ServerDescriptorPtr&);
    void addServiceTemplate(const std::string&, const ServiceDescriptorPtr&);

private:

    std::map<std::string, ServerDescriptorPtr> _serverTemplates;
    std::map<std::string, ServiceDescriptorPtr> _serviceTemplates;
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

    const ApplicationDescriptorPtr& getDescriptor() const;
    void setComment(const std::string&);
    ServerDescriptorHelper* addServer(const IceXML::Attributes&);
    ServerDescriptorHelper* addServerTemplate(const IceXML::Attributes&);
    ServiceDescriptorHelper* addServiceTemplate(const IceXML::Attributes&);

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

    void setComment(const std::string&);
    void addProperty(const IceXML::Attributes&);
    void addAdapter(const IceXML::Attributes&);
    void addObject(const IceXML::Attributes&);
    void addDbEnv(const IceXML::Attributes&);
    void addDbEnvProperty(const IceXML::Attributes&);

protected:
    
    void init(const ComponentDescriptorPtr&, const IceXML::Attributes& = IceXML::Attributes());
    virtual void instantiateImpl(const ComponentDescriptorPtr&, std::vector<std::string>&) const;

private:

    ComponentDescriptorPtr _descriptor;
};

class ServerDescriptorHelper : public ComponentDescriptorHelper
{
public:
    
    ServerDescriptorHelper(const DescriptorHelper&, const ServerDescriptorPtr&);
    ServerDescriptorHelper(const DescriptorHelper&, const IceXML::Attributes&);
    ServerDescriptorHelper(const Ice::CommunicatorPtr&, const DescriptorVariablesPtr&, const IceXML::Attributes&);

    bool operator==(const ServerDescriptorHelper&) const;
    virtual ServerDescriptorPtr instantiate(std::vector<std::string>&) const;
    const ServerDescriptorPtr& getDescriptor() const;

    ServiceDescriptorHelper* addService(const IceXML::Attributes&);
    void addOption(const std::string&);
    void addEnv(const std::string&);
    void addJvmOption(const std::string&);

private:

    void initFromXml(const IceXML::Attributes&);
    virtual void instantiateImpl(const ServerDescriptorPtr&, std::vector<std::string>&) const;

    ServerDescriptorPtr _descriptor;
};

class ServiceDescriptorHelper : public ComponentDescriptorHelper
{
public:

    ServiceDescriptorHelper(const DescriptorHelper&, const ServiceDescriptorPtr&);
    ServiceDescriptorHelper(const DescriptorHelper&, const IceXML::Attributes&);

    bool operator==(const ServiceDescriptorHelper&) const;
    virtual ServiceDescriptorPtr instantiate(std::vector<std::string>&) const;
    const ServiceDescriptorPtr& getDescriptor() const;

private:

    virtual void instantiateImpl(const ServiceDescriptorPtr&, std::vector<std::string>&) const;

    ServiceDescriptorPtr _descriptor;
};

}

#endif
