// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_COMPONENT_BUILDER_H
#define ICE_PACK_COMPONENT_BUILDER_H

#include <IceUtil/Shared.h>
#include <IcePack/Admin.h>
#include <Yellow/Yellow.h>

#include <xercesc/sax/HandlerBase.hpp>

#include <map>
#include <vector>
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

class DeploySAXParseException : public SAXParseException
{
public:

    DeploySAXParseException(const std::string&, const Locator* locator);

};

//
// A wrapper for ParserDeploymentException.
//
class ParserDeploymentWrapperException : public SAXException
{
public:

    ParserDeploymentWrapperException(const ParserDeploymentException&);
    void throwParserDeploymentException() const;

private:

    ParserDeploymentException _exception;
};

//
// SAX error handler for compoonent descriptors.
//
class ComponentErrorHandler : public ErrorHandler
{
public:

    ComponentErrorHandler(ComponentBuilder&);

    void warning(const SAXParseException& exception);
    void error(const SAXParseException& exception);
    void fatalError(const SAXParseException& exception);
    void resetErrors();

private:

    ComponentBuilder& _builder;
};

//
// SAX parser handler for component descriptors.
//
class ComponentHandler : public DocumentHandler
{
public:

    ComponentHandler(ComponentBuilder&);

    virtual void characters(const XMLCh*const, const unsigned int);
    virtual void startElement(const XMLCh*const, AttributeList&); 
    virtual void endElement(const XMLCh*const);
    virtual void setDocumentLocator(const Locator *const);

    virtual void ignorableWhitespace(const XMLCh*const, const unsigned int);
    virtual void processingInstruction(const XMLCh*const, const XMLCh*const);
    virtual void resetDocument();
    virtual void startDocument();
    virtual void endDocument();

protected:

    std::string getAttributeValue(const AttributeList&, const std::string&) const;
    std::string getAttributeValueWithDefault(const AttributeList&, const std::string&, const std::string&) const;

    std::string toString(const XMLCh*const) const;
    std::string elementValue() const;
    bool isCurrentTargetDeployable() const;

    ComponentBuilder& _builder;
    std::stack<std::string> _elements;
    std::string _currentAdapter;
    std::string _currentTarget;
    bool _isCurrentTargetDeployable;

    const Locator* _locator;
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

    ComponentBuilder(const Ice::CommunicatorPtr&, const std::string&, const std::vector<std::string>&);

    virtual void execute();
    virtual void undo();

    void parse(const std::string&, ComponentHandler&);
    void setDocumentLocator(const Locator*const locator);

    void setYellowAdmin(const Yellow::AdminPrx&);

    bool isTargetDeployable(const std::string&) const;

    void createDirectory(const std::string&, bool = false);
    void createConfigFile(const std::string&);
    void addProperty(const std::string&, const std::string&);
    void addOffer(const std::string&, const std::string&, const std::string&);
    void overrideBaseDir(const std::string&);

    std::string toLocation(const std::string&) const;
    std::string substitute(const std::string&) const;
    void undoFrom(std::vector<TaskPtr>::iterator);

protected:
    
    Ice::CommunicatorPtr _communicator;
    Yellow::AdminPrx _yellowAdmin;

    Ice::PropertiesPtr _properties;
    std::map<std::string, std::string> _variables;
    std::vector<TaskPtr> _tasks;
    std::string _configFile;
    std::string _componentPath;
    std::vector<std::string> _targets;

    const Locator* _locator;
};

}

#endif
