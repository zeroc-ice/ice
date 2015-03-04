// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IceXML/Parser.h>
#include <IceGrid/Parser.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceBox/IceBox.h>

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
using namespace IceUtilInternal;
using namespace Ice;
using namespace IceGrid;

namespace
{

const char* _commandsHelp[][3] = {
{ "application", "add", 
"application add [-n | --no-patch] DESC [TARGET ... ] [NAME=VALUE ... ]\n"
"                          Add application described in DESC. If specified\n"
"                          the optional targets TARGET will be deployed.\n" 
},
{ "application", "remove", 
"application remove NAME   Remove application NAME.\n" 
},
{ "application", "describe",
"application describe NAME Describe application NAME.\n" 
},
{ "application", "diff",
"application diff [-s | --servers] DESC [TARGET ... ] [NAME=VALUE ... ]\n"
"                          Print the differences betwen the application\n"
"                          described in DESC and the current deployment.\n" 
"                          If -s or --servers is specified, print the\n"
"                          the list of servers affected by the differences.\n" 
},
{ "application", "update",
"application update [-n | --no-restart] DESC [TARGET ... ] [NAME=VALUE ... ]\n"
"                          Update the application described in DESC. If -n or\n"
"                          --no-restart is specified, the update will fail if\n"
"                          it is necessary to stop some servers.\n"
},
{ "application", "patch",
"application patch [-f | --force] NAME\n"
"                          Patch the given application data. If -f or --force is\n"
"                          specified, the servers depending on the data to patch\n"
"                          will be stopped if necessary.\n"
},
{ "application", "list",
"application list          List all deployed applications.\n" 
},
{ "server template", "instantiate", 
"server template instantiate APPLICATION NODE TEMPLATE [NAME=VALUE ...]\n"
"                          Instantiate a server template.\n" 
},
{ "server template", "describe", 
"server template describe APPLICATION TEMPLATE\n" 
"                          Describe application server template TEMPLATE.\n" 
},
{ "service template", "describe",
"service template describe APPLICATION TEMPLATE\n" 
"                          Describe application service template TEMPLATE.\n" 
},
{ "node", "list",
"node list                 List all registered nodes.\n" 
},
{ "node", "describe", 
"node describe NAME        Show information about node NAME.\n" 
},
{ "node", "ping",
"node ping NAME            Ping node NAME.\n" 
},
{ "node", "load",
"node load NAME            Print the load of the node NAME.\n" 
},
{ "node", "sockets",
"node sockets [NAME]       Print the number of CPU sockets of the\n"
"                          node NAME or all the nodes if NAME is omitted.\n" 
},
{ "node", "show",
"node show [OPTIONS] NAME [stderr | stdout]\n"
"                          Show node NAME stderr or stdout.\n"
"                          Options:\n"
"                           -f | --follow: Wait for new data to be available\n"
"                           -t N | --tail N: Print the last N lines\n"
"                           -h N | --head N: Print the first N lines\n"
},
{ "node", "shutdown",
"node shutdown NAME        Shutdown node NAME.\n" 
},
{ "registry", "list",
"registry list             List all registered registries.\n" 
},
{ "registry", "describe",
"registry describe NAME    Show information about registry NAME.\n" 
},
{ "registry", "ping",
"registry ping NAME        Ping registry NAME.\n" 
},
{ "registry", "show",
"registry show [OPTIONS] NAME [stderr | stdout]\n" 
"                          Show registry NAME stderr or stdout.\n" 
"                          Options:\n"
"                           -f | --follow: Wait for new data to be available\n"
"                           -t N | --tail N: Print the last N lines\n"
"                           -h N | --head N: Print the first N lines\n"
},
{ "registry", "shutdown",
"registry shutdown NAME    Shutdown registry NAME.\n" 
},
{ "server", "list",
"server list               List all registered servers.\n" 
},
{ "server", "remove",
"server remove ID          Remove server ID.\n" 
},
{ "server", "describe", 
"server describe ID        Describe server ID.\n" 
},
{ "server", "properties",
"server properties ID      Get the run-time properties of server ID.\n"
},
{ "server", "property",
"server property ID NAME   Get the run-time property NAME of server ID.\n"
},
{ "server", "state",
"server state ID           Get the state of server ID.\n" 
},
{ "server", "pid",
"server pid ID             Get the process id of server ID.\n" 
},
{ "server", "start",
"server start ID           Start server ID.\n" 
},
{ "server", "stop",
"server stop ID            Stop server ID.\n" 
},
{ "server", "patch",
"server patch ID           Patch server ID.\n" 
},
{ "server", "signal", 
"server signal ID SIGNAL   Send SIGNAL (e.g. SIGTERM or 15) to server ID.\n" 
},
{ "server", "stdout",
"server stdout ID MESSAGE  Write MESSAGE on server ID's stdout.\n" 
},
{ "server", "stderr",
"server stderr ID MESSAGE  Write MESSAGE on server ID's stderr.\n" 
},
{ "server", "show",
"server show [OPTIONS] ID [stderr | stdout | LOGFILE ]\n"
"                          Show server ID stderr, stdout or log file LOGFILE.\n"
"                          Options:\n"
"                           -f | --follow: Wait for new data to be available\n"
"                           -t N | --tail N: Print the last N lines\n"
"                           -h N | --head N: Print the first N lines\n"
},
{ "server", "enable",
"server enable ID          Enable server ID.\n" 
},
{ "server", "disable",
"server disable ID         Disable server ID (a disabled server can't be\n"
"                          started on demand or administratively).\n" 
},

{ "service", "start",
"service start ID NAME     Starts service NAME in IceBox server ID.\n"
},
{ "service", "stop",
"service stop ID NAME      Stops service NAME in IceBox server ID.\n"
},
{ "service", "describe",
"service describe ID NAME  Describes service NAME in IceBox server ID.\n"
},
{ "service", "properties",
"service properties ID NAME\n"
"                          Get the run-time properties of service NAME in\n"
"                          IceBox server ID.\n"
},
{ "service", "property",
"service property ID NAME PROPERTY\n"
"                          Get the run-time property PROPERTY of service NAME\n"
"                          from IceBox server ID.\n"
},
{ "service", "list",
"service list ID           List the services in IceBox server ID.\n"
},

{ "adapter", "list",
"adapter list              List all registered adapters.\n" 
},
{ "adapter", "endpoints",
"adapter endpoints ID      Show the endpoints of adapter or replica group ID.\n" 
},
{ "adapter", "remove",
"adapter remove ID         Remove adapter or replica group ID.\n" 
},
{ "object", "add",
"object add PROXY [TYPE]   Add an object to the object registry,\n"
"                          optionally specifying its type.\n" 
},
{ "object", "remove",
"object remove IDENTITY    Remove an object from the object registry.\n" 
},
{ "object", "find",
"object find TYPE          Find all objects with the type TYPE.\n" 
},
{ "object", "describe",
"object describe EXPR      Describe all registered objects whose stringified\n" 
"                          identities match the expression EXPR. A trailing\n"
"                          wildcard is supported in EXPR, for example\n"
"                          \"object describe Ice*\".\n"
},
{ "object", "list",
"object list EXPR          List all registered objects whose stringified\n"
"                          identities match the expression EXPR. A trailing\n"
"                          wildcard is supported in EXPR, for example\n"
"                          \"object list Ice*\".\n" 
},
{ 0, 0, 0 }
};

}

namespace IceGrid
{

Parser* parser;

}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const AdminSessionPrx& session, const AdminPrx& admin,
                     bool interactive)
{
    return new Parser(communicator, session, admin, interactive);
}

void 
Parser::usage(const string& category, const string& command)
{
    if(_helpCommands.find(category) == _helpCommands.end())
    {
        invalidCommand("unknown command `" + category + "'");
    }
    else if(_helpCommands[category].find(command) == _helpCommands[category].end())
    {
        invalidCommand("unknown command `" + category + " " + command + "'");
    }
    else
    {
        cout << _helpCommands[category][command];
    }
}

void
Parser::usage(const string& category, const list<string>& args)
{
    if(args.empty())
    {
        usage(category);
    }
    else if(args.size() > 1)
    {
        invalidCommand("`help' requires at most 1 argument");
    }
    else 
    {
        usage(category, *args.begin());
    }
}

void
Parser::usage()
{
     cout <<
         "help                        Print this message.\n"
         "exit, quit                  Exit this program.\n"
         "CATEGORY help               Print the help section of the given CATEGORY.\n"
         "COMMAND help                Print the help of the given COMMAND.\n"
         "\n"
         "List of help categories:\n"
         "\n"
         "  application: commands to manage applications\n" 
         "  node: commands to manage nodes\n" 
         "  registry: commands to manage registries\n" 
         "  server: commands to manage servers\n" 
         "  service: commands to manage services\n"
         "  adapter: commands to manage adapters\n" 
         "  object: commands to manage objects\n"
         "  server template: commands to manage server templates\n" 
         "  service template: commands to manage service templates\n" 
         "\n";
}

void
Parser::interrupt()
{
    Lock sync(*this);
    _interrupted = true;
    notifyAll();
}

bool
Parser::interrupted() const
{
    Lock sync(*this);
    return _interrupted;
}

void
Parser::resetInterrupt()
{
    Lock sync(*this);
    _interrupted = false;
}

void
Parser::checkInterrupted()
{
    if(!_interactive)
    {
        Lock sync(*this);
        if(_interrupted)
        {
            throw "interrupted with Ctrl-C";
        }
    }
}

void
Parser::addApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");
    
    IceUtilInternal::Options opts;
    opts.addOpt("n", "no-patch");
    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() < 1)
    {
        invalidCommand("application add", "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        vector<string>::const_iterator p = args.begin();
        string desc = *p++;

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

        //
        // Add the application.
        //
        ApplicationDescriptor app = DescriptorParser::parseDescriptor(desc, targets, vars, _communicator, _admin);
        _admin->addApplication(app);
        
        if(!opts.isSet("no-patch"))
        {
            //
            // Patch the application.
            //
            try
            {
                _admin->patchApplication(app.name, true);
            }
            catch(const PatchException& ex)
            {
                warning(patchFailed(ex.reasons));
            }
        }
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
        invalidCommand("application remove", "requires exactly one argument");
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
        invalidCommand("application describe", "requires at least one argument");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();

        string name = *p++;

        Output out(cout);
        ApplicationInfo info = _admin->getApplicationInfo(name);
        ApplicationHelper helper(_communicator, info.descriptor);
        helper.print(out, info);
        out << nl;
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::diffApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");

    IceUtilInternal::Options opts;
    opts.addOpt("s", "servers");
    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() < 1)
    {
        invalidCommand("application diff" , "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        vector<string>::const_iterator p = args.begin();
        string desc = *p++;

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

        ApplicationDescriptor newApp = DescriptorParser::parseDescriptor(desc, targets, vars, _communicator, _admin);
        ApplicationInfo origApp = _admin->getApplicationInfo(newApp.name);

        ApplicationHelper newAppHelper(_communicator, newApp);
        ApplicationHelper oldAppHelper(_communicator, origApp.descriptor);
        
        Output out(cout);
        if(opts.isSet("servers"))
        {
            map<string, ServerInfo> oldServers = oldAppHelper.getServerInfos(origApp.uuid, origApp.revision);
            map<string, ServerInfo> newServers = newAppHelper.getServerInfos(origApp.uuid, origApp.revision);

            vector<string> messages;
            map<string, ServerInfo>::const_iterator p;
            for(p = oldServers.begin(); p != oldServers.end(); ++p)
            {
                map<string, ServerInfo>::const_iterator q = newServers.find(p->first);
                if(q == newServers.end())
                {
                    messages.push_back("server `" + p->first + "': removed");
                }
            }

            for(p = newServers.begin(); p != newServers.end(); ++p)
            {
                map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
                if(q == oldServers.end())
                {
                    messages.push_back("server `" + p->first + "': added");
                }
                else if(isServerUpdated(p->second, q->second))
                {
                    if(isServerUpdated(p->second, q->second, true)) // Ignore properties
                    {
                        messages.push_back("server `" + p->first + "': updated (restart required)");
                    }
                    else
                    {
                        messages.push_back("server `" + p->first + "': properties updated (no restart required)");
                    }
                }
            }

            out << "application `" << origApp.descriptor.name << "'";
            out << sb;
            sort(messages.begin(), messages.end());
            for(vector<string>::const_iterator r = messages.begin(); r != messages.end(); ++r)
            {
                out << nl << *r;
            }
            out << eb;
        }
        else
        {
            newAppHelper.printDiff(out, oldAppHelper);
        }
        out << nl;  
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::updateApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");

    IceUtilInternal::Options opts;
    opts.addOpt("n", "no-restart");
    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() < 1)
    {
        invalidCommand("application update", "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        vector<string>::const_iterator p = args.begin();
        string xml = *p++;

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

        ApplicationDescriptor desc = DescriptorParser::parseDescriptor(xml, targets, vars, _communicator, _admin);
        if(opts.isSet("no-restart"))
        {
            _admin->syncApplicationWithoutRestart(desc);
        }
        else
        {
            _admin->syncApplication(desc);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
        error("registry doesn't support updates without restart");
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::patchApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");
    
    IceUtilInternal::Options opts;
    opts.addOpt("f", "force");
    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() != 1)
    {
        invalidCommand("application patch", "requires exactly one argument");
        return;
    }

    try
    {
        vector<string>::const_iterator p = args.begin();
        string name = *p++;
        _admin->patchApplication(name, opts.isSet("force"));
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::listAllApplications(const list<string>& args)
{
    if(!args.empty())
    {
        invalidCommand("application list", "doesn't require any argument");
        return;
    }

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
        invalidCommand("server template describe", "requires exactly two arguments");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();

        string name = *p++;
        string templ = *p++;

        ApplicationInfo application = _admin->getApplicationInfo(name);
        
        Output out(cout);
        TemplateDescriptorDict::const_iterator q = application.descriptor.serverTemplates.find(templ);
        if(q != application.descriptor.serverTemplates.end())
        {
            out << "server template `" << templ << "'";
            out << sb;

            out << nl << "parameters = `" << toString(q->second.parameters) << "'";
            out << nl;

            ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(q->second.descriptor);
            IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server);
            if(iceBox)
            {
                IceBoxHelper(iceBox).print(_communicator, out);
            }
            else
            {
                ServerHelper(server).print(_communicator, out);
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
        invalidCommand("server template instantiate", "requires at least three arguments");
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
        _admin->instantiateServer(application, node, desc);
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
        invalidCommand("service template describe", "requires exactly two arguments");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();

        string name = *p++;
        string templ = *p++;

        ApplicationInfo application = _admin->getApplicationInfo(name);
        
        Output out(cout);
        TemplateDescriptorDict::const_iterator q = application.descriptor.serviceTemplates.find(templ);
        if(q != application.descriptor.serviceTemplates.end())
        {
            out << "service template `" << templ << "'";
            out << sb;

            out << nl << "parameters = `" << toString(q->second.parameters) << "'";
            out << nl;

            ServiceDescriptorPtr desc = ServiceDescriptorPtr::dynamicCast(q->second.descriptor);
            ServiceHelper(desc).print(_communicator, out);
            out << eb;
            out << nl;
        }
        else
        {
            invalidCommand("no service template with id `" + templ + "'");
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::describeNode(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("node describe", "requires exactly one argument");
        return;
    }

    try
    {
        NodeInfo info = _admin->getNodeInfo(args.front());
        Output out(cout);
        out << "node `" << args.front() << "'";
        out << sb;
        out << nl << "operating system = `" << info.os << "'";
        out << nl << "host name = `" << info.hostname << "'";
        out << nl << "release = `" << info.release << "'";
        out << nl << "version = `" << info.version << "'";
        out << nl << "machine type = `" << info.machine << "'";
        out << nl << "number of threads = `" << info.nProcessors << "'";
        out << eb;
        out << nl;
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
        invalidCommand("node ping", "requires exactly one argument");
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
Parser::printLoadNode(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("node load", "requires exactly one argument");
        return;
    }

    try
    {
        LoadInfo load = _admin->getNodeLoad(args.front());
        cout << "load average (1/5/15): " << load.avg1 << " / " << load.avg5 << " / " << load.avg15 << endl;
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::printNodeProcessorSockets(const list<string>& args)
{
    if(args.size() > 1)
    {
        invalidCommand("node sockets", "requires no more than one argument");
        return;
    }

    try
    {
        if(args.size() == 1)
        {
            try
            {
                cout << _admin->getNodeProcessorSocketCount(args.front()) << endl;
            }
            catch(const Ice::OperationNotExistException&)
            {
                cout << "not supported" << endl;
            }
        }
        else
        {
            Ice::StringSeq names = _admin->getAllNodeNames();
            map<string, pair< vector<string>, int> > processorSocketCounts;
            for(Ice::StringSeq::const_iterator p = names.begin(); p != names.end(); p++)
            {
                try
                {
                    NodeInfo info = _admin->getNodeInfo(*p);
                    processorSocketCounts[info.hostname].first.push_back(*p);
                    try
                    {
                        processorSocketCounts[info.hostname].second = _admin->getNodeProcessorSocketCount(*p);
                    }
                    catch(const Ice::OperationNotExistException&)
                    {
                        // Not supported.
                        processorSocketCounts[info.hostname].second = 0;
                    }
                }
                catch(const NodeNotExistException&)
                {
                }
                catch(const NodeUnreachableException&)
                {
                }
            }

            cout.flags(ios::left);
            cout << setw(20) << "Hostname" << setw(20) << "| # of sockets" << setw(39) << "| Nodes" << endl;
            cout << setw(79) << "=====================================================================" << endl;
            for(map<string, pair< vector<string>, int> >::const_iterator q = processorSocketCounts.begin();
                q != processorSocketCounts.end(); ++q)
            {
                cout << setw(20) << setiosflags(ios::left) <<q->first;
                cout << "| " << setw(18) << setiosflags(ios::left) << q->second.second;
                cout << "| " << setw(37) << setiosflags(ios::left) << toString(q->second.first);
                cout << endl;
            }
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
        invalidCommand("node shutdown", "requires exactly one argument");
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
Parser::listAllNodes(const list<string>& args)
{
    if(!args.empty())
    {
        invalidCommand("node list", "doesn't require any argument");
        return;
    }

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
Parser::describeRegistry(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("registry describe", "requires exactly one argument");
        return;
    }

    try
    {
        RegistryInfo info = _admin->getRegistryInfo(args.front());
        Output out(cout);
        out << "registry `" << args.front() << "'";
        out << sb;
        out << nl << "host name = `" << info.hostname << "'";
        out << eb;
        out << nl;
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::pingRegistry(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("registry ping", "requires exactly one argument");
        return;
    }

    try
    {
        if(_admin->pingRegistry(args.front()))
        {
            cout << "registry is up" << endl;
        }
        else
        {
            cout << "registry is down" << endl;
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::shutdownRegistry(const list<string>& args)
{
    if(args.size() > 1)
    {
        invalidCommand("registry shutdown", "requires at most one argument");
        return;
    }

    try
    {
        if(args.empty())
        {
            _admin->shutdown();
        }
        else
        {
            _admin->shutdownRegistry(args.front());
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::listAllRegistries(const list<string>& args)
{
    if(!args.empty())
    {
        invalidCommand("registry list", "doesn't require any argument");
        return;
    }

    try
    {
        Ice::StringSeq names = _admin->getAllRegistryNames();
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
        invalidCommand("server remove", "requires exactly one argument");
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
        invalidCommand("server start", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->startServer(args.front());
    }
    catch(const ServerStartException& ex)
    {
        error("the server didn't start successfully:\n" + ex.reason);
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
        invalidCommand("server stop", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->stopServer(args.front());
    }
    catch(const ServerStopException& ex)
    {
        error("the server didn't stop successfully:\n" + ex.reason);
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::patchServer(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");

    IceUtilInternal::Options opts;
    opts.addOpt("f", "force");
    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() != 1)
    {
        invalidCommand("server patch", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->patchServer(args.front(), opts.isSet("force"));
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
        invalidCommand("server signal", "requires exactly two arguments");
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
        invalidCommand("server stdout or server stderr", "requires exactly two arguments");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();
        string server = *p++;

        Ice::ObjectPrx serverAdmin = _admin->getServerAdmin(server);
        Ice::ProcessPrx process = Ice::ProcessPrx::uncheckedCast(serverAdmin, "Process");

        process->writeMessage(*p,  fd);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch(const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'Process' facet");
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
        invalidCommand("server describe", "requires exactly one argument");
        return;
    }
    
    try
    {
        ServerInfo info = _admin->getServerInfo(args.front());
        Output out(cout);
        IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
        if(iceBox)
        {
            IceBoxHelper(iceBox).print(_communicator, out, info);
        }
        else
        {
            ServerHelper(info.descriptor).print(_communicator, out, info);
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
        invalidCommand("server state", "requires exactly one argument");
        return;
    }

    try
    {
        ServerState state = _admin->getServerState(args.front());
        string enabled = _admin->isServerEnabled(args.front()) ? "enabled" : "disabled";
        switch(state)
        {
        case Inactive:
        {
            cout << "inactive (" << enabled << ")" << endl;
            break;
        }
        case Activating:
        {
            cout << "activating (" << enabled << ")" << endl;
            break;
        }
        case Active:
        {
            int pid = _admin->getServerPid(args.front());
            cout << "active (pid = " << pid << ", " << enabled << ")" << endl;
            break;
        }
        case ActivationTimedOut:
        {
            int pid = _admin->getServerPid(args.front());
            cout << "activation timed out (pid = " << pid << ", " << enabled << ")" << endl;
            break;
        }
        case Deactivating:
        {
            cout << "deactivating (" << enabled << ")" << endl;
            break;
        }
        case Destroying:
        {
            cout << "destroying (" << enabled << ")" << endl;
            break;
        }
        case Destroyed:
        {
            cout << "destroyed (" << enabled << ")" << endl;
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
        invalidCommand("server pid", "requires exactly one argument");
        return;
    }

    try
    {
        int pid = _admin->getServerPid(args.front());
        if(pid > 0)
        {
            cout << pid << endl;
        }
        else
        {
            error("server is not running"); 
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::propertiesServer(const list<string>& args, bool single)
{
    if(single && args.size() != 2)
    {
        invalidCommand("server property", "requires exactly two arguments");
        return;
    }
    else if(!single && args.size() != 1)
    {
        invalidCommand("server properties", "requires exactly one argument");
        return;
    }

    try
    {
        Ice::ObjectPrx serverAdmin = _admin->getServerAdmin(args.front());
        Ice::PropertiesAdminPrx propAdmin = Ice::PropertiesAdminPrx::uncheckedCast(serverAdmin, "Properties");

        if(single)
        {
            string val = propAdmin->getProperty(*(++args.begin()));
            cout << val << endl;
        }
        else
        {
            Ice::PropertyDict properties = propAdmin->getPropertiesForPrefix("");
            for(Ice::PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
            {
                cout << p->first << "=" << p->second << endl;
            }
        }
    }
    catch(const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch(const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'Properties' facet");
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::enableServer(const list<string>& args, bool enable)
{
    if(args.size() != 1)
    {
        if(enable)
        {
            invalidCommand("server enable", "requires exactly one argument");
        }
        else
        {
            invalidCommand("server disable", "requires exactly one argument");
        }
        return;
    }

    try
    {
        _admin->enableServer(args.front(), enable);
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::listAllServers(const list<string>& args)
{
    if(!args.empty())
    {
        invalidCommand("server list", "doesn't require any argument");
        return;
    }

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
Parser::startService(const list<string>& args)
{
    if(args.size() != 2)
    {
        invalidCommand("service start", "requires exactly two arguments");
        return;
    }

    string server = args.front();
    string service = *(++args.begin());
    try
    {
        Ice::ObjectPrx admin = _admin->getServerAdmin(server);
        IceBox::ServiceManagerPrx manager = IceBox::ServiceManagerPrx::uncheckedCast(admin, "IceBox.ServiceManager");
        manager->startService(service);
    }
    catch(const IceBox::AlreadyStartedException&)
    {
        error("the service `" + service + "' is already started");
    }
    catch(const IceBox::NoSuchServiceException&)
    {
        error("couldn't find service `" + service + "'");
    }
    catch(const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch(const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'IceBox.ServiceManager' facet");
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::stopService(const list<string>& args)
{
    if(args.size() != 2)
    {
        invalidCommand("service stop", "requires exactly two arguments");
        return;
    }

    string server = args.front();
    string service = *(++args.begin());
    try
    {
        Ice::ObjectPrx admin = _admin->getServerAdmin(server);
        IceBox::ServiceManagerPrx manager = IceBox::ServiceManagerPrx::uncheckedCast(admin, "IceBox.ServiceManager");
        manager->stopService(service);
    }
    catch(const IceBox::AlreadyStoppedException&)
    {
        error("the service `" + service + "' is already stopped");
    }
    catch(const IceBox::NoSuchServiceException&)
    {
        error("couldn't find service `" + service + "'");
    }
    catch(const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch(const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'IceBox.ServiceManager' facet");
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::describeService(const list<string>& args)
{
    if(args.size() != 2)
    {
        invalidCommand("service describe", "requires exactly two arguments");
        return;
    }
    
    string server = args.front();
    string service = *(++args.begin());
    try
    {
        ServerInfo info = _admin->getServerInfo(server);
        IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
        if(!iceBox)
        {
            error("server `" + server + "' is not an IceBox server");
            return;
        }

        Output out(cout);
        bool found = false;
        for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
        {
            if(p->descriptor && p->descriptor->name == service)
            {
                ServiceHelper(p->descriptor).print(_communicator, out);
                out << nl;
                found = true;
                break;
            }
        }
        
        if(!found)
        {
            error("couldn't find service `" + service + "'");
            return;
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::propertiesService(const list<string>& args, bool single)
{
    if(single && args.size() != 3)
    {
        invalidCommand("service property", "requires exactly three arguments");
        return;
    }
    else if(!single && args.size() != 2)
    {
        invalidCommand("service properties", "requires exactly two argument");
        return;
    }

    list<string>::const_iterator a = args.begin();
    string server = *a++;
    string service = *a++;
    string property = single ? *a++ : string();

    try
    {
        //
        // First, we ensure that the service exists.
        //
        ServerInfo info = _admin->getServerInfo(server);
        IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
        if(!iceBox)
        {
            error("server `" + server + "' is not an IceBox server");
            return;
        }

        bool found = false;
        for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
        {
            if(p->descriptor && p->descriptor->name == service)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            error("couldn't find service `" + service + "'");
            return;
        }

        Ice::ObjectPrx admin = _admin->getServerAdmin(server);
        Ice::PropertiesAdminPrx propAdmin = 
            Ice::PropertiesAdminPrx::uncheckedCast(admin, "IceBox.Service." + service + ".Properties");

        if(single)
        {
            string val = propAdmin->getProperty(property);
            cout << val << endl;
        }
        else
        {
            Ice::PropertyDict properties = propAdmin->getPropertiesForPrefix("");
            for(Ice::PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
            {
                cout << p->first << "=" << p->second << endl;
            }
        }
    }
    catch(const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch(const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide an 'IceBox.Service." + service + ".Properties' facet");
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::listServices(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("service list", "requires exactly one argument");
        return;
    }

    string server = args.front();
    try
    {
        ServerInfo info = _admin->getServerInfo(server);
        IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
        if(!iceBox)
        {
            error("server `" + server + "' is not an IceBox server");
            return;
        }
        for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
        {
            if(p->descriptor)
            {
                cout << p->descriptor->name << endl;
            }
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}


void
Parser::endpointsAdapter(const list<string>& args)
{
    if(args.size() != 1)
    {
        invalidCommand("adapter endpoints", "requires exactly one argument");
        return;
    }

    try
    {
        string adapterId = args.front();
        AdapterInfoSeq adpts = _admin->getAdapterInfo(adapterId);
        if(adpts.size() == 1 && adpts.begin()->id == adapterId)
        {
            string endpoints = _communicator->proxyToString(adpts.begin()->proxy);
            cout << (endpoints.empty() ? string("<inactive>") : endpoints) << endl;
        }
        else
        {
            for(AdapterInfoSeq::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
            {
                cout << (p->id.empty() ? string("<empty>") : p->id) << ": ";
                string endpoints = _communicator->proxyToString(p->proxy);
                cout << (endpoints.empty() ? string("<inactive>") : endpoints) << endl;
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
    if(args.size() != 1)
    {
        invalidCommand("adapter remove", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->removeAdapter(*args.begin());
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::listAllAdapters(const list<string>& args)
{
    if(!args.empty())
    {
        invalidCommand("adapter list", "doesn't require any argument");
        return;
    }

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
    if(args.size() != 1 && args.size() != 2)
    {
        invalidCommand("object add", "requires one or two arguments");
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
        invalidCommand("object remove", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->removeObject(_communicator->stringToIdentity((*(args.begin()))));
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
        invalidCommand("object find", "requires exactly one argument");
        return;
    }

    try
    {
        ObjectInfoSeq objects = _admin->getObjectInfosByType(*(args.begin()));
        for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
        {
            cout << _communicator->proxyToString(p->proxy) << endl;
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
    if(args.size() > 1)
    {
        invalidCommand("object describe", "requires at most one argument");
        return;
    }

    try
    {
        ObjectInfoSeq objects;
        if(args.size() == 1)
        {
            string arg = *(args.begin());
            if(arg.find('*') == string::npos)
            {
                ObjectInfo info = _admin->getObjectInfo(_communicator->stringToIdentity(arg));
                cout << "proxy = `" << _communicator->proxyToString(info.proxy) << "'" << endl;
                cout << "type = `" << info.type << "'" << endl;
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
    if(args.size() > 1)
    {
        invalidCommand("object list", "requires at most one argument");
        return;
    }

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
            cout << _communicator->identityToString(p->proxy->ice_getIdentity()) << endl;
        }       
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::showFile(const string& reader, const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.push_front("icegridadmin");
    
    IceUtilInternal::Options opts;
    opts.addOpt("f", "follow");
    opts.addOpt("h", "head", IceUtilInternal::Options::NeedArg);
    opts.addOpt("t", "tail", IceUtilInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        for(list<string>::const_iterator p = copyArgs.begin(); p != copyArgs.end(); ++p)
        {
            args.push_back(*p);
        }
        args = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        return;
    }

    if(args.size() != 2)
    {
        invalidCommand(reader + " show", "requires two arguments");
        return;
    }

    try
    {
        vector<string>::const_iterator p = args.begin();
        string id = *p++;
        string filename = *p++;

        cout << reader << " `" << id << "' " << filename << ": " << flush;
        Ice::StringSeq lines;

        bool head = opts.isSet("head");
        bool tail = opts.isSet("tail");
        if(head && tail)
        {
            invalidCommand("can't specify both -h | --head and -t | --tail options");
            return;
        }
        int lineCount = 20;
        int maxBytes = _communicator->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024) * 1024;
        if(head || tail)
        {
            if(head)
            {
                istringstream is(opts.optArg("head"));
                is >> lineCount;
            }
            else
            {
                istringstream is(opts.optArg("tail"));
                is >> lineCount;
            }
            if(lineCount <= 0)
            {
                invalidCommand("invalid argument for -h | --head or -t | --tail option");
                return;
            }
        }

        FileIteratorPrx it;
        if(reader == "node")
        {
            if(filename == "stderr")
            {
                it = _session->openNodeStdErr(id, tail ? lineCount : -1);
            }
            else if(filename == "stdout")
            {
                it = _session->openNodeStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                invalidCommand("invalid node log filename `" + filename + "'");
                return;
            }
        }
        else if(reader == "registry")
        {
            if(filename == "stderr")
            {
                it = _session->openRegistryStdErr(id, tail ? lineCount : -1);
            }
            else if(filename == "stdout")
            {
                it = _session->openRegistryStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                invalidCommand("invalid registry log filename `" + filename + "'");
                return;
            }
        }
        else if(reader == "server")
        {
            if(filename == "stderr")
            {
                it = _session->openServerStdErr(id, tail ? lineCount : -1);
            }
            else if(filename == "stdout")
            {
                it = _session->openServerStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                it = _session->openServerLog(id, filename, tail ? lineCount : -1);
            }
        }

        bool follow = opts.isSet("follow");
        resetInterrupt();
        if(head)
        {
            if(follow)
            {
                invalidCommand("can't use -f | --follow option with -h | --head option");
                return;
            }

            int i = 0;
            bool eof = false;
            while(!interrupted() && !eof && i < lineCount)
            {
                eof = it->read(maxBytes, lines);
                for(Ice::StringSeq::const_iterator p = lines.begin(); i < lineCount && p != lines.end(); ++p, ++i)
                {
                    cout << endl << *p << flush;
                }
            }
        }
        else
        {
            bool eof = false;
            while(!interrupted() && !eof)
            {
                eof = it->read(maxBytes, lines);
                for(Ice::StringSeq::const_iterator p = lines.begin(); p != lines.end(); ++p)
                {
                    cout << endl << *p << flush;
                }
            }
        }

        if(follow)
        {
            while(!interrupted())
            {
                bool eof = it->read(maxBytes, lines);
                for(Ice::StringSeq::const_iterator p = lines.begin(); p != lines.end(); ++p)
                {
                    cout << *p;
                    if((p + 1) != lines.end())
                    {
                        cout << endl;
                    }
                    else
                    {
                        cout << flush;
                    }
                }

                if(eof)
                {
                    Lock sync(*this);
                    if(_interrupted)
                    {
                        break;
                    }
                    timedWait(IceUtil::Time::seconds(5));
                }
            }
        }

        if(lines.empty() || !lines.back().empty())
        {
            cout << endl;
        }

        it->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
Parser::showBanner()
{
    cout << "Ice " << ICE_STRING_VERSION << "  Copyright (c) 2003-2013 ZeroC, Inc." << endl;
}

void
Parser::showCopying()
{
    cout << "This command is not implemented." << endl;
}

void
Parser::showWarranty()
{
    cout << "This command is not implemented." << endl;
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
            result = min(maxSize, static_cast<int>(_commands.length()));
            strncpy(buf, _commands.c_str(), result);
            _commands.erase(0, result);
            if(_commands.empty())
            {
                _commands = ";";
            }
        }
    }
    else
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
        
        result = static_cast<int>(line.length());
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
}

void
Parser::continueLine()
{
    _continue = true;
}

const char*
Parser::getPrompt()
{
    assert(_commands.empty());

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
Parser::invalidCommand(const char* s)
{
    error(s);
}

void
Parser::invalidCommand(const string& s)
{
    error(s.c_str());
}

void
Parser::invalidCommand(const string& command, const string& msg)
{
    error("`" + command + "' " + msg + "\n(`" + command + " help' for more info)");
}

void
Parser::invalidCommand(const list<string>& s)
{
    if(s.empty())
    {
        return;
    }
    
    string cat = *s.begin();
    if(_helpCommands.find(cat) == _helpCommands.end())
    {
        cerr << "unknown `" << cat << "' command (see `help' for more info)" << endl;
    }
    else if(s.size() == 1)
    {
        cerr << "invalid `" << cat << "' command (see `" << cat << " help' for more info)" << endl;
    }
    else
    {
        string cmd = *(++s.begin());
        if(_helpCommands[cat].find(cmd) == _helpCommands[cat].end())
        {
            cmd = cat + " " + cmd;
            cerr << "unknown `" << cmd << "' command (see `" << cat << " help' for more info)" << endl;
        }
        else
        {
            cmd = cat + " " + cmd;
            cerr << "invalid `" << cmd << "' command (see `" << cmd << " help' for more info)" << endl;
        }
    }
}

string
Parser::patchFailed(const Ice::StringSeq& reasons)
{
    if(reasons.size() == 1)
    {
        ostringstream s;
        s << "the patch failed:\n" << reasons[0];
        return s.str();
    }
    else
    {
        ostringstream os;
        IceUtilInternal::Output out(os);
        out.setIndent(2);
        out << "the patch failed on some nodes:\n";
        for(Ice::StringSeq::const_iterator p = reasons.begin(); p != reasons.end(); ++p)
        {
            string reason = *p;
            string::size_type beg = 0;
            string::size_type end = reason.find_first_of("\n");
            if(end == string::npos)
            {
                end = reason.size();
            }
            out << "- " << reason.substr(beg, end - beg);
            out.inc();
            while(end < reason.size())
            {
                beg = end + 1;
                end = reason.find_first_of("\n", beg);
                if(end == string::npos)
                {
                    end = reason.size();
                }
                out.newline();
                out << reason.substr(beg, end - beg);
            }
            out.dec();
            if(p + 1 != reasons.end())
            {
                out.newline();
            }
        }
        return os.str();
    }
}

void
Parser::error(const char* s)
{
    cerr << "error: " << s << endl;
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
    cerr << "warning: " << s << endl;
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

    _continue = false;

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

    _continue = false;

    int status = yyparse();
    if(_errors)
    {
        status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

Parser::Parser(const CommunicatorPtr& communicator, 
               const AdminSessionPrx& session, 
               const AdminPrx& admin,
               bool interactive) :
    _communicator(communicator),
    _session(session),
    _admin(admin),
    _interrupted(false),
    _interactive(interactive)
{
    for(int i = 0; _commandsHelp[i][0]; i++)
    {
        const string category = _commandsHelp[i][0];
        const string cmd = _commandsHelp[i][1];
        const string help = _commandsHelp[i][2];
        _helpCommands[category][""] += help;
        _helpCommands[category][cmd] += help;
    }
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
    catch(const RegistryNotExistException& ex)
    {
        error("couldn't find registry `" + ex.name + "'");
    }
    catch(const ServerNotExistException& ex)
    {
        error("couldn't find server `" + ex.id + "'");
    }
    catch(const AdapterNotExistException& ex)
    {
        error("couldn't find adapter `" + ex.id + "'");
    }
    catch(const ObjectNotRegisteredException& ex)
    {
        error("couldn't find object `" + _communicator->identityToString(ex.id) + "'");
    }
    catch(const ObjectExistsException& ex)
    {
        error("object `" + _communicator->identityToString(ex.id) + "' already exists");
    }
    catch(const DeploymentException& ex)
    {
        ostringstream s;
        s << ex << ":\n" << ex.reason;
        error(s.str()); 
    }
    catch(const PatchException& ex)
    {
        error(patchFailed(ex.reasons));
    }
    catch(const BadSignalException& ex)
    {
        ostringstream s;
        s << ex.reason;
        error(s.str());
    }
    catch(const NodeUnreachableException& ex)
    {
        error("node `" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch(const RegistryUnreachableException& ex)
    {
        error("registry `" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch(const ServerUnreachableException& ex)
    {
        error("server `" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch(const AccessDeniedException& ex)
    {
        error("couldn't update the registry, the session from `" + ex.lockUserId + "' is updating the registry");
    }
    catch(const FileNotAvailableException& ex)
    {
        error("couldn't access file:\n" + ex.reason);
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
        error(s.str());
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}
