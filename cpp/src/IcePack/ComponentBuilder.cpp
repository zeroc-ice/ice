// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ComponentBuilder.h>
#include <IcePack/Internal.h>
#include <Yellow/Yellow.h>

#include <xercesc/parsers/SAXParser.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include <iterator>
#include <fstream>

using namespace std;
using namespace IcePack;

namespace IcePack
{

static string
toString(const XMLCh* ch)
{
    char* t = XMLString::transcode(ch);
    string s(t);
    delete[] t;
    return s;
}

//
// Create a directory. 
//
// TODO: IcePatch implements portable version of filesystem primitives
// in the Util.cpp file. We should move these primitives in IceUtil
// and use them here.
//
class CreateDirectory : public Task
{
public:

    CreateDirectory(const string& name, bool force) :
	_name(name), 
	_force(force)
    {
    }

    virtual void
    execute()
    {
	if(mkdir(_name.c_str(), 0755) != 0)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't create directory " + _name + ": " + strerror(getSystemErrno());
	    throw ex;
	}
    }

    virtual void
    undo()
    {
	if(_force)
	{
	    //
	    // Only remove files inside the directory, don't remove
	    // directories (deployed directory should only be created
	    // through this task so other directories should be
	    // removed by another task).
	    //
	    struct dirent **namelist;
	    int n = ::scandir(_name.c_str(), &namelist, 0, alphasort);
	    if(n > 0)
	    {
		Ice::StringSeq entries;
		entries.reserve(n);
		for(int i = 0; i < n; ++i)
		{
		    string name = namelist[i]->d_name;
		    free(namelist[i]);
		    entries.push_back(_name + "/" + name);
		}
		free(namelist);
		
		for(Ice::StringSeq::iterator p = entries.begin(); p != entries.end(); ++p)
		{
		    struct stat buf;
		    
		    if(::stat(p->c_str(), &buf) != 0)
		    {
			if(errno != ENOENT)
			{
			    //
			    // TODO: log error
			    //
			}
		    }
		    else if(S_ISREG(buf.st_mode))
		    {
			if(unlink(p->c_str()) != 0)
			{
			    //
			    // TODO: log error
			    //
			}
		    }
		}
	    }
	    else if(n < 0)
	    {
		//
		// TODO: something seems to be wrong if we can't scan
		// the directory. Print a warning.
		//
	    }
	}

	if(rmdir(_name.c_str()) != 0)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't remove directory " + _name + ": " + strerror(getSystemErrno());
	    throw ex;
	}
    }
    
private:

    string _name;
    bool _force;
};

//
// Generate a configuration file from a property set.
//
class GenerateConfiguration : public Task
{
    class WriteConfigProperty : public unary_function<Ice::PropertyDict::value_type, string>
    {
    public:

	string 
	operator()(const Ice::PropertyDict::value_type& p) const
	{
	    return p.first + "=" + p.second;
	}
    };

public:

    GenerateConfiguration(const string& file, const Ice::PropertiesPtr& properties) :
	_file(file),
	_properties(properties)
    {
    }

    virtual void
    execute()  
    {
	ofstream configfile;
	configfile.open(_file.c_str(), ios::out);
	if(!configfile)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't create configuration file: " + _file;
	    throw ex;
	}
	
	Ice::PropertyDict props = _properties->getPropertiesForPrefix("");
	transform(props.begin(), props.end(), ostream_iterator<string>(configfile,"\n"), WriteConfigProperty());
	configfile.close();
    }

    virtual void
    undo()
    {
	if(unlink(_file.c_str()) != 0)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't remove configuration file: " + _file;
	    throw ex;
	}
    }

private:

    string _file;
    Ice::PropertiesPtr _properties;
};

//
// Register an offer with the yellow page service.
//
class RegisterOffer : public Task
{
public:

    RegisterOffer(const Yellow::AdminPrx& admin, const string& offer, const Ice::ObjectPrx& proxy) :
	_admin(admin),
	_offer(offer),
	_proxy(proxy)
    {
    }

    virtual void
    execute()
    {
	try
	{
	    _admin->add(_offer, _proxy);
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact Yellow to add offer:\n" << lex << ends;

	    OfferDeploymentException ex;
	    ex.reason = os.str();
	    ex.intf = _offer;
	    ex.proxy = _proxy;
	    throw ex;
	}
    }

    virtual void
    undo()
    {
	assert(_admin);
	try
	{
	    _admin->remove(_offer, _proxy);
	}
	catch(const Yellow::NoSuchOfferException& lex)
	{
	    ostringstream os;
	    os << "couldn't remove offer:\n" << lex << ends;

	    OfferDeploymentException ex;
	    ex.reason = os.str();
	    ex.intf = _offer;
	    ex.proxy = _proxy;
	    throw ex;
	}	
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact Yellow to remove offer:\n" << lex << ends;

	    OfferDeploymentException ex;
	    ex.reason = os.str();
	    ex.intf = _offer;
	    ex.proxy = _proxy;
	    throw ex;
	}
    }

private:

    Yellow::AdminPrx _admin;
    string _offer;
    Ice::ObjectPrx _proxy;
};

//
// Register an identity.
//
class RegisterObject : public Task
{
public:

    RegisterObject(const ObjectRegistryPrx& registry, const ObjectDescription& desc) :
	_registry(registry),
	_desc(desc)
    {
    }

    virtual void
    execute()
    {
	try
	{
	    _registry->add(_desc);
	}
	catch(const ObjectExistsException& lex)
	{
	    ostringstream os;
	    os << "couldn't add the object:\n" << lex << ends;

	    ObjectDeploymentException ex;
	    ex.reason = os.str();
	    ex.proxy = _desc.proxy;
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the object registry:\n" << lex << ends;

	    ObjectDeploymentException ex;
	    ex.reason = os.str();
	    ex.proxy = _desc.proxy;
	    throw ex;
	}
    }

    virtual void
    undo()
    {
	try
	{
	    _registry->remove(_desc.proxy);
	}
	catch(const ObjectNotExistException& ex)
	{
	    ostringstream os;
	    os << "couldn't remove the object:\n" << ex << ends;

	    ObjectDeploymentException ex;
	    ex.reason = os.str();
	    ex.proxy = _desc.proxy;
	    throw ex;
	}	
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the object registry:\n" << lex << ends;
	    
	    ObjectDeploymentException ex;
	    ex.reason = os.str();
	    ex.proxy = _desc.proxy;
	    throw ex;
	}
    }

private:

    ObjectRegistryPrx _registry;
    ObjectDescription _desc;
};

}

IcePack::DeploySAXParseException::DeploySAXParseException(const string& msg, const Locator*const locator)
    : SAXParseException(XMLString::transcode(msg.c_str()), *locator)
{
}

IcePack::ParserDeploymentWrapperException::ParserDeploymentWrapperException(const ParserDeploymentException& ex)
    : _exception(ex)
{
}

void
IcePack::ParserDeploymentWrapperException::throwParserDeploymentException() const
{
    throw _exception;
}

IcePack::ComponentErrorHandler::ComponentErrorHandler(ComponentBuilder& builder) :
    _builder(builder)
{
}

void
IcePack::ComponentErrorHandler::warning(const SAXParseException& exception)
{
    string s = toString(exception.getMessage());
    cerr << "warning: " << s << endl;
}

void
IcePack::ComponentErrorHandler::error(const SAXParseException& exception)
{
    throw exception;
//    string s = toString(exception.getMessage());
//    cerr << "error: " << s << endl;
}

void
IcePack::ComponentErrorHandler::fatalError(const SAXParseException& exception)
{
    throw exception;
//    string s = toString(exception.getMessage());
//    cerr << "fatal:" << s << endl;
}

void
IcePack::ComponentErrorHandler::resetErrors()
{
}

IcePack::ComponentHandler::ComponentHandler(ComponentBuilder& builder) :
    _builder(builder),
    _isCurrentTargetDeployable(true)
{
}

void
IcePack::ComponentHandler::characters(const XMLCh *const chars, const unsigned int length)
{
    _elements.top().assign(toString(chars));
}

void
IcePack::ComponentHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    _elements.push("");

    if(!isCurrentTargetDeployable())
    {
	return;
    }

    string str = toString(name);

    if(str == "property")
    {
	string value = getAttributeValueWithDefault(attrs, "value", "");
	if(value.empty())
	{
	    value = _builder.toLocation(getAttributeValueWithDefault(attrs, "location", ""));
	}
	_builder.addProperty(getAttributeValue(attrs, "name"), value);
    }
    else if(str == "adapter")
    {
	if(!_currentAdapterId.empty())
	{
	    throw DeploySAXParseException("Adapter element enclosed in an adapter element is not allowed", _locator);
	}

	//
	// If the id is not specified, we ask the builder to generate
	// an id for us based on the adapter name.
	//
	string name = getAttributeValue(attrs, "name");
	if(name.empty())
	{
	    throw DeploySAXParseException("empty adapter name", _locator);
	}
	_currentAdapterId = getAttributeValueWithDefault(attrs, "id", _builder.getDefaultAdapterId(name));
    }
    else if(str == "offer")
    {
	_builder.addOffer(getAttributeValue(attrs, "interface"),
			  _currentAdapterId,
			  getAttributeValue(attrs, "identity"));
    }
    else if(str == "object")
    {
	_builder.addObject(getAttributeValue(attrs, "identity"), 
			   _currentAdapterId,
			   getAttributeValue(attrs, "type"));
    }
    else if(str == "target")
    {
	if(!_currentTarget.empty())
	{
	    throw DeploySAXParseException("Target element enclosed in a target element is not allowed", _locator);
	}
	_isCurrentTargetDeployable = _builder.isTargetDeployable(getAttributeValue(attrs, "name"));
    }
}

void
IcePack::ComponentHandler::endElement(const XMLCh *const name)
{
    _elements.pop();

    string str = toString(name);

    if(str == "target")
    {
	_isCurrentTargetDeployable = true;
    }

    if(isCurrentTargetDeployable())
    {
	if(str == "adapter")
	{
	    _currentAdapterId = "";
	}
    }
}

void 
IcePack::ComponentHandler::ignorableWhitespace(const XMLCh*const, const unsigned int)
{
}

void 
IcePack::ComponentHandler::processingInstruction(const XMLCh*const, const XMLCh*const)
{
}

void 
IcePack::ComponentHandler::resetDocument()
{
}

void 
IcePack::ComponentHandler::startDocument()
{
}

void 
IcePack::ComponentHandler::endDocument()
{
}

void 
IcePack::ComponentHandler::setDocumentLocator(const Locator *const locator)
{
    _builder.setDocumentLocator(locator);

    _locator = locator;
}

string
IcePack::ComponentHandler::getAttributeValue(const AttributeList& attrs, const string& name) const
{
    XMLCh* n = XMLString::transcode(name.c_str());
    const XMLCh* value = attrs.getValue(n);
    delete[] n;
    
    if(value == 0)
    {
	throw DeploySAXParseException("missing attribute '" + name + "'", _locator);
    }

    return _builder.substitute(toString(value));
}

string
IcePack::ComponentHandler::getAttributeValueWithDefault(const AttributeList& attrs, const string& name, 
							const string& def) const
{
    XMLCh* n = XMLString::transcode(name.c_str());
    const XMLCh* value = attrs.getValue(n);
    delete[] n;
    
    if(value == 0)
    {
	return _builder.substitute(def);
    }
    else
    {
	return _builder.substitute(toString(value));
    }
}

string
IcePack::ComponentHandler::toString(const XMLCh* ch) const
{
    return IcePack::toString(ch);
}

string
IcePack::ComponentHandler::elementValue() const
{
    return _elements.top();
}

bool
IcePack::ComponentHandler::isCurrentTargetDeployable() const
{
    return _isCurrentTargetDeployable;
}

IcePack::ComponentBuilder::ComponentBuilder(const Ice::CommunicatorPtr& communicator,
					    const map<string, string>& variables,
					    const vector<string>& targets) :
    _communicator(communicator),
    _properties(Ice::createProperties()),
    _variables(variables),
    _targets(targets)
{
}

void 
IcePack::ComponentBuilder::parse(const string& xmlFile, ComponentHandler& handler)
{
    //
    // Setup the base directory for this deploment descriptor to the
    // location of the desciptor file.
    //
    string::size_type end = xmlFile.find_last_of('/');
    if(end != string::npos)
    {
	_variables["basedir"] = xmlFile.substr(0, end);
    }

    if(_variables["basedir"].empty())
    {
	_variables["basedir"] = ".";
    }
    
    SAXParser* parser = new SAXParser;
    try
    {
	parser->setValidationScheme(SAXParser::Val_Never);
	ComponentErrorHandler err(*this);
	parser->setDocumentHandler(&handler);
	parser->setErrorHandler(&err);
	parser->parse(xmlFile.c_str());
    }
    catch(const ParserDeploymentWrapperException& ex)
    {
	//
	// Throw the exception wrapped in ex.
	//
	ex.throwParserDeploymentException();
    }
    catch(const SAXParseException& e)
    {
	delete parser;

	ostringstream os;
	os << xmlFile << ":" << e.getLineNumber() << ": " << toString(e.getMessage());

	ParserDeploymentException ex;
	ex.component = _variables["fqn"];
	ex.reason = os.str();
	throw ex;
    }
    catch(const SAXException& e)
    {
	delete parser;

	ostringstream os;
	os << xmlFile << ": SAXException: " << toString(e.getMessage());

	ParserDeploymentException ex;
	ex.component = _variables["fqn"];
	ex.reason = os.str();
	throw ex;
    }
    catch(const XMLException& e)
    {
	delete parser;

	ostringstream os;
	os << xmlFile << ": XMLException: " << toString(e.getMessage());

	ParserDeploymentException ex;
	ex.component = _variables["fqn"];
	ex.reason = os.str();
	throw ex;
    }
    catch(...)
    {
	delete parser;

	ostringstream os;
	os << xmlFile << ": unknown exception while parsing file";

	ParserDeploymentException ex;
	ex.component = _variables["fqn"];
	ex.reason = os.str();
	throw ex;
    }

    int rc = parser->getErrorCount();
    delete parser;

    if(rc > 0)
    {
	ParserDeploymentException ex;
	ex.component = _variables["fqn"];
	ex.reason = xmlFile + ": parse error";
	throw ex;
    }
}

void
IcePack::ComponentBuilder::setDocumentLocator(const Locator* locator)
{
    _locator = locator;
}

bool
IcePack::ComponentBuilder::isTargetDeployable(const string& target) const
{
    map<string, string>::const_iterator p = _variables.find("fqn");
    assert(p != _variables.end());
    const string fqn = p->second;

    for(vector<string>::const_iterator p = _targets.begin(); p != _targets.end(); ++p)
    {
	if((*p) == target)
	{
	    return true;
	}
	else
	{
	    string componentTarget;
	    string::size_type end = 0;
	    while(end != string::npos)
	    {
		//
		// Add the first component name from the component
		// fully qualified name to the target and see if
		// matches.
		//
		end = fqn.find('.', end);
		if(end == string::npos)
		{
		    componentTarget = fqn + "." + target;
		}
		else
		{
		    componentTarget = fqn.substr(0, end) + "." + target;
		    ++end;
		}

		if((*p) == componentTarget)
		{
		    return true;
		}
	    }
	}
    }

    return false;
}

void
IcePack::ComponentBuilder::execute()
{
    vector<TaskPtr>::iterator p;
    for(p = _tasks.begin(); p != _tasks.end(); p++)
    {
	try
	{
	    (*p)->execute();
	}
	catch(DeploymentException& ex)
	{
	    if(ex.component.empty())
	    {
		ex.component = _variables["fqn"];
	    }
	    undoFrom(p);
	    throw;
	}
    }
}

void
IcePack::ComponentBuilder::undo()
{
    for(vector<TaskPtr>::reverse_iterator p = _tasks.rbegin(); p != _tasks.rend(); p++)
    {
	try
	{
	    (*p)->undo();
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << "exception while removing component " << (ex.component.empty() ? _variables["fqn"] : ex.component);
	    os << ":\n" << ex << ": " << ex.reason;

	    _communicator->getLogger()->warning(os.str());
	}
    }
}

void
IcePack::ComponentBuilder::createDirectory(const string& name, bool force)
{
    string path = _variables["datadir"] + (name.empty() || name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new CreateDirectory(path, force));
}

void
IcePack::ComponentBuilder::createConfigFile(const string& name)
{
    assert(!name.empty());
    _configFile = _variables["datadir"] + (name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new GenerateConfiguration(_configFile, _properties));
}

void
IcePack::ComponentBuilder::addProperty(const string& name, const string& value)
{
    _properties->setProperty(name, value);
}

void
IcePack::ComponentBuilder::addOffer(const string& offer, const string& adapterId, const string& identity)
{
    assert(!adapterId.empty());

    if(!_yellowAdmin)
    {	
	string msg = "Can't find a running Yellow service to deploy offers";
	throw DeploySAXParseException(msg, _locator);
    }

    Ice::ObjectPrx object = _communicator->stringToProxy(identity + "@" + adapterId);
    assert(object);
    
    _tasks.push_back(new RegisterOffer(_yellowAdmin, offer, object));
}

void
IcePack::ComponentBuilder::addObject(const string& id, const string& adapterId, const string& type)
{
    assert(!adapterId.empty());

    ObjectDescription desc;
    desc.proxy = _communicator->stringToProxy(id + "@" + adapterId);
    desc.type = type;
    desc.adapterId = adapterId;
    
    _tasks.push_back(new RegisterObject(_objectRegistry, desc));
}

void
IcePack::ComponentBuilder::overrideBaseDir(const string& basedir)
{    
    if(basedir[0] == '/')
    {
	_variables["basedir"] = basedir;
    }
    else
    {
	_variables["basedir"] += "/" + basedir;
    }
}

//
// Compute an adapter id for a given adapter name.
//
string
IcePack::ComponentBuilder::getDefaultAdapterId(const string& name)
{
    //
    // Concatenate the component name to the adapter name.
    //
    return name + "-" + _variables["name"];
}

//
// Returns a path including the base directory if path is a relative
// path.
//
string
IcePack::ComponentBuilder::toLocation(const string& path) const
{
    if(path.empty())
    {
	return "";
    }

    map<string, string>::const_iterator p = _variables.find("basedir");
    assert(p != _variables.end());
    return path[0] != '/' ? p->second + "/" + path : path;
}

//
// Substitute variables with their values.
//
string
IcePack::ComponentBuilder::substitute(const string& v) const
{
    string value(v);
    string::size_type beg;
    string::size_type end = 0;

    while((beg = value.find("${")) != string::npos)
    {
	end = value.find("}", beg);
	
	if(end == string::npos)
	{
	    throw DeploySAXParseException("malformed variable name in the '" + value + "' value", _locator);
	}

	
	string name = value.substr(beg + 2, end - beg - 2);
	map<string, string>::const_iterator p = _variables.find(name);
	if(p == _variables.end())
	{
	    throw DeploySAXParseException("unknown variable name in the '" + value + "' value", _locator);
	}

	value.replace(beg, end - beg + 1, p->second);
    }

    return value;
}

vector<string>
IcePack::ComponentBuilder::toTargets(const string& targets) const
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

    return result;
}

void
IcePack::ComponentBuilder::undoFrom(vector<TaskPtr>::iterator p)
{
    if(p != _tasks.begin())
    {
	for(vector<TaskPtr>::reverse_iterator q(p); q != _tasks.rend(); q++)
	{
	    try
	    {
		(*q)->undo();
	    }
	    catch(const DeploymentException& ex)
	    {
		ostringstream os;
		os << "exception while removing component " << _variables["fqn"] << ": " << ex.reason << ":" << endl;
		os << ex;
		
		_communicator->getLogger()->warning(os.str());
	    }
	}
    }
}

