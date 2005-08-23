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
	"application diff DESC [TARGET ... ] [NAME=VALUE ... ]\n"
        "                            Print the differences betwen the application\n"
        "                            described in DESC and the current deployment.\n"
	"application update DESC [TARGET ... ] [NAME=VALUE ... ]\n"
	"                            Update the application described in DESC.\n"
	"application patch NAME [PATCHID]\n"
	"                            Patch the given application data.\n"
	"application list            List all deployed applications.\n"
	"                            to the application."
        "\n"
	"server template instantiate APPLICATION NODE TEMPLATE [NAME=VALUE ...]\n"
	"                            Instantiate a server template\n"
	"server template describe APPLICATION TEMPLATE\n"
        "                            Describe application server template TEMPLATE.\n"
	"\n"
	"service template describe APPLICATION TEMPLATE\n"
        "                            Describe application service template TEMPLATE.\n"
	"\n"
	"node list                   List all registered nodes.\n"
	"node ping NAME              Ping node NAME.\n"
        "node remove NAME            Remove the servers deployed on node NAME and\n"
        "                            the node NAME.\n"
	"node shutdown NAME          Shutdown node NAME.\n"
	"\n"
        "server list                 List all registered servers.\n"
        "server remove ID            Remove server ID.\n"
        "server describe ID          Describe server ID.\n"
	"server state ID             Get server ID state.\n"
	"server pid ID               Get server ID pid.\n"
	"server start ID             Start server ID.\n"
	"server stop ID              Stop server ID.\n"
	"server patch ID             Patch server ID.\n"
        "server signal ID SIGNAL     Send SIGNAL (e.g. SIGTERM or 15) to server ID.\n"
        "server stdout ID MESSAGE    Write MESSAGE on server ID's stdout.\n"
	"server stderr ID MESSAGE    Write MESSAGE on server ID's stderr.\n"
	"server activation ID [on-demand | manual] \n"
	"                            Set the server activation mode to on-demand or\n"
	"                            manual."
	"\n"
        "adapter list                List all registered adapters.\n"
	"adapter endpoints ID [SERVERID]\n"
        "                            Get endpoints of adapter ID from server SERVERID.\n"
	"                            If SERVERID is not specified, get the endpoints of all\n"
        "                            the registered replicas.\n"
	"adapter remove ID [SERVERID]\n"
        "                            Remove the endpoints of adapter ID from server SERVERID.\n"
	"                            If SERVERID is not specified, remove the endpoints of all\n"
        "                            the registered replicas.\n"
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
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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

	Output out(cout);
	ApplicationHelper helper(_admin->getApplicationDescriptor(name));
	helper.print(out);
	out << nl;
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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

	ApplicationDescriptor newApp = DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator);
	ApplicationDescriptor origApp = _admin->getApplicationDescriptor(newApp.name);

	ApplicationHelper newAppHelper(newApp);
	ApplicationHelper oldAppHelper(origApp);
	
	Output out(cout);
	newAppHelper.printDiff(out, oldAppHelper);
	out << nl;  
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
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

	_admin->syncApplication(DescriptorParser::parseDescriptor(descriptor, targets, vars, _communicator));
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::patchApplication(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`application patch' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string patch;
	if(p != args.end())
	{
	    patch = *p++;
	}

	_admin->patchApplication(name, patch);
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
    }
}

void
Parser::describeServerTemplate(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`server template describe' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string templ = *p++;

	ApplicationDescriptor application = _admin->getApplicationDescriptor(name);
	
	Output out(cout);
	TemplateDescriptorDict::const_iterator q = application.serverTemplates.find(templ);
	if(q != application.serverTemplates.end())
	{
	    out << "server template '" << templ << "'";
	    out << sb;
	    if(!q->second.parameters.empty())
	    {
		out << nl << "parameters = '" << toString(q->second.parameters) << "'";
	    }
	    out << nl;
	    ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(q->second.descriptor);
	    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server);
	    if(iceBox)
	    {
		IceBoxHelper(iceBox).print(out);
	    }
	    else
	    {
		ServerHelper(server).print(out);
	    }
	    out << eb;
	    out << nl;
	}
	else
	{
	    error("no server template with id `" + templ + "'");
	}
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::instantiateServerTemplate(const list<string>& args)
{
    if(args.size() < 3)
    {
	error("`server template instantiate' requires at least three arguments\n(`help' for more info)");
	return;
    }

    try
    {
	map<string, string> vars;

	list<string>::const_iterator p = args.begin();
	string application = *p++;
	string node = *p++;
	string templ = *p++;

	for(; p != args.end(); ++p)
	{
	    string::size_type pos = p->find('=');
	    if(pos != string::npos)
	    {
		vars[p->substr(0, pos)] = p->substr(pos + 1);
	    }
	}

	ServerInstanceDescriptor desc;
	desc._cpp_template = templ;
	desc.parameterValues = vars;
	NodeUpdateDescriptor nodeUpdate;
	nodeUpdate.name = node;
	nodeUpdate.serverInstances.push_back(desc);
	ApplicationUpdateDescriptor update;
	update.nodes.push_back(nodeUpdate);
	_admin->updateApplication(update);
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::describeServiceTemplate(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`service template describe' requires exactly two arguments\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string templ = *p++;

	ApplicationDescriptor application = _admin->getApplicationDescriptor(name);
	
	Output out(cout);
	TemplateDescriptorDict::const_iterator q = application.serviceTemplates.find(templ);
	if(q != application.serviceTemplates.end())
	{
	    out << "service template '" << templ << "'";
	    out << sb;
	    if(!q->second.parameters.empty())
	    {
		out << nl << "parameters = '" << toString(q->second.parameters) << "'";
	    }
	    out << nl;
	    ServiceDescriptorPtr service = ServiceDescriptorPtr::dynamicCast(q->second.descriptor);
	    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(service);
	    if(iceBox)
	    {
		IceBoxHelper(iceBox).print(out);
	    }
	    else
	    {
		ServiceHelper(service).print(out);
	    }
	    out << eb;
	    out << nl;
	}
	else
	{
	    error("no service template with id `" + templ + "'");
	}
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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
	exception(ex);
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
	exception(ex);
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
	exception(ex);
    }
}

void
Parser::removeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server start' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	ServerInfo info = _admin->getServerInfo(args.front());
	NodeUpdateDescriptor nodeUpdate;
	nodeUpdate.name = info.node;
	nodeUpdate.removeServers.push_back(args.front());
	ApplicationUpdateDescriptor update;
	update.name = info.application;
	update.nodes.push_back(nodeUpdate);
	_admin->updateApplication(update);
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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
	exception(ex);
    }
}

void
Parser::patchServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server patch' requires exactly one argument\n(`help' for more info)");
	return;
    }

    try
    {
	_admin->patchServer(args.front());
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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
	exception(ex);
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
	ServerInfo info = _admin->getServerInfo(args.front());
	Output out(cout);
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
	if(iceBox)
	{
	    IceBoxHelper(iceBox).print(out, info.application, info.node);
	}
	else
	{
	    ServerHelper(info.descriptor).print(out, info.application, info.node);
	}
	out << nl;
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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
	exception(ex);
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
	exception(ex);
    }
}

void
Parser::listAllServers()
{
    try
    {
	Ice::StringSeq ids = _admin->getAllServerIds();
	copy(ids.begin(), ids.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::endpointsAdapter(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`adapter endpoints' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();
	string adapterId = *p++;
	StringObjectProxyDict proxies = _admin->getAdapterEndpoints(adapterId);
	if(args.size() > 1)
	{
	    string serverId = *p++;
	    if(proxies.find(serverId) == proxies.end())
	    {
		throw ServerNotExistException();
	    }

	    string endpoints = _communicator->proxyToString(proxies[serverId]);
	    cout << (endpoints.empty() ? "<inactive>" : endpoints) << endl;
	}
	else
	{
	    for(StringObjectProxyDict::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
	    {
		cout << (p->first.empty() ? "<empty>" : p->first) << ": ";
		string endpoints = _communicator->proxyToString(p->second);
		cout << (endpoints.empty() ? "<inactive>" : endpoints) << endl;
	    }
	}
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::removeAdapter(const list<string>& args)
{
    if(args.size() < 1)
    {
	error("`adapter remove' requires at least one argument\n(`help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();
	string adapterId = *p++;
	StringObjectProxyDict proxies = _admin->getAdapterEndpoints(adapterId);
	if(args.size() > 1)
	{
	    string serverId = *p++;
	    _admin->removeAdapterWithServerId(adapterId, serverId);
	}
	else
	{
	    _admin->removeAdapter(adapterId);
	}
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::listAllAdapters()
{
    try
    {
	Ice::StringSeq ids = _admin->getAllAdapterIds();
	copy(ids.begin(), ids.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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
	exception(ex);
    }
}

void
Parser::describeObject(const list<string>& args)
{
    try
    {
	ObjectInfoSeq objects;
	if(args.size() == 1)
	{
	    string arg = *(args.begin());
	    if(arg.find('*') == string::npos)
	    {
		ObjectInfo info = _admin->getObjectInfo(Ice::stringToIdentity(arg));
		cout << "proxy = '" << _communicator->proxyToString(info.proxy) << "'" << endl;
		cout << "type = '" << info.type << "'" << endl;
		return;
	    }
	    else
	    {
		objects = _admin->getAllObjectInfos(arg);
	    }
	}
	else
	{
	    objects = _admin->getAllObjectInfos("");
	}
	
	for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    cout << "proxy = `" << _communicator->proxyToString(p->proxy) << "' type = `" << p->type << "'" << endl;
	}
	
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
    }
}

void
Parser::listObject(const list<string>& args)
{
    try
    {
	ObjectInfoSeq objects;
	if(args.size() == 1)
	{
	    objects = _admin->getAllObjectInfos(*(args.begin()));
	}
	else
	{
	    objects = _admin->getAllObjectInfos("");
	}
	
	for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    cout << Ice::identityToString(p->proxy->ice_getIdentity()) << endl;
	}	
    }
    catch(const Ice::Exception& ex)
    {
	exception(ex);
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
	exception(ex);
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

void
Parser::exception(const Ice::Exception& ex)
{
    try
    {
	ex.ice_throw();
    }
    catch(const ApplicationNotExistException& ex)
    {
	error("couldn't find application `" + ex.name + "'");
    }
    catch(const NodeNotExistException& ex)
    {
	error("couldn't find node `" + ex.name + "'");
    }
    catch(const ServerNotExistException& ex)
    {
	error("couldn't find server `" + ex.id + "'");
    }
    catch(const AdapterNotExistException& ex)
    {
	error("couldn't find adapter `" + ex.id + "'");
    }
    catch(const ObjectExistsException& ex)
    {
	error("object `" + Ice::identityToString(ex.id) + "' already exists");
    }
    catch(const ObjectNotExistException& ex)
    {
	error("couldn't find object `" + Ice::identityToString(ex.id) + "'");
    }
    catch(const DeploymentException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());	
    }
    catch(const PatchException& ex)
    {
	ostringstream s;
	s << ex << ":\n" << ex.reason;
	error(s.str());
    }
    catch(const BadSignalException& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
    catch(const NodeUnreachableException& ex)
    {
	error("node `" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch(const IceXML::ParserException& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
    catch(const Ice::LocalException& ex)
    {
	ostringstream s;
	s << "couldn't reach the IceGrid registry:\n" << ex;
	throw s.str();
    }
    catch(const Ice::Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}
