// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>

#include <IcePack/ComponentDeployer.h>
#include <IcePack/Admin.h>

#include <Yellow/Yellow.h>

#include <parsers/SAXParser.hpp>
#include <sax/HandlerBase.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include <stack>
#include <iterator>
#include <fstream>

using namespace std;
using namespace IcePack;

void IcePack::incRef(Task* p) { p->__incRef(); }
void IcePack::decRef(Task* p) { p->__decRef(); }

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
    deploy()
    {
	if(mkdir(_name.c_str(), 0755) != 0)
	{
	    cerr << "Can't create directory: " << _name << endl;

	    Ice::SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
    }

    virtual void
    undeploy()
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
	    if(n < 0)
	    {
		Ice::SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }

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
			Ice::SystemException ex(__FILE__, __LINE__);
			ex.error = getSystemErrno();
			throw ex;
		    }
		}
		else if(S_ISREG(buf.st_mode))
		{
		    if(unlink(p->c_str()) != 0)
		    {
			Ice::SystemException ex(__FILE__, __LINE__);
			ex.error = getSystemErrno();
			throw ex;
		    }
		}
	    }
	}

	if(rmdir(_name.c_str()) != 0)
	{
	    cerr << "Can't remove directory: " << _name << endl;

	    Ice::SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
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
    deploy()  
    {
	ofstream configfile;
	configfile.open(_file.c_str(), ios::out);
	if(!configfile)
	{
	    cerr << "Can't create configuration file: " << _file << endl;
	    Ice::SystemException ex(__FILE__, __LINE__);
	    throw ex;
	}
	
	Ice::PropertyDict props = _properties->getPropertiesForPrefix("");
	transform(props.begin(), props.end(), ostream_iterator<string>(configfile,"\n"), WriteConfigProperty());
	configfile.close();
    }

    virtual void
    undeploy()
    {
	if(unlink(_file.c_str()) != 0)
	{
	    cerr << "Can't remove configuration file: " << _file << endl;

	    Ice::SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
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
    deploy()
    {
	_admin->add(_offer, _proxy);
    }

    virtual void
    undeploy()
    {
	try
	{
	    _admin->remove(_offer, _proxy);
	}
	catch(const Yellow::NoSuchOfferException&)
	{
	    //
	    // TODO: The offer doesn't exist anymore so no need to
	    // worry about removing it. We should print a warning
	    // though.
	    //
	}	
    }

private:

    Yellow::AdminPrx _admin;
    string _offer;
    Ice::ObjectPrx _proxy;
};

}

IcePack::ComponentErrorHandler::ComponentErrorHandler(ComponentDeployer& deployer) :
    _deployer(deployer)
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
    string s = toString(exception.getMessage());
    cerr << "error: " << s << endl;
}

void
IcePack::ComponentErrorHandler::fatalError(const SAXParseException& exception)
{
    string s = toString(exception.getMessage());
    cerr << "fatal:" << s << endl;
}

void
IcePack::ComponentErrorHandler::resetErrors()
{
}

IcePack::ComponentDeployHandler::ComponentDeployHandler(ComponentDeployer& deployer) :
    _deployer(deployer)
{
}

void
IcePack::ComponentDeployHandler::characters(const XMLCh *const chars, const unsigned int length)
{
    _elements.top().assign(toString(chars));
}

void
IcePack::ComponentDeployHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    _elements.push("");

    string str = toString(name);

    if(str == "property")
    {
	string value = getAttributeValueWithDefault(attrs, "value", "");
	if(value.empty())
	{
	    value = getAttributeValueWithDefault(attrs, "location", "");
	    if(!value.empty() && value[0] != '/')
	    {
		value = _deployer.substitute("${basedir}/") + value;
	    }
	}
	_deployer.addProperty(getAttributeValue(attrs, "name"), value);
    }
    else if(str == "adapter")
    {
	_adapter = getAttributeValue(attrs, "name");
    }
    else if(str == "offer")
    {
	_deployer.addOffer(getAttributeValue(attrs, "interface"), _adapter, getAttributeValue(attrs, "identity"));
    }
}

void
IcePack::ComponentDeployHandler::endElement(const XMLCh *const name)
{
    _elements.pop();

    string str = toString(name);

    if(str == "adapter")
    {
	_adapter = "";
    }
}

string
IcePack::ComponentDeployHandler::getAttributeValue(const AttributeList& attrs, const string& name) const
{
    XMLCh* n = XMLString::transcode(name.c_str());
    const XMLCh* value = attrs.getValue(n);
    delete[] n;
    
    if(value == 0)
    {
	cerr << "Missing attribute '" << name << "'" << endl;
	return "";
    }

    return _deployer.substitute(toString(value));
}

string
IcePack::ComponentDeployHandler::getAttributeValueWithDefault(const AttributeList& attrs, const string& name, 
							      const string& def) const
{
    XMLCh* n = XMLString::transcode(name.c_str());
    const XMLCh* value = attrs.getValue(n);
    delete[] n;
    
    if(value == 0)
    {
	return _deployer.substitute(def);
    }
    else
    {
	return _deployer.substitute(toString(value));
    }
}

string
IcePack::ComponentDeployHandler::toString(const XMLCh* ch) const
{
    return IcePack::toString(ch);
}

string
IcePack::ComponentDeployHandler::elementValue() const
{
    return _elements.top();
}

IcePack::ComponentDeployer::ComponentDeployer(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _properties(Ice::createProperties())
{
    string serversPath = _communicator->getProperties()->getProperty("IcePack.Data");
    assert(!serversPath.empty());
    _variables["datadir"] = serversPath + (serversPath[serversPath.length() - 1] == '/' ? "" : "/") + "servers";
}

void 
IcePack::ComponentDeployer::parse(const string& xmlFile, ComponentDeployHandler& handler)
{
    _error = 0;

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
    parser->setValidationScheme(SAXParser::Val_Never);

    try
    {
	ComponentErrorHandler err(*this);
	parser->setDocumentHandler(&handler);
	parser->setErrorHandler(&err);
	parser->parse(xmlFile.c_str());
    }
    catch(const XMLException& e)
    {
	cerr << "XMLException: " << toString(e.getMessage()) << endl;
	_error++;
    }
    int rc = parser->getErrorCount();
    delete parser;

    if(_error > 0 || rc > 0)
    {
	throw DeploymentException();
    }    
}

void
IcePack::ComponentDeployer::deploy()
{
    vector<TaskPtr>::iterator p;
    for(p = _tasks.begin(); p != _tasks.end(); p++)
    {
	try
	{
	    (*p)->deploy();
	}
	catch(const DeploymentException& ex)
	{
	    cerr << "Deploy: " << ex << endl;
	    undeployFrom(p);
	    throw;
	}
	catch(const Ice::SystemException& ex)
	{
	    cerr << "Deploy: " << ex << endl;
	    undeployFrom(p);
	    throw DeploymentException();;
	}
    }
}

void
IcePack::ComponentDeployer::undeploy()
{
    for(vector<TaskPtr>::reverse_iterator p = _tasks.rbegin(); p != _tasks.rend(); p++)
    {
	try
	{
	    (*p)->undeploy();
	}
	catch(const DeploymentException& ex)
	{
	    //
	    // TODO: we probably need to log the failure to execute
	    // this task so that the use can take necessary steps to
	    // ensure it's correctly undeployed.
	    //
	    cerr << "Undeploy: " << ex << endl;
	}
	catch(const Ice::SystemException& ex)
	{
	    cerr << "Undeploy: " << ex << endl;
	}
    }
}

void
IcePack::ComponentDeployer::createDirectory(const string& name, bool force)
{
    string path = _variables["datadir"] + (name.empty() || name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new CreateDirectory(path, force));
}

void
IcePack::ComponentDeployer::createConfigFile(const string& name)
{
    assert(!name.empty());
    _configFile = _variables["datadir"] + (name[0] == '/' ? name : "/" + name);
    _tasks.push_back(new GenerateConfiguration(_configFile, _properties));
}

void
IcePack::ComponentDeployer::addProperty(const string& name, const string& value)
{
    _properties->setProperty(name, value);
}

void
IcePack::ComponentDeployer::addOffer(const string& offer, const string& adapter, const string& identity)
{
    assert(!adapter.empty());

    Yellow::AdminPrx yellowAdmin;    
    try
    {
	//
	// TODO: Find a better way to bootstrap the yellow service. We
	// need to set the locator on the proxy here, because the
	// communicator doesn't have a default locator since it's the
	// locator communicator...
	//
	Ice::ObjectPrx object = _communicator->stringToProxy(
	    _communicator->getProperties()->getProperty("IcePack.Yellow.Admin"));

	if(!object)
	{	
	    cerr << "IcePack.Yellow.Admin is not set, can't register the offer '" << offer << "'" << endl;
	    _error++;
	    return;	
	}

	Ice::LocatorPrx locator = Ice::LocatorPrx::uncheckedCast(
	    _communicator->stringToProxy(_communicator->getProperties()->getProperty("Ice.Default.Locator")));

	yellowAdmin = Yellow::AdminPrx::checkedCast(object->ice_locator(locator));
    }
    catch(Ice::LocalException& ex)
    {
	cerr << "Couldn't contact the yellow service to register the offer '" << offer << "':\n" << ex << endl;
	_error++;
	return;	
    }

    Ice::ObjectPrx object = _communicator->stringToProxy(identity + "@" + adapter);
    assert(object);
    
    _tasks.push_back(new RegisterOffer(yellowAdmin, offer, object));
}

void
IcePack::ComponentDeployer::overrideBaseDir(const string& basedir)
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
// Substitute variables with their values.
//
string
IcePack::ComponentDeployer::substitute(const string& v) const
{
    string value(v);
    string::size_type beg;
    string::size_type end = 0;

    while((beg = value.find("${")) != string::npos)
    {
	end = value.find("}", beg);
	
	if(end == string::npos)
	{
	    cerr << "Malformed variable name in : " << value << endl;
	    break; // Throw instead?
	}

	
	string name = value.substr(beg + 2, end - beg - 2);
	map<string, string>::const_iterator p = _variables.find(name);
	if(p == _variables.end())
	{
	    cerr << "Unknown variable: " << name << endl;
	    break; // Throw instead?
	}

	value.replace(beg, end - beg + 1, p->second);
    }

    return value;
}

void
IcePack::ComponentDeployer::undeployFrom(vector<TaskPtr>::iterator p)
{
    if(p != _tasks.begin())
    {
	for(vector<TaskPtr>::reverse_iterator q(p); q != _tasks.rend(); q++)
	{
	    try
	    {
		(*q)->undeploy();
	    }
	    catch(DeploymentException& ex)
	    {
		cerr << "Undeploy: " << ex << endl;
	    }
	    catch(Ice::SystemException& ex)
	    {
		cerr << "Undeploy: " << ex << endl;
	    }
	}
    }
}

