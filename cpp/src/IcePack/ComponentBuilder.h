// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_PACK_COMPONENT_BUILDER_H
#define ICE_PACK_COMPONENT_BUILDER_H

#include <IceUtil/Shared.h>
#include <IceXML/Parser.h>
#include <IcePack/Internal.h>

#include <stack>

namespace IcePack
{

//
// A deployment task. A deployment task is executed when a component
// is deployed and it's undo when the component is removed.
//
class Task : public ::IceUtil::SimpleShared
{
public:

    virtual void execute() = 0;
    virtual void undo() = 0;
};

typedef ::IceUtil::Handle< ::IcePack::Task> TaskPtr;

class ComponentBuilder;

//
// SAX parser handler for component descriptors.
//
class ComponentHandler : public IceXML::Handler
{
public:

    ComponentHandler(ComponentBuilder&);

    virtual void startElement(const std::string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const std::string&, int, int);
    virtual void characters(const std::string&, int, int);

protected:

    std::string getAttributeValue(const IceXML::Attributes&, const std::string&) const;
    std::string getAttributeValueWithDefault(const IceXML::Attributes&, const std::string&,
                                             const std::string&) const;

    std::string elementValue() const;
    bool isCurrentTargetDeployable() const;

    ComponentBuilder& _builder;
    std::stack<std::string> _elements;
    std::string _currentAdapterId;
    std::string _currentTarget;
    bool _isCurrentTargetDeployable;
};

//
// The component builder builds and execute the tasks that need to be
// executed to deploy a component described in a component
// descriptor. There's two phase to deploy or remove a component:
//
// * descriptor parsing: the builder parse() method initiate the
//   parsing. The parser will call the builder methods to setup the
//   deployment tasks. Once the parsing is finished all the deployment
//   tasks should be ready to be executed or undo
//
// * execution of the tasks to deploy the component or undo'ing of the
//   tasks to remove the component.
//
class ComponentBuilder : public Task
{
public:

    ComponentBuilder(const Ice::CommunicatorPtr&,
		     const std::map<std::string, std::string>&,
		     const std::vector<std::string>&);

    virtual void execute();
    virtual void undo();

    void parse(const std::string&, ComponentHandler&);

    bool isTargetDeployable(const std::string&) const;

    void createDirectory(const std::string&, bool = false);
    void createConfigFile(const std::string&);
    void addProperty(const std::string&, const std::string&);
    void addVariable(const std::string&, const std::string&);
    void addObject(const std::string&, const std::string&, const std::string&);
    void overrideBaseDir(const std::string&);

    const std::string& getVariable(const std::string&) const;
    void setVariable(const std::string&, const std::string&);
    bool findVariable(const std::string&) const;
    void pushVariables();
    void popVariables();

    virtual std::string getDefaultAdapterId(const std::string&);
    std::string toLocation(const std::string&) const;
    std::string substitute(const std::string&) const;
    std::vector<std::string> toTargets(const std::string&) const;
    void undoFrom(std::vector<TaskPtr>::iterator);

protected:

    void generateConfigFile(const std::string&, const std::string&, const std::string&, const Ice::PropertiesPtr&);
    
    Ice::CommunicatorPtr _communicator;
    ObjectRegistryPrx _objectRegistry;

    Ice::PropertiesPtr _properties;
    std::vector< std::map<std::string, std::string> > _variables;
    std::vector<TaskPtr> _tasks;
    std::string _configFile;
    std::vector<std::string> _targets;
};

}

#endif
