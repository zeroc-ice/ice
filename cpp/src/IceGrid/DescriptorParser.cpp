// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceXML/Parser.h>
#include <IcePatch2Lib/Util.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorBuilder.h>
#include <IceGrid/Util.h>

#include <stack>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class DescriptorHandler : public IceXML::Handler
{
public:

    DescriptorHandler(const string&, const Ice::CommunicatorPtr&);

    void setAdmin(const IceGrid::AdminPrx&);
    void setVariables(const map<string, string>&, const vector<string>&);

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    virtual void characters(const string&, int, int);
    virtual void error(const string&, int, int);

    const ApplicationDescriptor& getApplicationDescriptor() const;

private:

    bool isCurrentTargetDeployable() const;
    string elementValue();
    vector<string> getTargets(const string&) const;
    void error(const string&) const;
    bool isTargetDeployable(const string&) const;

    const Ice::CommunicatorPtr _communicator;
    IceGrid::AdminPrx _admin;
    string _filename;
    map<string, string> _overrides;
    vector<string> _targets;
    string _data;
    string _previousElementName;
    int _targetCounter;
    bool _isCurrentTargetDeployable;
    int _line;
    int _column;

    IceInternal::UniquePtr<ApplicationDescriptorBuilder> _currentApplication;
    IceInternal::UniquePtr<NodeDescriptorBuilder> _currentNode;
    IceInternal::UniquePtr<TemplateDescriptorBuilder> _currentTemplate;
    IceInternal::UniquePtr<ServerInstanceDescriptorBuilder> _currentServerInstance;
    IceInternal::UniquePtr<ServiceInstanceDescriptorBuilder> _currentServiceInstance;
    IceInternal::UniquePtr<ServerDescriptorBuilder> _currentServer;
    IceInternal::UniquePtr<ServiceDescriptorBuilder> _currentService;
    CommunicatorDescriptorBuilder* _currentCommunicator;
    IceInternal::UniquePtr<PropertySetDescriptorBuilder> _currentPropertySet;

    bool _isTopLevel;
    bool _inAdapter;
    bool _inReplicaGroup;
    bool _inDbEnv;
    bool _inDistrib;
};

}

DescriptorHandler::DescriptorHandler(const string& filename, const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _filename(filename),
    _isCurrentTargetDeployable(true),
    _currentCommunicator(0),
    _isTopLevel(true),
    _inAdapter(false),
    _inReplicaGroup(false),
    _inDbEnv(false)
{
}

void
DescriptorHandler::setAdmin(const AdminPrx& admin)
{
    _admin = admin;
}

void
DescriptorHandler::setVariables(const map<string, string>& variables, const vector<string>& targets)
{
    _overrides = variables;
    _targets = targets;
}

void
DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    _line = line;
    _column = column;
    XmlAttributesHelper attributes(attrs, _communicator->getLogger(), _filename, line);

    try
    {
        if(name == "icegrid")
        {
            if(!_isTopLevel)
            {
                error("element <icegrid> is a top level element");
            }
            _isTopLevel = false;
        }
        else if(_isTopLevel)
        {
            error("only the <icegrid> element is allowed at the top-level");
        }
        else if(name == "target")
        {
            if(!_isCurrentTargetDeployable)
            {
                ++_targetCounter;
            }
            else
            {
                _isCurrentTargetDeployable = isTargetDeployable(attributes("name"));
                _targetCounter = 1;
                return;
            }
        }
        else if(!isCurrentTargetDeployable())
        {
            //
            // We don't bother to parse the elements if the elements are enclosed in a target element
            // which won't be deployed.
            //
            attributes.asMap();
            return;
        }
        else if(name == "include")
        {
            string targets = attributes("targets", "");
            string file = attributes("file");
            if(file[0] != '/')
            {
                string::size_type end = _filename.find_last_of('/');
                if(end != string::npos)
                {
                    file = _filename.substr(0, end) + "/" + file;
                }
            }

            string oldFileName = _filename;
            vector<string> oldTargets = _targets;
            _isTopLevel = true;
            _filename = file;
            _targets = getTargets(targets);

            IceXML::Parser::parse(file, *this);

            _filename = oldFileName;
            _targets = oldTargets;
        }
        else if(name == "application")
        {
            if(_currentApplication.get())
            {
                error("only one <application> element is allowed");
            }

            bool importTemplates = attributes.asBool("import-default-templates", false);

            //
            // TODO: is ignoring importTemplates the desired behavior when _admin == 0?
            //
            if(importTemplates && _admin != 0)
            {
                try
                {
                    ApplicationDescriptor application = _admin->getDefaultApplicationDescriptor();
                    _currentApplication.reset(new ApplicationDescriptorBuilder(_communicator, application,
                                                                               attributes, _overrides));
                }
                catch(const DeploymentException& ex)
                {
                    throw ex.reason;
                }
            }
            else
            {
                _currentApplication.reset(new ApplicationDescriptorBuilder(_communicator, attributes, _overrides));
            }
        }
        else if(name == "node")
        {
            if(!_currentApplication.get() || _currentNode.get())
            {
                error("the <node> element can only be a child of an <application> element");
            }
            _currentNode.reset(_currentApplication->createNode(attributes));
        }
        else if(name == "server-instance")
        {
            if(!_currentNode.get() || _currentServer.get() || _currentServerInstance.get())
            {
                error("the <server-instance> element can only be a child of a <node> element");
            }
            _currentServerInstance.reset(_currentNode->createServerInstance(attributes));
        }
        else if(name == "server")
        {
            if((!_currentNode.get() && !_currentTemplate.get()) || _currentServer.get() || _currentServerInstance.get())
            {
                error("the <server> element can only be a child of a <node> or <server-template> element");
            }
            if(_currentNode.get())
            {
                _currentServer.reset(_currentNode->createServer(attributes));
            }
            else
            {
                _currentServer.reset(_currentTemplate->createServer(attributes));
            }
            _currentCommunicator = _currentServer.get();
        }
        else if(name == "icebox")
        {
            if((!_currentNode.get() && !_currentTemplate.get()) || _currentServer.get() || _currentServerInstance.get())
            {
                error("the <icebox> element can only be a child of a <node> or <server-template> element");
            }
            if(_currentNode.get())
            {
                _currentServer.reset(_currentNode->createIceBox(attributes));
            }
            else
            {
                _currentServer.reset(_currentTemplate->createIceBox(attributes));
            }
            _currentCommunicator = _currentServer.get();
        }
        else if(name == "server-template")
        {
            if(!_currentApplication.get() || _currentTemplate.get() || _currentNode.get())
            {
                error("the <server-template> element can only be a child of an <application> element");
            }
            _currentTemplate.reset(_currentApplication->createServerTemplate(attributes));
        }
        else if(name == "service-instance")
        {
            if(!_currentServer.get() || _currentServiceInstance.get())
            {
                error("the <service-instance> element can only be a child of an <icebox> element");
            }
            _currentServiceInstance.reset(_currentServer->createServiceInstance(attributes));
        }
        else if(name == "service")
        {
            if((!_currentServer.get() && !_currentTemplate.get()) || _currentService.get() ||
               _currentServiceInstance.get())
            {
                error("the <service> element can only be a child of an <icebox> or <service-template> element");
            }

            if(_currentServer.get())
            {
                _currentService.reset(_currentServer->createService(attributes));
            }
            else
            {
                _currentService.reset(_currentTemplate->createService(attributes));
            }
            _currentCommunicator = _currentService.get();
        }
        else if(name == "service-template")
        {
            if(!_currentApplication.get() || _currentNode.get() || _currentTemplate.get())
            {
                error("the <service-template> element can only be a child of an <application> element");
            }

            _currentTemplate.reset(_currentApplication->createServiceTemplate(attributes));
        }
        else if(name == "replica-group")
        {
            if(!_currentApplication.get())
            {
                error("the <replica-group> element can only be a child of an <application> element");
            }
            _currentApplication->addReplicaGroup(attributes);
            _inReplicaGroup = true;
        }
        else if(name == "load-balancing")
        {
            if(!_inReplicaGroup)
            {
                error("the <load-balancing> element can only be a child of a <replica-group> element");
            }
            _currentApplication->setLoadBalancing(attributes);
        }
        else if(name == "variable")
        {
            if(_currentNode.get())
            {
                _currentNode->addVariable(attributes);
            }
            else if(_currentApplication.get())
            {
                _currentApplication->addVariable(attributes);
            }
            else
            {
                error("the <variable> element can only be a child of an <application> or <node> element");
            }
        }
        else if(name == "parameter")
        {
            if(!_currentTemplate.get())
            {
                error("the <parameter> element can only be a child of a <template> element");
            }
            _currentTemplate->addParameter(attributes);
        }
        else if(name == "properties")
        {
            if(_currentPropertySet.get())
            {
                _currentPropertySet->addPropertySet(attributes);
            }
            else if(_currentServiceInstance.get())
            {
                _currentPropertySet.reset(_currentServiceInstance->createPropertySet());
            }
            else if(_currentServerInstance.get())
            {
                _currentPropertySet.reset(_currentServerInstance->createPropertySet(attributes));
            }
            else if(_currentCommunicator)
            {
                _currentPropertySet.reset(_currentCommunicator->createPropertySet());
            }
            else if(_currentNode.get())
            {
                _currentPropertySet.reset(_currentNode->createPropertySet(attributes));
            }
            else if(_currentApplication.get() && !_currentTemplate.get())
            {
                _currentPropertySet.reset(_currentApplication->createPropertySet(attributes));
            }
            else
            {
                error("the <properties> element is not allowed here");
            }
        }
        else if(name == "property")
        {
            if(_currentPropertySet.get())
            {
                _currentPropertySet->addProperty(attributes);
            }
            else if(_currentCommunicator)
            {
                _currentCommunicator->addProperty(attributes);
            }
            else
            {
                error("the <property> element can only be a child of a <properties>, <icebox>, <server> or <service> "
                      "element");
            }
        }
        else if(name == "adapter")
        {
            if(!_currentCommunicator)
            {
                error("the <adapter> element can only be a child of a <server> or <service> element");
            }
            _currentCommunicator->addAdapter(attributes);
            _inAdapter = true;
        }
        else if(name == "object")
        {
            if(!_inAdapter && !_inReplicaGroup)
            {
                error("the <object> element can only be a child of an <adapter> or <replica-group> element");
            }
            if(_inReplicaGroup)
            {
                _currentApplication->addObject(attributes);
            }
            else
            {
                _currentCommunicator->addObject(attributes);
            }
        }
        else if(name == "allocatable")
        {
            if(!_inAdapter)
            {
                error("the <allocatable> element can only be a child of an <adapter> element");
            }
            _currentCommunicator->addAllocatable(attributes);
        }
        else if(name == "distrib")
        {
            if(!_currentApplication.get() ||
               ((_currentNode.get() || _currentTemplate.get()) && !_currentServer.get()) ||
               _currentServer.get() != _currentCommunicator)
            {
                error("the <distrib> element can only be a child of an <application>, <server> or <icebox> element");
            }
            if(!_currentServer.get())
            {
                _currentApplication->addDistribution(attributes);
            }
            else
            {
                _currentServer->addDistribution(attributes);
            }
            _inDistrib = true;
        }
        else if(name == "dbenv")
        {
            if(!_currentCommunicator)
            {
                error("the <dbenv> element can only be a child of a <server> or <service> element");
            }
            _currentCommunicator->addDbEnv(attributes);
            _inDbEnv = true;
        }
        else if(name == "log")
        {
            if(!_currentCommunicator)
            {
                error("the <log> element can only be a child of a <server> or <service> element");
            }
            _currentCommunicator->addLog(attributes);
        }
        else if(name == "dbproperty")
        {
            if(!_inDbEnv)
            {
                error("the <dbproperty> element can only be a child of a <dbenv> element");
            }
            _currentCommunicator->addDbEnvProperty(attributes);
        }
        else if(name == "description" || name == "option" || name == "env" || name == "directory")
        {
            //
            // Nothing to do.
            //
        }
        else
        {
            error("unknown element `" + name + "'");
        }

        attributes.checkUnknownAttributes();
    }
    catch(const string& reason)
    {
        error(reason);
    }
    catch(const char* reason)
    {
        error(reason);
    }

    //
    // Check if the previous element value has been consumed and if not make
    // sure it's "empty".
    //
    string value = elementValue();
    if(!value.empty() && value.find_first_not_of(" \t\r\n") != string::npos)
    {
        error("invalid element value for element `" + _previousElementName + "'");
    }
    _previousElementName = name;
}

void
DescriptorHandler::endElement(const string& name, int line, int column)
{
    _line = line;
    _column = column;

    try
    {
        if(name == "target")
        {
            if(!_isCurrentTargetDeployable && --_targetCounter == 0)
            {
                _isCurrentTargetDeployable = true;
                _targetCounter = 0;
            }
            return;
        }
        else if(!isCurrentTargetDeployable())
        {
            //
            // We don't bother to parse the elements if the elements are enclosed in a target element
            // which won't be deployed.
            //
            return;
        }
        else if(name == "node")
        {
            _currentApplication->addNode(_currentNode->getName(), _currentNode->getDescriptor());
            _currentNode.reset(0);
        }
        else if(name == "server" || name == "icebox")
        {
            assert(_currentServer.get());
            if(_currentTemplate.get())
            {
                _currentTemplate->setDescriptor(_currentServer->getDescriptor());
            }
            else
            {
                assert(_currentNode.get());
                _currentNode->addServer(_currentServer->getDescriptor());
            }
            _currentServer->finish();
            _currentServer.reset(0);
            _currentCommunicator = 0;
        }
        else if(name == "server-template")
        {
            assert(_currentApplication.get());
            _currentApplication->addServerTemplate(_currentTemplate->getId(), _currentTemplate->getDescriptor());
            _currentTemplate.reset(0);
        }
        else if(name == "service")
        {
            assert(_currentService.get());
            if(_currentServer.get())
            {
                _currentServer->addService(_currentService->getDescriptor());
            }
            else
            {
                _currentTemplate->setDescriptor(_currentService->getDescriptor());
            }
            _currentService->finish();
            _currentService.reset(0);
            _currentCommunicator = _currentServer.get();
        }
        else if(name == "service-template")
        {
            assert(_currentTemplate.get());
            _currentApplication->addServiceTemplate(_currentTemplate->getId(), _currentTemplate->getDescriptor());
            _currentTemplate.reset(0);
        }
        else if(name == "server-instance")
        {
            assert(_currentNode.get() && _currentServerInstance.get());
            _currentNode->addServerInstance(_currentServerInstance->getDescriptor());
            _currentServerInstance.reset(0);
        }
        else if(name == "service-instance")
        {
            assert(_currentServer.get() && _currentServiceInstance.get());
            _currentServer->addServiceInstance(_currentServiceInstance->getDescriptor());
            _currentServiceInstance.reset(0);
        }
        else if(name == "properties")
        {
            assert(_currentPropertySet.get());
            if(_currentPropertySet->finish())
            {
                if(_currentServiceInstance.get())
                {
                    _currentServiceInstance->addPropertySet(_currentPropertySet->getDescriptor());
                }
                else if(_currentServerInstance.get())
                {
                    _currentServerInstance->addPropertySet(_currentPropertySet->getService(),
                                                           _currentPropertySet->getDescriptor());
                }
                else if(_currentCommunicator)
                {
                    _currentCommunicator->addPropertySet(_currentPropertySet->getDescriptor());
                }
                else if(_currentNode.get())
                {
                    _currentNode->addPropertySet(_currentPropertySet->getId(),
                                                 _currentPropertySet->getDescriptor());
                }
                else if(_currentApplication.get())
                {
                    _currentApplication->addPropertySet(_currentPropertySet->getId(),
                                                        _currentPropertySet->getDescriptor());
                }
                else
                {
                    assert(false);
                }
                _currentPropertySet.reset(0);
            }
        }
        else if(name == "description")
        {
            if(_inAdapter)
            {
                _currentCommunicator->setAdapterDescription(elementValue());
            }
            else if(_inReplicaGroup)
            {
                _currentApplication->setReplicaGroupDescription(elementValue());
            }
            else if(_inDbEnv)
            {
                assert(_currentCommunicator);
                _currentCommunicator->setDbEnvDescription(elementValue());
            }
            else if(_currentCommunicator)
            {
                _currentCommunicator->setDescription(elementValue());
            }
            else if(_currentNode.get())
            {
                _currentNode->setDescription(elementValue());
            }
            else if(_currentApplication.get())
            {
                _currentApplication->setDescription(elementValue());
            }
            else
            {
                error("element <description> is not allowed here");
            }
        }
        else if(name == "option")
        {
            if(!_currentServer.get())
            {
                error("element <option> can only be the child of a <server> element");
            }
            _currentServer->addOption(elementValue());
        }
        else if(name == "env")
        {
            if(!_currentServer.get())
            {
                error("element <env> can only be the child of a <server> element");
            }
            _currentServer->addEnv(elementValue());
        }
        else if(name == "directory")
        {
            if(!_inDistrib)
            {
                error("the <directory> element can only be a child of a <distrib> element");
            }
            if(!_currentServer.get())
            {
                _currentApplication->addDistributionDirectory(elementValue());
            }
            else
            {
                _currentServer->addDistributionDirectory(elementValue());
            }
        }
        else if(name == "adapter")
        {
            _inAdapter = false;
        }
        else if(name == "replica-group")
        {
            _currentApplication->finishReplicaGroup();
            _inReplicaGroup = false;
        }
        else if(name == "dbenv")
        {
            _inDbEnv = false;
        }
        else if(name == "distrib")
        {
            _inDistrib = false;
        }
    }
    catch(const string& reason)
    {
        error(reason);
    }
    catch(const char* reason)
    {
        error(reason);
    }

    //
    // Check if the element value has been consumed and if not make
    // sure it's "empty".
    //
    string value = elementValue();
    if(!value.empty() && value.find_first_not_of(" \t\r\n") != string::npos)
    {
        error("invalid element value for element `" + name + "'");
    }
}

void
DescriptorHandler::characters(const string& chars, int, int)
{
    if(isCurrentTargetDeployable())
    {
        _data += chars;
    }
}

void
DescriptorHandler::error(const string& msg, int line, int column)
{
    ostringstream os;
    os << "error in <" << _filename << "> descriptor, line " << line << ", column " << column << ":\n" << msg;
    throw IceXML::ParserException(__FILE__, __LINE__, os.str());
}

const ApplicationDescriptor&
DescriptorHandler::getApplicationDescriptor() const
{
    if(!_currentApplication.get())
    {
        error("no application descriptor defined in this file");
    }
    return _currentApplication->getDescriptor();
}

bool
DescriptorHandler::isCurrentTargetDeployable() const
{
    return _isCurrentTargetDeployable;
}

vector<string>
DescriptorHandler::getTargets(const string& targets) const
{
    vector<string> result;

    if(!targets.empty())
    {
        const string delim = " \t\n\r";

        string::size_type beg = 0;
        string::size_type end = 0;
        do
        {
            end = targets.find_first_of(delim, end);
            if(end == string::npos)
            {
                end = targets.size();
            }

            result.push_back(targets.substr(beg, end - beg));
            beg = ++end;
        }
        while(end < targets.size());
    }

    copy(_targets.begin(), _targets.end(), back_inserter(result));

    return result;
}

void
DescriptorHandler::error(const string& msg) const
{
    ostringstream os;
    os << "error in <" << _filename << "> descriptor, line " << _line << ", column " << _column << ":\n" << msg;
    throw IceXML::ParserException(__FILE__, __LINE__, os.str());
}

string
DescriptorHandler::elementValue()
{
    string tmp;
    tmp = _data;
    _data = "";
    return tmp;
}

bool
DescriptorHandler::isTargetDeployable(const string& target) const
{
    string application = _currentApplication.get() ? _currentApplication->getDescriptor().name : string("");
    string node = _currentNode.get() ? _currentNode->getName() : string("");
    string server = _currentServer.get() ? _currentServer->getDescriptor()->id : string("");
    string service = _currentService.get() ? _currentService->getDescriptor()->name : string("");

    //
    // Compute the current fully qualified name of the communicator.
    //
    string fqn;
    if(!application.empty())
    {
        fqn += (fqn.empty() ? "" : ".") + application;
    }
    if(!node.empty())
    {
        fqn += (fqn.empty() ? "" : ".") + node;
    }
    if(!server.empty())
    {
        fqn += (fqn.empty() ? "" : ".") + server;
    }
    if(!service.empty())
    {
        fqn += (fqn.empty() ? "" : ".") + service;
    }

    //
    // Go through the list of supplied targets and see if we can match one with the current communicator + target.
    //
    for(vector<string>::const_iterator p = _targets.begin(); p != _targets.end(); ++p)
    {
        if((*p) == target)
        {
            //
            // A supplied target without any communicator prefix is matching the target.
            //
            return true;
        }
        else
        {
            string communicatorTarget;
            string::size_type end = 0;
            while(end != string::npos)
            {
                //
                // Add the first communicator name from the communicator fully qualified name to the
                // target and see if matches.
                //
                end = fqn.find('.', end);
                if(end == string::npos)
                {
                    communicatorTarget = fqn + "." + target;
                }
                else
                {
                    communicatorTarget = fqn.substr(0, end) + "." + target;
                    ++end;
                }

                if((*p) == communicatorTarget)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

ApplicationDescriptor
DescriptorParser::parseDescriptor(const string& descriptor,
                                  const Ice::StringSeq& targets,
                                  const map<string, string>& variables,
                                  const Ice::CommunicatorPtr& communicator,
                                  const IceGrid::AdminPrx& admin)
{
    string filename = IcePatch2Internal::simplify(descriptor);
    DescriptorHandler handler(filename, communicator);
    handler.setAdmin(admin);
    handler.setVariables(variables, targets);
    IceXML::Parser::parse(filename, handler);
    return handler.getApplicationDescriptor();
}

ApplicationDescriptor
DescriptorParser::parseDescriptor(const string& descriptor, const Ice::CommunicatorPtr& communicator)
{
    string filename = IcePatch2Internal::simplify(descriptor);
    DescriptorHandler handler(filename, communicator);
    IceXML::Parser::parse(filename, handler);
    return handler.getApplicationDescriptor();
}
