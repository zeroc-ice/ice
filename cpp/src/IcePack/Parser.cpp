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
#include <IcePack/Parser.h>
#include <IcePack/DescriptorVisitor.h>
#include <IcePack/DescriptorParser.h>
#include <IcePack/DescriptorUtil.h>

#ifdef GPL_BUILD
#   include <IcePack/GPL.h>
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
using namespace IcePack;

namespace IcePack
{

Parser* parser;

class ServerDescribe : public DescriptorVisitor
{
public:

    ServerDescribe(IceUtil::Output&, const Ice::CommunicatorPtr&, AdminPrx&);

    void describe(const ServerDescriptorPtr&);

private:

    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual void visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual bool visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr&);
    virtual void visitServiceEnd(const ServiceWrapper&, const ServiceDescriptorPtr&);
    virtual void visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor&);
    virtual bool visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor&);
    virtual void visitAdapterEnd(const AdapterWrapper&, const AdapterDescriptor&);
    virtual void visitObject(const ObjectWrapper&, const ObjectDescriptor&);

    IceUtil::Output& _out;
    Ice::CommunicatorPtr _communicator;
    AdminPrx _admin;
};

}

ServerDescribe::ServerDescribe(IceUtil::Output& out, const Ice::CommunicatorPtr& communicator, AdminPrx& admin) : 
    _out(out),
    _communicator(communicator),
    _admin(admin)
{
}

void
ServerDescribe::describe(const ServerDescriptorPtr& desc)
{
    ServerWrapper(desc).visit(*this);
}

bool
ServerDescribe::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    _out << "server '" << server->name << "' ";
    if(JavaServerDescriptorPtr::dynamicCast(server))
    {
	if(JavaIceBoxDescriptorPtr::dynamicCast(server))
	{
	    _out << " (Java IceBox)";
	}
	else
	{
	    _out << " (Java)";
	}
    }
    else if(CppIceBoxDescriptorPtr::dynamicCast(server))
    {
	_out << " (IceBox)";
    }

    _out << sb;
    _out << nl << "node = '" << server->node << "'";
    _out << nl << "application = '" << server->application << "'";
    _out << nl << "exe = '" << server->exe << "'";
    if(!server->pwd.empty())
    {
	_out << nl << "pwd = '" << server->pwd << "'";
    }
    _out << nl << "activation = '" << (_admin->getServerActivation(server->name) == OnDemand ? "on-demand" : "manual")
	 << "'";
    if(JavaServerDescriptorPtr::dynamicCast(server))
    {
	JavaServerDescriptorPtr s = JavaServerDescriptorPtr::dynamicCast(server);
	_out << nl << "className = '" << s->className << "'";
	if(!s->jvmOptions.empty())
	{
	    _out << nl << "jvmOptions = '";
	    for(Ice::StringSeq::const_iterator p = s->jvmOptions.begin(); p != s->jvmOptions.end();)
	    {
		_out << *p << ((++p != s->jvmOptions.end()) ? " " : "'");
	    }
	}
    }
    if(JavaIceBoxDescriptorPtr::dynamicCast(server))
    {
	JavaIceBoxDescriptorPtr s = JavaIceBoxDescriptorPtr::dynamicCast(server);
	_out << nl << "service manager endpoints = '" << s->endpoints << "'";
    }
    if(CppIceBoxDescriptorPtr::dynamicCast(server))
    {
	CppIceBoxDescriptorPtr s = CppIceBoxDescriptorPtr::dynamicCast(server);
	_out << nl << "service manager endpoints = '" << s->endpoints << "'";
    }
    if(!server->comment.empty())
    {
	_out << nl << "comment";
	_out << sb;
	_out << nl << server->comment;
	_out << eb;
    }
    if(!server->options.empty())
    {
	_out << nl << "options = '";
	for(Ice::StringSeq::const_iterator p = server->options.begin(); p != server->options.end();)
	{
	    _out << *p << ((++p != server->options.end()) ? " " : "'");
	}
    }
    if(!server->envs.empty())
    {
	_out << nl << "envs = '";
	for(Ice::StringSeq::const_iterator p = server->envs.begin(); p != server->envs.end(); ++p)
	{
	    _out << *p << " ";
	}
    }
    if(!server->properties.empty())
    {
	_out << nl << "properties";
	_out << sb;
	for(PropertyDescriptorSeq::const_iterator p = server->properties.begin(); p != server->properties.end(); ++p)
	{
	    _out << nl << p->name << "=" << p->value;
	}
	_out << eb;
    }

    return true;
}

void
ServerDescribe::visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    _out << eb;
    _out << nl;
}

bool
ServerDescribe::visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr& service)
{
    _out << nl << "service '" << service->name << "'";
    _out << sb;
    _out << nl << "entry = '" << service->entry << "'";
    if(!service->comment.empty())
    {
	_out << nl << "comment";
	_out << sb;
	_out << nl << service->comment;
	_out << eb;
    }
    if(!service->properties.empty())
    {
	_out << nl << "properties";
	_out << sb;
	for(PropertyDescriptorSeq::const_iterator p = service->properties.begin(); p != service->properties.end(); ++p)
	{
	    _out << nl << p->name << "=" << p->value;
	}
	_out << eb;
    }
    return true;
}

void
ServerDescribe::visitServiceEnd(const ServiceWrapper&, const ServiceDescriptorPtr& service)
{
    _out << eb;
}

void 
ServerDescribe::visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor& dbEnv)
{
    _out << nl << "database environment '" << dbEnv.name << "'";
    if(!dbEnv.dbHome.empty() || !dbEnv.properties.empty())
    {
	_out << sb;
	if(!dbEnv.dbHome.empty())
	{
	    _out << nl << "home = '" << dbEnv.dbHome << "'";
	}
	if(!dbEnv.properties.empty())
	{
	    _out << nl << "properties";
	    _out << sb;
	    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
	    {
	    _out << nl << p->name << "=" << p->value;
	    }
	    _out << eb;
	}
	_out << eb;
    }
}

bool 
ServerDescribe::visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor& adapter)
{
    _out << nl << "adapter '" << adapter.name << "'";
    _out << sb;
    _out << nl << "id = '" << adapter.id << "'";
    _out << nl << "endpoints = '" << adapter.endpoints << "'";
    _out << nl << "register process = '" << (adapter.registerProcess ? "true" : "false") << "'";
    return true;
}

void
ServerDescribe::visitAdapterEnd(const AdapterWrapper&, const AdapterDescriptor& adapter)
{
    _out << eb;
}

void 
ServerDescribe::visitObject(const ObjectWrapper&, const ObjectDescriptor& object)
{
    _out << nl << "object";
    if(!object.type.empty())
    {
	_out << sb;
	_out << nl << "proxy = '" << _communicator->proxyToString(object.proxy) << "' ";
	_out << nl << "type = '" << object.type << "'";
	_out << eb;
    }
}

ParserPtr
IcePack::Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query)
{
    return new Parser(communicator, admin, query);
}

void
IcePack::Parser::usage()
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
	"application diff DESC [TARGET ... ] [NAME=VALUE ... ]\n"
        "                            Print the differences betwen the application\n"
        "                            described in DESC and the current deployment.\n"
	"application update DESC [TARGET ... ] [NAME=VALUE ... ]\n"
	"                            Update the application described in DESC.\n"
	"application list            List all deployed applications.\n"
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
        "shutdown                    Shut the IcePack registry down.\n"
#ifdef GPL_BUILD
	"show copying                Show conditions for redistributing copies of this\n"
	"                            program.\n"
	"show warranty               Show the warranty for this program.\n"
#endif
	;
}

void
IcePack::Parser::addApplication(const list<string>& args)
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

	_admin->addApplication(DescriptorParser::parseApplicationDescriptor(descriptor, targets, vars, _communicator));
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
IcePack::Parser::removeApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application remove' requires at exactly one argument\n(`help' for more info)");
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
IcePack::Parser::describeApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application describe' requires at exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;

	ApplicationDescriptorPtr application = _admin->getApplicationDescriptor(name);
	
	IceUtil::Output out(cout);
	out << "application '" << application->name << "'";
	out << sb;
	if(!application->comment.empty())
	{
	    out << nl << "comment = " << application->comment;
	}
	if(!application->servers.empty())
	{
	    map<string, set<string> > servers;
	    {
		for(ServerDescriptorSeq::const_iterator p = application->servers.begin(); 
		    p != application->servers.end();
		    ++p)
		{
		    map<string, set<string> >::iterator q = servers.find((*p)->node);
		    if(q == servers.end())
		    {
		    q = servers.insert(make_pair((*p)->node, set<string>())).first;
		    }
		    q->second.insert((*p)->name);
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
IcePack::Parser::diffApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application diff' requires at exactly one argument\n(`help' for more info)");
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

	newApp = DescriptorParser::parseApplicationDescriptor(descriptor, targets, vars, _communicator);
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

    ServerDescriptorSeq::const_iterator p;
    for(p = newApp->servers.begin(); p != newApp->servers.end(); ++p)
    {
	ServerDescriptorPtr orig;
	for(ServerDescriptorSeq::const_iterator q = origApp->servers.begin(); q != origApp->servers.end(); ++q)
	{
	    if((*q)->name == (*p)->name)
	    {
		orig = *q;
		break;
	    }
	}

	if(orig)
	{
	    if(!equal(orig, *p))
	    {
		out << nl << "server `" << orig->name << "' updated";
	    }
	}
	else
	{
	    out << nl << "server `" << (*p)->name << "' added";
	}
    }

    for(p = origApp->servers.begin(); p != origApp->servers.end(); ++p)
    {
	bool found = false;
	for(ServerDescriptorSeq::const_iterator q = newApp->servers.begin(); q != newApp->servers.end(); ++q)
	{
	    if((*q)->name == (*p)->name)
	    {
		found = true;
		break;
	    }
	}
	
	if(!found)
	{
	    out << nl << "server `" << (*p)->name << "' removed";
	}
    }

    out << eb;
    out << nl;
}

void
IcePack::Parser::updateApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application diff' requires at exactly one argument\n(`help' for more info)");
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

	_admin->updateApplication(
	    DescriptorParser::parseApplicationDescriptor(descriptor, targets, vars, _communicator));
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
IcePack::Parser::listAllApplications()
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
IcePack::Parser::pingNode(const list<string>& args)
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
IcePack::Parser::shutdownNode(const list<string>& args)
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
IcePack::Parser::removeNode(const list<string>& args)
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
IcePack::Parser::listAllNodes()
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
IcePack::Parser::addServer(const list<string>& args)
{
    if(args.size() < 3)
    {
	error("`server add' requires at least three arguments\n(`help' for more info)");
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

	_admin->addServer(DescriptorParser::parseServerDescriptor(descriptor, targets, vars, _communicator));
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
IcePack::Parser::updateServer(const list<string>& args)
{
    if(args.size() < 3)
    {
	error("`server add' requires at least three arguments\n(`help' for more info)");
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

	_admin->updateServer(DescriptorParser::parseServerDescriptor(descriptor, targets, vars, _communicator));
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
IcePack::Parser::startServer(const list<string>& args)
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
IcePack::Parser::stopServer(const list<string>& args)
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
IcePack::Parser::signalServer(const list<string>& args)
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
IcePack::Parser::writeMessage(const list<string>& args, int fd)
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
IcePack::Parser::describeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server describe' requires exactly one argument\n(`help' for more info)");
	return;
    }
    
    try
    {
	ServerDescriptorPtr desc = _admin->getServerDescriptor(args.front());

	IceUtil::Output out(cout);
	ServerDescribe(out, _communicator, _admin).describe(desc);
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
IcePack::Parser::removeServer(const list<string>& args)
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
IcePack::Parser::stateServer(const list<string>& args)
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
IcePack::Parser::pidServer(const list<string>& args)
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
IcePack::Parser::activationServer(const list<string>& args)
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
IcePack::Parser::listAllServers()
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
IcePack::Parser::endpointsAdapter(const list<string>& args)
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
IcePack::Parser::listAllAdapters()
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
IcePack::Parser::addObject(const list<string>& args)
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
IcePack::Parser::removeObject(const list<string>& args)
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
IcePack::Parser::findObject(const list<string>& args)
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
IcePack::Parser::describeObject(const list<string>& args)
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
IcePack::Parser::listObject(const list<string>& args)
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
IcePack::Parser::shutdown()
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
IcePack::Parser::showBanner()
{
    cout << "Ice " << ICE_STRING_VERSION << "  Copyright 2003-2004 ZeroC, Inc." << endl;
#ifdef GPL_BUILD
    cout << gplBanner << endl;
#endif
}

void
IcePack::Parser::showCopying()
{
#if defined(GPL_BUILD)
    cout << gplCopying << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
}

void
IcePack::Parser::showWarranty()
{
#if defined(GPL_BUILD)
    cout << gplWarranty << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
}


void
IcePack::Parser::getInput(char* buf, int& result, int maxSize)
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
IcePack::Parser::nextLine()
{
    _currentLine++;
}

void
IcePack::Parser::continueLine()
{
    _continue = true;
}

const char*
IcePack::Parser::getPrompt()
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
IcePack::Parser::scanPosition(const char* s)
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
IcePack::Parser::error(const char* s)
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
IcePack::Parser::error(const string& s)
{
    error(s.c_str());
}

void
IcePack::Parser::warning(const char* s)
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
IcePack::Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
IcePack::Parser::parse(FILE* file, bool debug)
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
IcePack::Parser::parse(const std::string& commands, bool debug)
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

IcePack::Parser::Parser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query) :
    _communicator(communicator),
    _admin(admin),
    _query(query)
{
}
