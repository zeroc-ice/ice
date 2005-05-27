// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/OutputUtil.h>
#include <Ice/Ice.h>
#include <IceXML/Parser.h>
#include <IceGrid/Parser.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorHelper.h>

#ifdef GPL_BUILD
#   include <IceGrid/GPL.h>
#endif

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#include <iterator>
#include <iomanip>

extern FILE* yyin;
extern int yydebug;

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

Parser* parser;

string
toString(const vector<string>& v)
{
    ostringstream os;
    Ice::StringSeq::const_iterator p = v.begin();
    while(p != v.end())
    {
	os << *p;
	++p;
	if(p != v.end())
	{
	    os << " ";
	}
    }
    return os.str();
}

void 
describeDbEnv(Output& out, const DbEnvDescriptor& dbEnv)
{
    //
    // Database environments
    //
    out << nl << "database environment '" << dbEnv.name << "'";
    if(!dbEnv.dbHome.empty() || !dbEnv.properties.empty())
    {
	out << sb;
	if(!dbEnv.dbHome.empty())
	{
	    out << nl << "home = '" << dbEnv.dbHome << "'";
	}
	if(!dbEnv.properties.empty())
	{
	    out << nl << "properties";
	    out << sb;
	    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
	    {
		out << nl << p->name << " = " << p->value;
	    }
	    out << eb;
	}
	out << eb;
    }
}

void
describeObjectAdapter(Output& out, const Ice::CommunicatorPtr& communicator, const AdapterDescriptor& adapter)
{
    out << nl << "adapter '" << adapter.name << "'";
    out << sb;
    out << nl << "id = '" << adapter.id << "'";
    out << nl << "endpoints = '" << adapter.endpoints << "'";
    out << nl << "register process = '" << (adapter.registerProcess ? "true" : "false") << "'";
    for(ObjectDescriptorSeq::const_iterator p = adapter.objects.begin(); p != adapter.objects.end(); ++p)
    {
	out << nl << "object";
	if(!p->type.empty())
	{
	    out << sb;
	    out << nl << "proxy = '" << communicator->proxyToString(p->proxy) << "' ";
	    out << nl << "type = '" << p->type << "'";
	    out << eb;
	}
    }
    out << eb;
}

void
describeProperties(Output& out, const PropertyDescriptorSeq& properties)
{
    out << nl << "properties";
    out << sb;
    for(PropertyDescriptorSeq::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
	out << nl << p->name << " = " << p->value;
    }
    out << eb;
}

void
describeComponent(Output& out, const Ice::CommunicatorPtr& communicator, const ComponentDescriptorPtr& desc)
{
    if(!desc->comment.empty())
    {
	out << nl << "comment";
	out << sb;
	out << nl << desc->comment;
	out << eb;
    }
    if(!desc->properties.empty())
    {
	describeProperties(out, desc->properties);
    }
    {
	for(DbEnvDescriptorSeq::const_iterator p = desc->dbEnvs.begin(); p != desc->dbEnvs.end(); ++p)
	{
	    describeDbEnv(out, *p);
	}
    }
    {
	for(AdapterDescriptorSeq::const_iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
	{
	    describeObjectAdapter(out, communicator, *p);
	}
    }
}

void
describeService(Output& out, const Ice::CommunicatorPtr& communicator, const ServiceDescriptorPtr& service)
{
    out << nl << "entry = '" << service->entry << "'";
    describeComponent(out, communicator, service);
}

void describe(Output& out, const Ice::CommunicatorPtr&, const InstanceDescriptor&);

void
describeServer(Output& out, const Ice::CommunicatorPtr& communicator, const ServerDescriptorPtr& server)
{
    out << nl << "node = '" << server->node << "'";
    out << nl << "application = '" << server->application << "'";
    if(!server->interpreter.empty())
    {
	out << nl << "interpreter = '" << server->interpreter << "'";
    }
    out << nl << "exe = '" << server->exe << "'";
    if(!server->pwd.empty())
    {
	out << nl << "pwd = '" << server->pwd << "'";
    }
    out << nl << "activation = '" << server->activation << "'";

    if(!server->interpreterOptions.empty())
    {
	out << nl << "interpreterOptions = '" << toString(server->interpreterOptions) << "'";
    }
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server);
    if(iceBox)
    {
	out << nl << "service manager endpoints = '" << iceBox->endpoints << "'";
    }
    if(!server->options.empty())
    {
	out << nl << "options = '" << toString(server->options) << "'";
    }
    if(!server->envs.empty())
    {
	out << nl << "envs = '" << toString(server->envs) << "'";
    }

    describeComponent(out, communicator, server);

    //
    // Services
    //
    if(iceBox)
    {
	for(InstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    describe(out, communicator, *p);
	}
    }
}

void
describe(Output& out, const Ice::CommunicatorPtr& communicator, const string& id, const TemplateDescriptor& templ)
{
    ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(templ.descriptor);
    if(server)
    {
	out << "server template '" << id << "'";
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server);
	if(iceBox)
	{
	    out << " (IceBox)";
	}
    }
    ServiceDescriptorPtr service = ServiceDescriptorPtr::dynamicCast(templ.descriptor);
    if(service)
    {
	out << nl << "service template '" << id << "'";
    }

    out << sb;
    if(!templ.parameters.empty())
    {
	out << nl << "parameters = '" << toString(templ.parameters) << "'";
    }
    if(!server->variables.empty())
    {
	out << nl << "variables";
	out << sb;
	for(StringStringDict::const_iterator p = server->variables.begin(); p != server->variables.end(); ++p)
	{
	    out << nl << p->first << " = " << p->second;
	}
	out << eb;
    }

    out << nl << "name = '" << server->name << "'";    
    if(server)
    {
	describeServer(out, communicator, server);
	out << eb;
	out << nl;
    }
    if(service)
    {
	describeService(out, communicator, service);
	out << eb;
    }
}


void
describe(Output& out, const Ice::CommunicatorPtr& communicator, const InstanceDescriptor& inst)
{
    ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(inst.descriptor);
    if(server)
    {
	out << "server '" << server->name << "' ";
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server);
	if(iceBox)
	{
	    out << " (IceBox)";
	}
	out << sb;
	describeServer(out, communicator, server);	
	out << eb;
    }

    ServiceDescriptorPtr service = ServiceDescriptorPtr::dynamicCast(inst.descriptor);
    if(service)
    {
	out << nl << "service '" << service->name << "'";
	out << sb;
	describeService(out, communicator, service);
	out << eb;
    }

    out << nl;
}

}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query)
{
    return new Parser(communicator, admin, query);
}

void
Parser::usage()
{
    cout <<
        "help                        Print this message.\n"
        "exit, quit                  Exit this program.\n"
	"\n"
	"application add DESC [TARGET ... ] [NAME=VALUE ... ]\n"
	"                            Add application described in DESC. If specified\n"
        "                            the optional targets TARGET will be deployed.\n"
	"application remove NAME     Remove application NAME.\n"
	"application describe NAME   Describe application NAME.\n"
	"application server template describe NAME TEMPLATE\n"
        "                            Describe application NAME server template TEMPLATE.\n"
	"application service template describe NAME TEMPLATE\n"
        "                            Describe application NAME service template TEMPLATE.\n"
	"application describe NAME   Describe application NAME.\n"
	"application diff DESC [TARGET ... ] [NAME=VALUE ... ]\n"
        "                            Print the differences betwen the application\n"
        "                            described in DESC and the current deployment.\n"
	"application update DESC [TARGET ... ] [NAME=VALUE ... ]\n"
	"                            Update the application described in DESC.\n"
	"application instantiate NAME TEMPLATE [NAME=VALUE ...]\n"
	"                            Instantiate a server template and add the server\n"
	"application list            List all deployed applications.\n"
	"                            to the application."
        "\n"
	"node list                   List all registered nodes.\n"
	"node ping NAME              Ping node NAME.\n"
        "node remove NAME            Remove the servers deployed on node NAME and\n"
        "                            the node NAME.\n"
	"node shutdown NAME          Shutdown node NAME.\n"
	"\n"
        "server list                 List all registered servers.\n"
        "server add DESC NODE [TARGET ... ] [NAME=VALUE ... ]\n"
        "                            Add server described in descriptor DESC to the node\n"
        "                            NODE. If specified the optional targets TARGET will\n"
        "                            be deployed.\n"
        "server update DESC NODE [TARGET ... ] [NAME=VALUE ... ]\n"
        "                            Update server described in descriptor DESC on the\n"
        "                            node NODE. If specified the optional targets TARGET\n"
        "                            will be deployed.\n"
        "server remove NAME          Remove server NAME.\n"
        "server describe NAME        Describe server NAME.\n"
	"server state NAME           Get server NAME state.\n"
	"server pid NAME             Get server NAME pid.\n"
	"server start NAME           Start server NAME.\n"
	"server stop NAME            Stop server NAME.\n"
        "server signal NAME SIGNAL   Send SIGNAL (e.g. SIGTERM or 15) to server NAME.\n"
        "server stdout NAME MESSAGE  Write MESSAGE on server NAME's stdout.\n"
	"server stderr NAME MESSAGE  Write MESSAGE on server NAME's stderr.\n"
	"server activation NAME [on-demand | manual] \n"
	"                            Set the server activation mode to on-demand or\n"
	"                            manual."
	"\n"
        "adapter list                List all registered adapters.\n"
	"adapter endpoints NAME      Get endpoints of adapter NAME.\n"
	"\n"
	"object add PROXY [TYPE]     Add an object to the object registry,\n"
	"                            optionally specifying its type.\n"
	"object remove IDENTITY      Remove an object from the object registry.\n"
	"object find TYPE            Find all objects with the type TYPE.\n"
	"object describe EXPR        Describe all registered objects whose stringified\n"
        "                            identities match the expression EXPR. A trailing\n"
	"                            wildcard is supported in EXPR, for example\n"
	"                            \"object describe Ice*\".\n"
	"object list EXPR            List all registered objects whose stringified\n"
        "                            identities match the expression EXPR. A trailing\n"
	"                            wildcard is supported in EXPR, for example\n"
	"                            \"object list Ice*\".\n"
	"\n"
        "shutdown                    Shut the IceGrid registry down.\n"
#ifdef GPL_BUILD
	"show copying                Show conditions for redistributing copies of this\n"
	"                            program.\n"
	"show warranty               Show the warranty for this program.\n"
#endif
	;
}

void
Parser::addApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application add' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	StringSeq targets;
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string descriptor = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	    else
	    {
		targets.push_back(*p);
	    }
	}

	_admin->addApplication(DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator));
    }
    catch(const IceXML::ParserException& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::removeApplication(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`application remove' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;

	_admin->removeApplication(name);
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::describeApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application describe' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;

	ApplicationDescriptorPtr application = _admin->getApplicationDescriptor(name);
	
	Output out(cout);
	out << "application '" << application->name << "'";
	out << sb;
	if(!application->comment.empty())
	{
	    out << nl << "comment = " << application->comment;
	}

	if(!application->serverTemplates.empty())
	{
	    out << nl << "server templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = application->serverTemplates.begin();
		p != application->serverTemplates.end();
		++p)
	    {
		out << nl << p->first;
	    }
	    out << eb;
	}
	if(!application->serviceTemplates.empty())
	{
	    out << nl << "service templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = application->serviceTemplates.begin();
		p != application->serviceTemplates.end();
		++p)
	    {
		out << nl << p->first;
	    }
	    out << eb;
	}
	if(!application->servers.empty())
	{
	    map<string, set<string> > servers;
	    {
		for(InstanceDescriptorSeq::const_iterator p = application->servers.begin(); 
		    p != application->servers.end();
		    ++p)
		{
		    const ServerDescriptorPtr descriptor = ServerDescriptorPtr::dynamicCast(p->descriptor);
		    map<string, set<string> >::iterator q = servers.find(descriptor->node);
		    if(q == servers.end())
		    {
			q = servers.insert(make_pair(descriptor->node, set<string>())).first;
		    }
		    q->second.insert(descriptor->name);
		}
	    }
	    {
		for(map<string, set<string> >::const_iterator p = servers.begin(); p != servers.end(); ++p)
		{
		    out << nl << "node '" << p->first << "'";
		    out << sb;
		    for(set<string>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
		    {
			out << nl << *q;
		    }
		    out << eb;
		}
	    }
	}
	out << eb;
	out << nl;
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::describeApplicationServerTemplate(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`application server template describe' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string templ = *p++;

	ApplicationDescriptorPtr application = _admin->getApplicationDescriptor(name);
	
	Output out(cout);
	TemplateDescriptorDict::const_iterator q = application->serverTemplates.find(templ);
	if(q != application->serverTemplates.end())
	{
	    describe(out, _communicator, templ, q->second);
	}
	else
	{
	    error("no server template with id `" + templ + "'");
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::describeApplicationServiceTemplate(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`application service template describe' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string templ = *p++;

	ApplicationDescriptorPtr application = _admin->getApplicationDescriptor(name);
	
	Output out(cout);
	TemplateDescriptorDict::const_iterator q = application->serviceTemplates.find(templ);
	if(q != application->serviceTemplates.end())
	{
	    describe(out, _communicator, templ, q->second);
	}
	else
	{
	    error("no service template with id `" + templ + "'");
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}


void
Parser::diffApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application diff' requires at least one argument\n(`help' for more info)");
	return;
    }

    ApplicationDescriptorPtr newApp;
    ApplicationDescriptorPtr origApp;

    try
    {
	StringSeq targets;
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string descriptor = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	    else
	    {
		targets.push_back(*p);
	    }
	}

	newApp = DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator);
	origApp = _admin->getApplicationDescriptor(newApp->name);
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
	return;
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
	return;
    }

    Output out(cout);
    out << "application `" << newApp->name << "'";
    out << sb;

    InstanceDescriptorSeq::const_iterator p;
    ApplicationDescriptorHelper newAppHelper(_communicator, newApp);
    ApplicationDescriptorHelper origAppHelper(_communicator, origApp);
    for(p = newApp->servers.begin(); p != newApp->servers.end(); ++p)
    {
	ServerDescriptorPtr desc = ServerDescriptorPtr::dynamicCast(p->descriptor);
	ServerDescriptorPtr orig;
	for(InstanceDescriptorSeq::const_iterator q = origApp->servers.begin(); q != origApp->servers.end(); ++q)
	{
	    if(desc->name == q->descriptor->name)
	    {
		orig = ServerDescriptorPtr::dynamicCast(q->descriptor);
		break;
	    }
	}

	if(orig)
	{
	    if(ServerDescriptorHelper(newAppHelper, desc) != ServerDescriptorHelper(origAppHelper, orig))
	    {
		out << nl << "server `" << orig->name << "' updated";
	    }
	}
	else
	{
	    out << nl << "server `" << desc->name << "' added";
	}
    }

    for(p = origApp->servers.begin(); p != origApp->servers.end(); ++p)
    {
	bool found = false;
	for(InstanceDescriptorSeq::const_iterator q = newApp->servers.begin(); q != newApp->servers.end(); ++q)
	{
	    if(p->descriptor->name == q->descriptor->name)
	    {
		found = true;
		break;
	    }
	}
	if(!found)
	{
	    out << nl << "server `" << p->descriptor->name << "' removed";
	}
    }

    out << eb;
    out << nl;
}

void
Parser::updateApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application diff' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	StringSeq targets;
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string descriptor = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	    else
	    {
		targets.push_back(*p);
	    }
	}

	_admin->updateApplication(DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator));
    }
    catch(const IceXML::ParserException& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::instantiateApplication(const list<string>& args)
{
    if(args.size() < 2)
    {
	error("`application instantiate' requires at least two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string application = *p++;
	string templ = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	}

	_admin->instantiateApplicationServer(application, templ, vars);
    }
    catch(const IceXML::ParserException& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::listAllApplications()
{
    try
    {
	Ice::StringSeq names = _admin->getAllApplicationNames();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::pingNode(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`node ping' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	if(_admin->pingNode(args.front()))
	{
	    cout << "node is up" << endl;
	}
	else
	{
	    cout << "node is down" << endl;
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::shutdownNode(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`node shutdown' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->shutdownNode(args.front());
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::removeNode(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`node remove' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->removeNode(args.front());
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::listAllNodes()
{
    try
    {
	Ice::StringSeq names = _admin->getAllNodeNames();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::addServer(const list<string>& args)
{
    if(args.size() < 2)
    {
	error("`server add' requires at least two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	StringSeq targets;
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string descriptor = *p++;
	vars["node"] = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	    else
	    {
		targets.push_back(*p);
	    }
	}

	_admin->addServer(DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator));
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::updateServer(const list<string>& args)
{
    if(args.size() < 2)
    {
	error("`server update' requires at least two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	StringSeq targets;
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string descriptor = *p++;
	vars["node"] = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	    else
	    {
		targets.push_back(*p);
	    }
	}

	_admin->updateServer(DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator));
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::startServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server start' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	if(!_admin->startServer(args.front()))
	{
	    error("the server didn't start successfully");
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::stopServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server stop' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->stopServer(args.front());
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::signalServer(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`server signal' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();
	string server = *p++;
	_admin->sendSignal(server, *p);
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}


void
Parser::writeMessage(const list<string>& args, int fd)
{
    if(args.size() != 2)
    {
	error("`server stdout or server stderr' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();
	string server = *p++;
	_admin->writeMessage(server, *p,  fd);
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::describeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server describe' requires exactly one argument\n(`help' for more info)");
	return;
    }
    
    try
    {
	InstanceDescriptor desc = _admin->getServerDescriptor(args.front());
	Output out(cout);
	describe(out, _communicator, desc);
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::removeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server remove' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->removeServer(args.front());
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::stateServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server state' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	ServerState state = _admin->getServerState(args.front());

	switch(state)
	{
	case Inactive:
	{
	    cout << "inactive" << endl;
	    break;
	}
	case Activating:
	{
	    cout << "activating" << endl;
	    break;
	}
	case Active:
	{
	    int pid = _admin->getServerPid(args.front());
	    cout << "active (pid = " << pid << ")" << endl;
	    break;
	}
	case Deactivating:
	{
	    cout << "deactivating" << endl;
	    break;
	}
	case Destroyed:
	{
	    cout << "destroyed" << endl;
	    break;
	}
	default:
	    assert(false);
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::pidServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server pid' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	cout << _admin->getServerPid(args.front()) << endl;
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::activationServer(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`server activation' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();
	string name = *p++;
	string mode = *p++;

	if(mode == "on-demand")
	{
	    _admin->setServerActivation(name, OnDemand);
	}
	else if(mode == "manual")
	{
	    _admin->setServerActivation(name, Manual);
	}
	else
	{
	    error("Unknown mode: " + mode);
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::listAllServers()
{
    try
    {
	Ice::StringSeq names = _admin->getAllServerNames();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::endpointsAdapter(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`adapter endpoints' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	string endpoints = _admin->getAdapterEndpoints(args.front());
	if(endpoints.empty())
	{
	    cout << "<inactive>" << endl;
	}
	else
	{
	    cout << endpoints << endl;
	}
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::listAllAdapters()
{
    try
    {
	Ice::StringSeq names = _admin->getAllAdapterIds();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::addObject(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`object add' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string proxy = *p++;

	if(p != args.end())
	{
	    string type = *p++;
	    _admin->addObjectWithType(_communicator->stringToProxy(proxy), type);
	}
	else
	{
	    _admin->addObject(_communicator->stringToProxy(proxy));
	}
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::removeObject(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`object remove' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->removeObject(Ice::stringToIdentity((*(args.begin()))));
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::findObject(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`object find' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	Ice::ObjectProxySeq objects = _query->findAllObjectsWithType(*(args.begin()));
	for (Ice::ObjectProxySeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    cout << _communicator->proxyToString(*p) << endl;
	}	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::describeObject(const list<string>& args)
{
    try
    {
	ObjectDescriptorSeq objects;
	if(args.size() == 1)
	{
	    string arg = *(args.begin());
	    if(arg.find('*') == string::npos)
	    {
		ObjectDescriptor desc = _admin->getObjectDescriptor(Ice::stringToIdentity(arg));
		cout << "proxy = " << _communicator->proxyToString(desc.proxy) << endl;
		cout << "type = " << desc.type << endl;
		return;
	    }
	    else
	    {
		objects = _admin->getAllObjectDescriptors(arg);
	    }
	}
	else
	{
	    objects = _admin->getAllObjectDescriptors("");
	}
	
	for(ObjectDescriptorSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    cout << "proxy = `" << _communicator->proxyToString(p->proxy) << "' type = `" << p->type << "'" << endl;
	}	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::listObject(const list<string>& args)
{
    try
    {
	ObjectDescriptorSeq objects;
	if(args.size() == 1)
	{
	    objects = _admin->getAllObjectDescriptors(*(args.begin()));
	}
	else
	{
	    objects = _admin->getAllObjectDescriptors("");
	}
	
	for(ObjectDescriptorSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    cout << Ice::identityToString(p->proxy->ice_getIdentity()) << endl;
	}	
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::shutdown()
{
    try
    {
	_admin->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::showBanner()
{
    cout << "Ice " << ICE_STRING_VERSION << "  Copyright 2003-2005 ZeroC, Inc." << endl;
#ifdef GPL_BUILD
    cout << gplBanner << endl;
#endif
}

void
Parser::showCopying()
{
#if defined(GPL_BUILD)
    cout << gplCopying << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
}

void
Parser::showWarranty()
{
#if defined(GPL_BUILD)
    cout << gplWarranty << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
}


void
Parser::getInput(char* buf, int& result, int maxSize)
{
    if(!_commands.empty())
    {
	if(_commands == ";")
	{
	    result = 0;
	}
	else
	{
#if defined(_MSC_VER) && !defined(_STLP_MSVC)
	    // COMPILERBUG: Stupid Visual C++ defines min and max as macros
	    result = _MIN(maxSize, static_cast<int>(_commands.length()));
#else
	    result = min(maxSize, static_cast<int>(_commands.length()));
#endif
	    strncpy(buf, _commands.c_str(), result);
	    _commands.erase(0, result);
	    if(_commands.empty())
	    {
		_commands = ";";
	    }
	}
    }
    else if(isatty(fileno(yyin)))
    {
#ifdef HAVE_READLINE

        const char* prompt = parser->getPrompt();
	char* line = readline(const_cast<char*>(prompt));
	if(!line)
	{
	    result = 0;
	}
	else
	{
	    if(*line)
	    {
		add_history(line);
	    }

	    result = strlen(line) + 1;
	    if(result > maxSize)
	    {
		free(line);
		error("input line too long");
		result = 0;
	    }
	    else
	    {
		strcpy(buf, line);
		strcat(buf, "\n");
		free(line);
	    }
	}

#else

	cout << parser->getPrompt() << flush;

	string line;
	while(true)
	{
	    char c = static_cast<char>(getc(yyin));
	    if(c == EOF)
	    {
		if(line.size())
		{
		    line += '\n';
		}
		break;
	    }

	    line += c;

	    if(c == '\n')
	    {
		break;
	    }
	}
	
	result = (int) line.length();
	if(result > maxSize)
	{
	    error("input line too long");
	    buf[0] = EOF;
	    result = 1;
	}
	else
	{
	    strcpy(buf, line.c_str());
	}

#endif
    }
    else
    {
	if(((result = (int) fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin))
	{
	    error("input in flex scanner failed");
	    buf[0] = EOF;
	    result = 1;
	}
    }
}

void
Parser::nextLine()
{
    _currentLine++;
}

void
Parser::continueLine()
{
    _continue = true;
}

const char*
Parser::getPrompt()
{
    assert(_commands.empty() && isatty(fileno(yyin)));

    if(_continue)
    {
	_continue = false;
	return "(cont) ";
    }
    else
    {
	return ">>> ";
    }
}

void
Parser::scanPosition(const char* s)
{
    string line(s);
    string::size_type idx;

    idx = line.find("line");
    if(idx != string::npos)
    {
	line.erase(0, idx + 4);
    }

    idx = line.find_first_not_of(" \t\r#");
    if(idx != string::npos)
    {
	line.erase(0, idx);
    }

    _currentLine = atoi(line.c_str()) - 1;

    idx = line.find_first_of(" \t\r");
    if(idx != string::npos)
    {
	line.erase(0, idx);
    }

    idx = line.find_first_not_of(" \t\r\"");
    if(idx != string::npos)
    {
	line.erase(0, idx);

	idx = line.find_first_of(" \t\r\"");
	if(idx != string::npos)
	{
	    _currentFile = line.substr(0, idx);
	    line.erase(0, idx + 1);
	}
	else
	{
	    _currentFile = line;
	}
    }
}

void
Parser::error(const char* s)
{
    if(_commands.empty() && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ": " << s << endl;
    }
    else
    {
	cerr << "error: " << s << endl;
    }
    _errors++;
}

void
Parser::error(const string& s)
{
    error(s.c_str());
}

void
Parser::warning(const char* s)
{
    if(_commands.empty() && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ": warning: " << s << endl;
    }
    else
    {
	cerr << "warning: " << s << endl;
    }
}

void
Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
Parser::parse(FILE* file, bool debug)
{
    yydebug = debug ? 1 : 0;

    assert(!parser);
    parser = this;

    _errors = 0;
    _commands.empty();
    yyin = file;
    assert(yyin);

    _currentFile = "";
    _currentLine = 0;
    _continue = false;
    nextLine();

    int status = yyparse();
    if(_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

int
Parser::parse(const std::string& commands, bool debug)
{
    yydebug = debug ? 1 : 0;

    assert(!parser);
    parser = this;

    _errors = 0;
    _commands = commands;
    assert(!_commands.empty());
    yyin = 0;

    _currentFile = "";
    _currentLine = 0;
    _continue = false;
    nextLine();

    int status = yyparse();
    if(_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

Parser::Parser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query) :
    _communicator(communicator),
    _admin(admin),
    _query(query)
{
}
