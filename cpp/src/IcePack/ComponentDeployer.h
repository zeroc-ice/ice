// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_COMPONENT_DEPLOYER_H
#define ICE_PACK_COMPONENT_DEPLOYER_H

#include <IceUtil/Shared.h>
#include <IcePack/Admin.h>
#include <Yellow/Yellow.h>
#include <sax/HandlerBase.hpp>

#include <map>
#include <vector>
#include <stack>

namespace IcePack
{

class Task : public ::IceUtil::SimpleShared
{
public:

    virtual void deploy() = 0;
    virtual void undeploy() = 0;
};

typedef ::IceUtil::Handle< ::IcePack::Task> TaskPtr;

class ComponentDeployer;

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

class ComponentErrorHandler : public ErrorHandler
{
public:

    ComponentErrorHandler(ComponentDeployer&);

    void warning(const SAXParseException& exception);
    void error(const SAXParseException& exception);
    void fatalError(const SAXParseException& exception);
    void resetErrors();

private:

    ComponentDeployer& _deployer;
};

class ComponentDeployHandler : public DocumentHandler
{
public:

    ComponentDeployHandler(ComponentDeployer&);

    virtual void characters(const XMLCh*const, const unsigned int);
    virtual void startElement(const XMLCh*const, AttributeList&); 
    virtual void endElement(const XMLCh*const);

    // TODO: ML: No reason to make inline, see style guide.
    virtual void ignorableWhitespace(const XMLCh*const, const unsigned int) { }
    virtual void processingInstruction(const XMLCh*const, const XMLCh*const) { }
    virtual void resetDocument() { }
    virtual void setDocumentLocator(const Locator *const);
    virtual void startDocument() { }
    virtual void endDocument() { }

protected:

    std::string getAttributeValue(const AttributeList&, const std::string&) const;
    std::string getAttributeValueWithDefault(const AttributeList&, const std::string&, const std::string&) const;

    std::string toString(const XMLCh*const) const;
    std::string elementValue() const;
    bool isCurrentTargetDeployable() const;

private:

    ComponentDeployer& _deployer;
    std::stack<std::string> _elements;
    std::string _currentAdapter;
    std::string _currentTarget;
    bool _isCurrentTargetDeployable;

    const Locator* _locator;
};

class ComponentDeployer : public Task
{
public:

    ComponentDeployer(const Ice::CommunicatorPtr&, const std::string&, const std::vector<std::string>&);

    virtual void deploy();
    virtual void undeploy();

    void parse(const std::string&, ComponentDeployHandler&);
    void setDocumentLocator(const Locator*const locator);
    std::string substitute(const std::string&) const;

    bool isTargetDeployable(const std::string&) const;

    void createDirectory(const std::string&, bool = false);
    void createConfigFile(const std::string&);
    void addProperty(const std::string&, const std::string&);
    void addOffer(const std::string&, const std::string&, const std::string&);
    void overrideBaseDir(const std::string&);

protected:

    void undeployFrom(std::vector<TaskPtr>::iterator);

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
