// **********************************************************************
//
// Copyright (c) 2003
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

#ifdef __sun
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <Ice/Ice.h>
#include <IcePack/ComponentBuilder.h>
#include <IcePack/Internal.h>

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

	    DIR* dir = opendir(_name.c_str());
	    
	    if(dir == 0)
	    {
		// TODO: log a warning, throw an exception?
		return;
	    }
	    
	    
	    // TODO: make the allocation/deallocation exception-safe
	    struct dirent* entry = static_cast<struct dirent*>(malloc(pathconf(_name.c_str(), _PC_NAME_MAX) + 1));

	    Ice::StringSeq entries;
	   
	    while(readdir_r(dir, entry, &entry) == 0 && entry != 0)
	    {
		string name = entry->d_name;
		entries.push_back(_name + "/" + name);
	    }
	    free(entry);
	    closedir(dir);
		
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
	    os << "couldn't add the object:\n" << lex;

	    ObjectDeploymentException ex;
	    ex.reason = os.str();
	    ex.proxy = _desc.proxy;
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the object registry:\n" << lex;

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
	    os << "couldn't remove the object:\n" << ex;

	    ObjectDeploymentException ode;
	    ode.reason = os.str();
	    ode.proxy = _desc.proxy;
	    throw ode;
	}	
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the object registry:\n" << lex;
	    
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

IcePack::ComponentHandler::ComponentHandler(ComponentBuilder& builder) :
    _builder(builder),
    _isCurrentTargetDeployable(true)
{
}

void
IcePack::ComponentHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int)
{
    _elements.push("");

    if(!isCurrentTargetDeployable())
    {
	return;
    }

    if(name == "variable")
    {
	string value = getAttributeValueWithDefault(attrs, "value", "");
	if(value.empty())
	{
	    value = _builder.toLocation(getAttributeValueWithDefault(attrs, "location", ""));
	}
	_builder.addVariable(getAttributeValue(attrs, "name"), value);
    }

    _builder.pushVariables();

    if(name == "property")
    {
	string value = getAttributeValueWithDefault(attrs, "value", "");
	if(value.empty())
	{
	    value = _builder.toLocation(getAttributeValueWithDefault(attrs, "location", ""));
	}
	_builder.addProperty(getAttributeValue(attrs, "name"), value);
    }
    else if(name == "adapter")
    {
	if(!_currentAdapterId.empty())
	{
            ostringstream ostr;
            ostr << "line " << line << ": Adapter element enclosed in an adapter element is not allowed";
	    throw IceXML::ParserException(__FILE__, __LINE__, ostr.str());
	}

	//
	// If the id is not specified, we ask the builder to generate
	// an id for us based on the adapter name.
	//
	string adapterName = getAttributeValue(attrs, "name");
	if(adapterName.empty())
	{
            ostringstream ostr;
            ostr << "line " << line << ": empty adapter name";
	    throw IceXML::ParserException(__FILE__, __LINE__, ostr.str());
	}
	_currentAdapterId = getAttributeValueWithDefault(attrs, "id", _builder.getDefaultAdapterId(adapterName));
    }
    else if(name == "object")
    {
	_builder.addObject(getAttributeValue(attrs, "identity"), 
			   _currentAdapterId,
			   getAttributeValue(attrs, "type"));
    }
    else if(name == "target")
    {
	if(!_currentTarget.empty())
	{
            ostringstream ostr;
            ostr << "line " << line << ": Target element enclosed in a target element is not allowed";
	    throw IceXML::ParserException(__FILE__, __LINE__, ostr.str());
	}
	_isCurrentTargetDeployable = _builder.isTargetDeployable(getAttributeValue(attrs, "name"));
    }
}

void
IcePack::ComponentHandler::endElement(const string& name, int, int)
{
    _elements.pop();

    if(name == "target")
    {
	_isCurrentTargetDeployable = true;
    }

    if(isCurrentTargetDeployable())
    {
	_builder.popVariables();
	if(name == "adapter")
	{
	    _currentAdapterId = "";
	}
    }
}

void
IcePack::ComponentHandler::characters(const string& chars, int, int)
{
    _elements.top().assign(chars);
}

string
IcePack::ComponentHandler::getAttributeValue(const IceXML::Attributes& attrs, const string& name) const
{
    IceXML::Attributes::const_iterator p = attrs.find(name);
    if(p == attrs.end())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "missing attribute '" + name + "'");
    }

    return _builder.substitute(p->second);
}

string
IcePack::ComponentHandler::getAttributeValueWithDefault(const IceXML::Attributes& attrs, const string& name, 
							const string& def) const
{
    IceXML::Attributes::const_iterator p = attrs.find(name);
    if(p == attrs.end())
    {
        return _builder.substitute(def);
    }
    else
    {
        return _builder.substitute(p->second);
    }
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
    _targets(targets)
{
    _variables.push_back(variables);
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
	setVariable("basedir", xmlFile.substr(0, end));
    }

    if(getVariable("basedir").empty())
    {
	setVariable("basedir", ".");
    }
    
    try
    {
        IceXML::Parser::parse(xmlFile, handler);
    }
    catch(const IcePack::ParserDeploymentException& ex)
    {
	throw ex;
    }
    catch(const IceXML::ParserException& e)
    {
	ostringstream os;
	os << xmlFile << ": " << e;

	ParserDeploymentException ex;
	ex.component = getVariable("fqn");
	ex.reason = os.str();
	throw ex;
    }
    catch(...)
    {
	ostringstream os;
	os << xmlFile << ": unknown exception while parsing file";

	ParserDeploymentException ex;
	ex.component = getVariable("fqn");
	ex.reason = os.str();
	throw ex;
    }
}

bool
IcePack::ComponentBuilder::isTargetDeployable(const string& target) const
{
    const string fqn = getVariable("fqn");

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
		ex.component = getVariable("fqn");
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
	    os << "exception while removing component ";
	    os << (ex.component.empty() ? getVariable("fqn") : ex.component);
	    os << ":\n" << ex << ": " << ex.reason;

	    _communicator->getLogger()->warning(os.str());
	}
    }
}

void
IcePack::ComponentBuilder::createDirectory(const string& name, bool force)
{
    string path = getVariable("datadir") + (name.empty() || name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new CreateDirectory(path, force));
}

void
IcePack::ComponentBuilder::createConfigFile(const string& name)
{
    assert(!name.empty());
    _configFile = getVariable("datadir") + (name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new GenerateConfiguration(_configFile, _properties));
}

void
IcePack::ComponentBuilder::addProperty(const string& name, const string& value)
{
    _properties->setProperty(name, value);
}

void
IcePack::ComponentBuilder::addVariable(const string& name, const string& value)
{
    setVariable(name, value);
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
	setVariable("basedir", basedir);
    }
    else
    {
	setVariable("basedir", getVariable("basedir") + "/" + basedir);
    }
}

const string&
IcePack::ComponentBuilder::getVariable(const string& name) const
{
    static const string empty;

    vector< map< string, string> >::const_reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	map<string, string>::const_iterator q = p->find(name);
	if(q != p->end())
	{
	    return q->second;
	}
	++p;
    }
    return empty;
}

void
IcePack::ComponentBuilder::setVariable(const string& name, const string& value)
{
    _variables.back()[name] = value;
}

bool
IcePack::ComponentBuilder::findVariable(const string& name) const
{
    vector< map< string, string> >::const_reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	map<string, string>::const_iterator q = p->find(name);
	if(q != p->end())
	{
	    return true;
	}
	++p;
    }
    return false;
}

void
IcePack::ComponentBuilder::pushVariables()
{
    _variables.push_back(map<string, string>());
}

void
IcePack::ComponentBuilder::popVariables()
{
    _variables.pop_back();
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
    return name + "-" + getVariable("name");
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

    return path[0] != '/' ? getVariable("basedir") + "/" + path : path;
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
	    throw IceXML::ParserException(__FILE__, __LINE__, "malformed variable name in the '" + value + "' value");
	}

	
	string name = value.substr(beg + 2, end - beg - 2);
	if(!findVariable(name))
	{
	    throw IceXML::ParserException(__FILE__, __LINE__, "unknown variable name in the '" + value + "' value");
	}

	value.replace(beg, end - beg + 1, getVariable(name));
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
		os << "exception while removing component " << getVariable("fqn") << ": ";
		os << ex.reason << ":" << endl;
		os << ex;
		
		_communicator->getLogger()->warning(os.str());
	    }
	}
    }
}

