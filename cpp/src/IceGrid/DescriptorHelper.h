// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_DESCRIPTOR_HELPER_H
#define ICEGRID_DESCRIPTOR_HELPER_H

#include "../Ice/OutputUtil.h"
#include "IceGrid/Admin.h"
#include "Internal.h"
#include "XMLParser.h"
#include <set>

namespace IceGrid
{
    class ApplicationHelper;
    class Resolver
    {
    public:
        Resolver(const ApplicationDescriptor&, const Ice::CommunicatorPtr&, bool);
        Resolver(const Resolver&, const std::map<std::string, std::string>&, bool);
        Resolver(const std::shared_ptr<InternalNodeInfo>&, const Ice::CommunicatorPtr&);

        std::string operator()(const std::string&, const std::string& = std::string(), bool = true) const;
        Ice::StringSeq operator()(const Ice::StringSeq&, const std::string&) const;
        PropertyDescriptorSeq
        operator()(const PropertyDescriptorSeq&, const std::string& = std::string("property")) const;
        PropertySetDescriptorDict operator()(const PropertySetDescriptorDict&) const;
        ObjectDescriptorSeq operator()(const ObjectDescriptorSeq&, const std::string&, const std::string&) const;
        Ice::Identity operator()(const Ice::Identity&, const std::string&) const;
        PropertySetDescriptor operator()(const PropertySetDescriptor&) const;

        [[nodiscard]] std::string asInt(const std::string&, const std::string& = std::string()) const;
        [[nodiscard]] std::string asFloat(const std::string&, const std::string& = std::string()) const;
        [[nodiscard]] std::string asId(const std::string&, const std::string& = std::string(), bool = false) const;

        void setReserved(const std::string&, const std::string&);
        void setContext(const std::string&);
        void addPropertySets(const PropertySetDescriptorDict&);
        [[nodiscard]] const PropertySetDescriptor& getPropertySet(const std::string&) const;
        [[nodiscard]] PropertyDescriptorSeq getProperties(const Ice::StringSeq&) const;
        void addIgnored(const std::string&);

        void setVersion(const std::string&);
        [[nodiscard]] int getVersion() const;

        void exception(const std::string&) const;

        [[nodiscard]] TemplateDescriptor getServerTemplate(const std::string&) const;
        [[nodiscard]] TemplateDescriptor getServiceTemplate(const std::string&) const;
        [[nodiscard]] bool hasReplicaGroup(const std::string&) const;

        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const { return _communicator; }
        [[nodiscard]] bool warningEnabled() const { return _enableWarning; }

    private:
        [[nodiscard]] std::string substitute(const std::string&, bool, bool) const;
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
        CommunicatorHelper() : _ignoreProps(false) {}
        CommunicatorHelper(const std::shared_ptr<CommunicatorDescriptor>&, bool = false);

        virtual bool operator==(const CommunicatorHelper&) const;
        bool operator!=(const CommunicatorHelper&) const;

        virtual void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
        virtual void getReplicaGroups(std::set<std::string>&) const;

        void print(const Ice::CommunicatorPtr&, IceInternal::Output&) const;

    protected:
        void printObjectAdapter(const Ice::CommunicatorPtr&, IceInternal::Output&, const AdapterDescriptor&) const;
        void printPropertySet(IceInternal::Output&, const PropertySetDescriptor&) const;
        [[nodiscard]] virtual std::string getProperty(const std::string&) const;

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

        [[nodiscard]] std::shared_ptr<ServiceDescriptor> getDescriptor() const;
        [[nodiscard]] std::shared_ptr<ServiceDescriptor>
        instantiate(const Resolver&, const PropertyDescriptorSeq&, const PropertySetDescriptorDict&) const;

        void print(const Ice::CommunicatorPtr&, IceInternal::Output&) const;

    protected:
        using CommunicatorHelper::instantiateImpl;

        void instantiateImpl(
            const std::shared_ptr<ServiceDescriptor>&,
            const Resolver&,
            const PropertyDescriptorSeq&,
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

        [[nodiscard]] std::shared_ptr<ServerDescriptor> getDescriptor() const;
        [[nodiscard]] virtual std::shared_ptr<ServerDescriptor>
        instantiate(const Resolver&, const PropertyDescriptorSeq&, const PropertySetDescriptorDict&) const;

        void print(const Ice::CommunicatorPtr&, IceInternal::Output&) const;
        void print(const Ice::CommunicatorPtr&, IceInternal::Output&, const ServerInfo&) const;

    protected:
        using CommunicatorHelper::instantiateImpl;

        void printImpl(const Ice::CommunicatorPtr&, IceInternal::Output&, const ServerInfo&) const;
        void
        instantiateImpl(const std::shared_ptr<ServerDescriptor>&, const Resolver&, const PropertyDescriptorSeq&) const;

    private:
        std::shared_ptr<ServerDescriptor> _desc;
    };

    class InstanceHelper
    {
    protected:
        [[nodiscard]] std::map<std::string, std::string> instantiateParams(
            const Resolver&,
            const std::string&,
            const std::map<std::string, std::string>&,
            const std::vector<std::string>&,
            const std::map<std::string, std::string>&) const;
    };

    class ServiceInstanceHelper final : public InstanceHelper
    {
    public:
        ServiceInstanceHelper(ServiceInstanceDescriptor, bool);

        bool operator==(const ServiceInstanceHelper&) const;
        bool operator!=(const ServiceInstanceHelper&) const;

        [[nodiscard]] ServiceInstanceDescriptor instantiate(const Resolver&, const PropertySetDescriptorDict&) const;
        void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
        void getReplicaGroups(std::set<std::string>&) const;

        void print(const Ice::CommunicatorPtr&, IceInternal::Output&) const;

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

        [[nodiscard]] std::shared_ptr<ServerDescriptor>
        instantiate(const Resolver&, const PropertyDescriptorSeq&, const PropertySetDescriptorDict&) const override;

        void getIds(std::multiset<std::string>&, std::multiset<Ice::Identity>&) const override;
        void getReplicaGroups(std::set<std::string>&) const override;

        void print(const Ice::CommunicatorPtr&, IceInternal::Output&) const;
        void print(const Ice::CommunicatorPtr&, IceInternal::Output&, const ServerInfo&) const;

    protected:
        void instantiateImpl(
            const std::shared_ptr<IceBoxDescriptor>&,
            const Resolver&,
            const PropertyDescriptorSeq&,
            const PropertySetDescriptorDict&) const;

    private:
        std::shared_ptr<IceBoxDescriptor> _desc;

        std::vector<ServiceInstanceHelper> _services;
    };

    class ServerInstanceHelper final : public InstanceHelper
    {
    public:
        ServerInstanceHelper(ServerInstanceDescriptor, const Resolver&, bool);
        ServerInstanceHelper(const std::shared_ptr<ServerDescriptor>&, const Resolver&, bool);

        bool operator==(const ServerInstanceHelper&) const;
        bool operator!=(const ServerInstanceHelper&) const;

        [[nodiscard]] std::string getId() const;
        [[nodiscard]] ServerInstanceDescriptor getDefinition() const;
        [[nodiscard]] ServerInstanceDescriptor getInstance() const;

        [[nodiscard]] std::shared_ptr<ServerDescriptor> getServerDefinition() const;
        [[nodiscard]] std::shared_ptr<ServerDescriptor> getServerInstance() const;

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
        NodeHelper(std::string, NodeDescriptor, const Resolver&, bool);

        bool operator==(const NodeHelper&) const;
        bool operator!=(const NodeHelper&) const;

        [[nodiscard]] NodeUpdateDescriptor diff(const NodeHelper&) const;
        [[nodiscard]] NodeDescriptor update(const NodeUpdateDescriptor&, const Resolver&) const;

        void getIds(std::multiset<std::string>&, std::multiset<std::string>&, std::multiset<Ice::Identity>&) const;
        void getReplicaGroups(std::set<std::string>&) const;
        [[nodiscard]] const NodeDescriptor& getDefinition() const;
        [[nodiscard]] const NodeDescriptor& getInstance() const;
        void getServerInfos(const std::string&, const std::string&, int, std::map<std::string, ServerInfo>&) const;
        [[nodiscard]] bool hasServers() const;
        [[nodiscard]] bool hasServer(const std::string&) const;
        void print(IceInternal::Output&) const;
        void printDiff(IceInternal::Output&, const NodeHelper&) const;

    private:
        [[nodiscard]] NodeDescriptor instantiate(const Resolver&) const;

        std::string _name;
        NodeDescriptor _def;
        NodeDescriptor _instance;
        bool _instantiated;

        using ServerInstanceHelperDict = std::map<std::string, ServerInstanceHelper>;
        ServerInstanceHelperDict _serverInstances;
        ServerInstanceHelperDict _servers;
    };

    class ApplicationHelper final
    {
    public:
        ApplicationHelper(const Ice::CommunicatorPtr&, ApplicationDescriptor, bool = false, bool = true);

        [[nodiscard]] ApplicationUpdateDescriptor diff(const ApplicationHelper&) const;
        [[nodiscard]] ApplicationDescriptor update(const ApplicationUpdateDescriptor&) const;
        [[nodiscard]] ApplicationDescriptor
        instantiateServer(const std::string&, const ServerInstanceDescriptor&) const;

        void getIds(std::set<std::string>&, std::set<std::string>&, std::set<Ice::Identity>&) const;
        void getReplicaGroups(std::set<std::string>&, std::set<std::string>&) const;
        [[nodiscard]] const ApplicationDescriptor& getDefinition() const;
        [[nodiscard]] const ApplicationDescriptor& getInstance() const;

        void print(IceInternal::Output&, const ApplicationInfo&) const;
        void printDiff(IceInternal::Output&, const ApplicationHelper&) const;
        [[nodiscard]] std::map<std::string, ServerInfo> getServerInfos(const std::string&, int) const;

    private:
        Ice::CommunicatorPtr _communicator;
        ApplicationDescriptor _def;
        ApplicationDescriptor _instance;

        using NodeHelperDict = std::map<std::string, NodeHelper>;
        NodeHelperDict _nodes;
    };

    bool
    descriptorEqual(const std::shared_ptr<ServerDescriptor>&, const std::shared_ptr<ServerDescriptor>&, bool = false);
    std::shared_ptr<ServerHelper> createHelper(const std::shared_ptr<ServerDescriptor>&);
    bool isServerUpdated(const ServerInfo&, const ServerInfo&, bool = false);
}

#endif
