//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_DESCRIPTOR_BUILDER_H
#define ICE_GRID_DESCRIPTOR_BUILDER_H

#include <Ice/Logger.h>
#include <IceXML/Parser.h>
#include <IceGrid/Descriptor.h>
#include <set>

namespace IceGrid
{

class XmlAttributesHelper
{
public:

    XmlAttributesHelper(const IceXML::Attributes&, const std::shared_ptr<Ice::Logger>&, const std::string&, int);

    void checkUnknownAttributes();
    bool contains(const std::string&) const;
    std::map<std::string, std::string> asMap() const;

    bool asBool(const std::string&) const;
    bool asBool(const std::string&, bool) const;

    std::string operator()(const std::string&) const;
    std::string operator()(const std::string&, const std::string&) const;

private:

    const IceXML::Attributes& _attributes;
    const std::shared_ptr<Ice::Logger> _logger;
    const std::string _filename;
    const int _line;

    mutable std::set<std::string> _used;
};

class PropertySetDescriptorBuilder;

class DescriptorBuilder
{
public:

    virtual ~DescriptorBuilder() = default;

    virtual void addVariable(const XmlAttributesHelper&);
};

class PropertySetDescriptorBuilder : DescriptorBuilder
{
public:

    PropertySetDescriptorBuilder();

    void setId(const std::string&);
    void setService(const std::string&);

    const std::string& getId() const;
    const std::string& getService() const;
    const PropertySetDescriptor& getDescriptor() const;

    void addProperty(const XmlAttributesHelper&);
    void addPropertySet(const XmlAttributesHelper&);
    bool finish();

private:

    std::string _id;
    std::string _service;
    PropertySetDescriptor _descriptor;
    bool _inPropertySetRef;
};

class NodeDescriptorBuilder;
class TemplateDescriptorBuilder;

class ApplicationDescriptorBuilder : public DescriptorBuilder
{
public:

    ApplicationDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&, const XmlAttributesHelper&,
                                 const std::map<std::string, std::string>&);
    ApplicationDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&, const ApplicationDescriptor&,
                                 const XmlAttributesHelper&, const std::map<std::string, std::string>&);

    const ApplicationDescriptor& getDescriptor() const;

    void setVariableOverrides(const std::map<std::string, std::string>&);
    void setDescription(const std::string&);
    void addReplicaGroup(const XmlAttributesHelper&);
    void finishReplicaGroup();
    void setLoadBalancing(const XmlAttributesHelper&);
    void setReplicaGroupDescription(const std::string&);
    void addObject(const XmlAttributesHelper&);
    virtual void addVariable(const XmlAttributesHelper&);

    virtual NodeDescriptorBuilder* createNode(const XmlAttributesHelper&);
    virtual TemplateDescriptorBuilder* createServerTemplate(const XmlAttributesHelper&);
    virtual TemplateDescriptorBuilder* createServiceTemplate(const XmlAttributesHelper&);
    virtual PropertySetDescriptorBuilder* createPropertySet(const XmlAttributesHelper&) const;

    void addNode(const std::string&, const NodeDescriptor&);
    void addServerTemplate(const std::string&, const TemplateDescriptor&);
    void addServiceTemplate(const std::string&, const TemplateDescriptor&);
    void addPropertySet(const std::string&, const PropertySetDescriptor&);

    bool isOverride(const std::string&);

    const std::shared_ptr<Ice::Communicator>& getCommunicator() const { return _communicator; }

private:

    std::shared_ptr<Ice::Communicator> _communicator;
    ApplicationDescriptor _descriptor;
    std::map<std::string, std::string> _overrides;
};

class ServerDescriptorBuilder;
class IceBoxDescriptorBuilder;

class ServerInstanceDescriptorBuilder : public DescriptorBuilder
{
public:

    ServerInstanceDescriptorBuilder(const XmlAttributesHelper&);
    const ServerInstanceDescriptor& getDescriptor() const { return _descriptor; }

    virtual PropertySetDescriptorBuilder* createPropertySet(const XmlAttributesHelper& attrs) const;
    virtual void addPropertySet(const std::string&, const PropertySetDescriptor&);

private:

    ServerInstanceDescriptor _descriptor;
};

class NodeDescriptorBuilder : public DescriptorBuilder
{
public:

    NodeDescriptorBuilder(ApplicationDescriptorBuilder&, const XmlAttributesHelper&);
    NodeDescriptorBuilder(ApplicationDescriptorBuilder&, const NodeDescriptor&, const XmlAttributesHelper&);

    virtual ServerDescriptorBuilder* createServer(const XmlAttributesHelper&);
    virtual ServerDescriptorBuilder* createIceBox(const XmlAttributesHelper&);
    virtual ServerInstanceDescriptorBuilder* createServerInstance(const XmlAttributesHelper&);
    virtual PropertySetDescriptorBuilder* createPropertySet(const XmlAttributesHelper&) const;

    void addVariable(const XmlAttributesHelper&);
    void addServer(const std::shared_ptr<ServerDescriptor>&);
    void addServerInstance(const ServerInstanceDescriptor&);
    void addPropertySet(const std::string&, const PropertySetDescriptor&);
    void setDescription(const std::string&);

    const std::string& getName() const { return _name; }
    const NodeDescriptor& getDescriptor() const { return _descriptor; }

private:

    ApplicationDescriptorBuilder& _application;
    std::string _name;
    NodeDescriptor _descriptor;
};

class ServiceDescriptorBuilder;

class TemplateDescriptorBuilder : public DescriptorBuilder
{
public:

    TemplateDescriptorBuilder(ApplicationDescriptorBuilder&, const XmlAttributesHelper&, bool);

    virtual ServerDescriptorBuilder* createServer(const XmlAttributesHelper&);
    virtual ServerDescriptorBuilder* createIceBox(const XmlAttributesHelper&);
    virtual ServiceDescriptorBuilder* createService(const XmlAttributesHelper&);

    void addParameter(const XmlAttributesHelper&);
    void setDescriptor(const std::shared_ptr<CommunicatorDescriptor>&);

    const std::string& getId() const { return _id; }
    const TemplateDescriptor& getDescriptor() const { return _descriptor; }

protected:

    ApplicationDescriptorBuilder& _application;
    const bool _serviceTemplate;
    const std::string _id;
    TemplateDescriptor _descriptor;
};

class CommunicatorDescriptorBuilder : public DescriptorBuilder
{
public:

    CommunicatorDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&);

    void init(const std::shared_ptr<CommunicatorDescriptor>&, const XmlAttributesHelper&);
    virtual void finish();

    virtual void setDescription(const std::string&);
    virtual void addProperty(const XmlAttributesHelper&);
    virtual void addPropertySet(const PropertySetDescriptor&);
    virtual void addAdapter(const XmlAttributesHelper&);
    virtual void setAdapterDescription(const std::string&);
    virtual void addObject(const XmlAttributesHelper&);
    virtual void addAllocatable(const XmlAttributesHelper&);
    virtual void addLog(const XmlAttributesHelper&);

    virtual PropertySetDescriptorBuilder* createPropertySet() const;

protected:

    void addProperty(PropertyDescriptorSeq&, const std::string&, const std::string&);

    PropertyDescriptorSeq _hiddenProperties;
    std::shared_ptr<Ice::Communicator> _communicator;

private:

    std::shared_ptr<CommunicatorDescriptor> _descriptor;
};

class ServiceInstanceDescriptorBuilder : public DescriptorBuilder
{
public:

    ServiceInstanceDescriptorBuilder(const XmlAttributesHelper&);
    const ServiceInstanceDescriptor& getDescriptor() const { return _descriptor; }

    virtual PropertySetDescriptorBuilder* createPropertySet() const;
    virtual void addPropertySet(const PropertySetDescriptor&);

private:

    ServiceInstanceDescriptor _descriptor;
};

class ServerDescriptorBuilder : public CommunicatorDescriptorBuilder
{
public:

    ServerDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&, const XmlAttributesHelper&);
    ServerDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&);

    void init(const std::shared_ptr<ServerDescriptor>&, const XmlAttributesHelper&);

    virtual ServiceDescriptorBuilder* createService(const XmlAttributesHelper&);
    virtual ServiceInstanceDescriptorBuilder* createServiceInstance(const XmlAttributesHelper&);

    virtual void addOption(const std::string&);
    virtual void addEnv(const std::string&);
    virtual void addService(const std::shared_ptr<ServiceDescriptor>&);
    virtual void addServiceInstance(const ServiceInstanceDescriptor&);

    const std::shared_ptr<ServerDescriptor>& getDescriptor() const { return _descriptor; }

private:

    std::shared_ptr<ServerDescriptor> _descriptor;
};

class IceBoxDescriptorBuilder : public ServerDescriptorBuilder
{
public:

    IceBoxDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&, const XmlAttributesHelper&);

    void init(const std::shared_ptr<IceBoxDescriptor>&, const XmlAttributesHelper&);

    virtual ServiceDescriptorBuilder* createService(const XmlAttributesHelper&);
    virtual ServiceInstanceDescriptorBuilder* createServiceInstance(const XmlAttributesHelper&);

    virtual void addAdapter(const XmlAttributesHelper&);
    virtual void addServiceInstance(const ServiceInstanceDescriptor&);
    virtual void addService(const std::shared_ptr<ServiceDescriptor>&);

private:

    std::shared_ptr<IceBoxDescriptor> _descriptor;
};

class ServiceDescriptorBuilder : public CommunicatorDescriptorBuilder
{
public:

    ServiceDescriptorBuilder(const std::shared_ptr<Ice::Communicator>&, const XmlAttributesHelper&);
    void init(const std::shared_ptr<ServiceDescriptor>&, const XmlAttributesHelper&);

    const std::shared_ptr<ServiceDescriptor>& getDescriptor() const { return _descriptor; }

private:

    std::shared_ptr<ServiceDescriptor> _descriptor;
};

}

#endif
