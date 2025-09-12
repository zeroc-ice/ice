// Copyright (c) ZeroC, Inc.

#include "Parser.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "../Ice/TimeUtil.h"
#include "DescriptorHelper.h"
#include "DescriptorParser.h"
#include "Ice/Ice.h"
#include "IceBox/IceBox.h"
#include "Util.h"
#include "XMLParser.h"

#include "../Ice/DisableWarnings.h"

#if defined(__APPLE__) || defined(__linux__)
#    include <editline/readline.h>
#    include <unistd.h>
#elif defined(_WIN32)
#    include <windows.h>
#endif

#include <iomanip>
#include <iterator>

extern FILE* yyin;
extern int yydebug;

int yyparse();

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceGrid;

namespace
{
    const char* _commandsHelp[][3] = {
        {"application",
         "add",
         "application add DESC [TARGET ... ] [NAME=VALUE ... ]\n"
         "                          Add application described in DESC. If specified\n"
         "                          the optional targets TARGET will be deployed.\n"},
        {"application", "remove", "application remove NAME   Remove application NAME.\n"},
        {"application", "describe", "application describe NAME Describe application NAME.\n"},
        {"application",
         "diff",
         "application diff [-s | --servers] DESC [TARGET ... ] [NAME=VALUE ... ]\n"
         "                          Print the differences between the application\n"
         "                          described in DESC and the current deployment.\n"
         "                          If -s or --servers is specified, print the\n"
         "                          the list of servers affected by the differences.\n"},
        {"application",
         "update",
         "application update [-n | --no-restart] DESC [TARGET ... ] [NAME=VALUE ... ]\n"
         "                          Update the application described in DESC. If -n or\n"
         "                          --no-restart is specified, the update will fail if\n"
         "                          it is necessary to stop some servers.\n"},
        {"application", "list", "application list          List all deployed applications.\n"},
        {"server template",
         "instantiate",
         "server template instantiate APPLICATION NODE TEMPLATE [NAME=VALUE ...]\n"
         "                          Instantiate a server template.\n"},
        {"server template",
         "describe",
         "server template describe APPLICATION TEMPLATE\n"
         "                          Describe application server template TEMPLATE.\n"},
        {"service template",
         "describe",
         "service template describe APPLICATION TEMPLATE\n"
         "                          Describe application service template TEMPLATE.\n"},
        {"node", "list", "node list                 List all registered nodes.\n"},
        {"node", "describe", "node describe NAME        Show information about node NAME.\n"},
        {"node", "ping", "node ping NAME            Ping node NAME.\n"},
        {"node", "load", "node load NAME            Print the load of the node NAME.\n"},
        {"node",
         "sockets",
         "node sockets [NAME]       Print the number of CPU sockets of the\n"
         "                          node NAME or all the nodes if NAME is omitted.\n"},
        {"node",
         "show",
         "node show [OPTIONS] NAME [log | stderr | stdout]\n"
         "                          Show node NAME Ice log, stderr or stdout.\n"
         "                          Options:\n"
         "                           -f | --follow: Wait for new data to be available\n"
         "                           -t N | --tail N: Print the last N log messages or lines\n"
         "                           -h N | --head N: Print the first N lines (stderr and stdout only)\n"},
        {"node", "shutdown", "node shutdown NAME        Shutdown node NAME.\n"},
        {"registry", "list", "registry list             List all registered registries.\n"},
        {"registry", "describe", "registry describe NAME    Show information about registry NAME.\n"},
        {"registry", "ping", "registry ping NAME        Ping registry NAME.\n"},
        {"registry",
         "show",
         "registry show [OPTIONS] NAME [log | stderr | stdout ]\n"
         "                          Show registry NAME Ice log, stderr or stdout.\n"
         "                          Options:\n"
         "                           -f | --follow: Wait for new log or data to be available\n"
         "                           -t N | --tail N: Print the last N log messages or lines\n"
         "                           -h N | --head N: Print the first N lines (stderr and stdout only)\n"},
        {"registry", "shutdown", "registry shutdown NAME    Shutdown registry NAME.\n"},
        {"server", "list", "server list               List all registered servers.\n"},
        {"server", "remove", "server remove ID          Remove server ID.\n"},
        {"server", "describe", "server describe ID        Describe server ID.\n"},
        {"server", "properties", "server properties ID      Get the run-time properties of server ID.\n"},
        {"server", "property", "server property ID NAME   Get the run-time property NAME of server ID.\n"},
        {"server", "state", "server state ID           Get the state of server ID.\n"},
        {"server", "pid", "server pid ID             Get the process id of server ID.\n"},
        {"server", "start", "server start ID           Start server ID.\n"},
        {"server", "stop", "server stop ID            Stop server ID.\n"},
        {"server", "signal", "server signal ID SIGNAL   Send SIGNAL (e.g. SIGTERM or 15) to server ID.\n"},
        {"server", "stdout", "server stdout ID MESSAGE  Write MESSAGE on server ID's stdout.\n"},
        {"server", "stderr", "server stderr ID MESSAGE  Write MESSAGE on server ID's stderr.\n"},
        {"server",
         "show",
         "server show [OPTIONS] ID [log | stderr | stdout | LOGFILE ]\n"
         "                          Show server ID Ice log, stderr, stdout or log file LOGFILE.\n"
         "                          Options:\n"
         "                           -f | --follow: Wait for new data to be available\n"
         "                           -t N | --tail N: Print the last N log messages or lines\n"
         "                           -h N | --head N: Print the first N lines (not available for Ice log)\n"},
        {"server", "enable", "server enable ID          Enable server ID.\n"},
        {"server",
         "disable",
         "server disable ID         Disable server ID (a disabled server can't be\n"
         "                          started on demand or administratively).\n"},
        {"service", "start", "service start ID NAME     Starts service NAME in IceBox server ID.\n"},
        {"service", "stop", "service stop ID NAME      Stops service NAME in IceBox server ID.\n"},
        {"service", "describe", "service describe ID NAME  Describes service NAME in IceBox server ID.\n"},
        {"service",
         "properties",
         "service properties ID NAME\n"
         "                          Get the run-time properties of service NAME in\n"
         "                          IceBox server ID.\n"},
        {"service",
         "property",
         "service property ID NAME PROPERTY\n"
         "                          Get the run-time property PROPERTY of service NAME\n"
         "                          from IceBox server ID.\n"},
        {"service", "list", "service list ID           List the services in IceBox server ID.\n"},
        {"adapter", "list", "adapter list              List all registered adapters.\n"},
        {"adapter", "endpoints", "adapter endpoints ID      Show the endpoints of adapter or replica group ID.\n"},
        {"adapter", "remove", "adapter remove ID         Remove adapter or replica group ID.\n"},
        {"object",
         "add",
         "object add PROXY [TYPE]   Add an object to the object registry,\n"
         "                          optionally specifying its type.\n"},
        {"object", "remove", "object remove IDENTITY    Remove an object from the object registry.\n"},
        {"object", "find", "object find TYPE          Find all objects with the type TYPE.\n"},
        {"object",
         "describe",
         "object describe EXPR      Describe all registered objects whose stringified\n"
         "                          identities match the expression EXPR. A trailing\n"
         "                          wildcard is supported in EXPR, for example\n"
         "                          \"object describe Ice*\".\n"},
        {"object",
         "list",
         "object list EXPR          List all registered objects whose stringified\n"
         "                          identities match the expression EXPR. A trailing\n"
         "                          wildcard is supported in EXPR, for example\n"
         "                          \"object list Ice*\".\n"},
        {nullptr, nullptr, nullptr}};

    int loggerCallbackCount = 0;

#ifdef _WIN32
    shared_ptr<Ice::StringConverter> windowsConsoleConverter = nullptr;
#endif

    void outputNewline() { consoleOut << endl; }

    void flushOutput() { consoleOut << flush; }

    void outputString(const string& s) { consoleOut << s; }

    void writeMessage(const string& message, bool indent)
    {
        string s = message;

        if (indent)
        {
            string::size_type idx = 0;
            while ((idx = s.find('\n', idx)) != string::npos)
            {
                s.insert(idx + 1, "   ");
                ++idx;
            }
        }

        outputString(s);
        outputNewline();
        flushOutput();
    }

    void printLogMessage(const string& p, const Ice::LogMessage& logMessage)
    {
        string prefix = p;

        if (!prefix.empty())
        {
            prefix += ": ";
        }

        string timestamp = IceInternal::timePointToDateTimeString(
            chrono::system_clock::time_point(chrono::microseconds(logMessage.timestamp)));

        switch (logMessage.type)
        {
            case Ice::LogMessageType::PrintMessage:
            {
                writeMessage(timestamp + " " + logMessage.message, false);
                break;
            }
            case Ice::LogMessageType::TraceMessage:
            {
                string s = "-- " + timestamp + " " + prefix;
                if (!logMessage.traceCategory.empty())
                {
                    s += logMessage.traceCategory + ": ";
                }
                s += logMessage.message;
                writeMessage(s, true);
                break;
            }
            case Ice::LogMessageType::WarningMessage:
            {
                writeMessage("!- " + timestamp + " " + prefix + "warning: " + logMessage.message, true);
                break;
            }
            case Ice::LogMessageType::ErrorMessage:
            {
                writeMessage("!! " + timestamp + " " + prefix + "error: " + logMessage.message, true);
                break;
            }
            default:
            {
                assert(0);
            }
        }
    }

    class RemoteLoggerI final : public Ice::RemoteLogger
    {
    public:
        RemoteLoggerI();

        void init(string, Ice::LogMessageSeq, const Ice::Current&) override;
        void log(Ice::LogMessage, const Ice::Current&) override;

        void destroy();

    private:
        mutex _mutex;
        condition_variable _condVar;
        bool _initDone{false};
        bool _destroyed{false};
        string _prefix;
    };

    RemoteLoggerI::RemoteLoggerI() = default;

    void RemoteLoggerI::init(string prefix, Ice::LogMessageSeq logMessages, const Ice::Current&)
    {
        lock_guard lock(_mutex);
        if (!_destroyed)
        {
            _prefix = prefix;

            for (const auto& message : logMessages)
            {
                printLogMessage(_prefix, message);
            }

            _initDone = true;
            _condVar.notify_all();
        }
    }

    void RemoteLoggerI::log(Ice::LogMessage logMessage, const Ice::Current&)
    {
        unique_lock lock(_mutex);

        _condVar.wait(lock, [this] { return _initDone || _destroyed; });

        if (!_destroyed)
        {
            printLogMessage(_prefix, logMessage);
        }
    }

    void RemoteLoggerI::destroy()
    {
        lock_guard lock(_mutex);
        _destroyed = true;
        _condVar.notify_all();
    }
}

namespace IceGrid
{
    Parser* parser;
}

Parser::Parser(CommunicatorPtr communicator, AdminSessionPrx session, optional<AdminPrx> admin, bool interactive)
    : _communicator(std::move(communicator)),
      _session(std::move(session)),
      _admin(std::move(admin)),
      _interactive(interactive)
{
    for (int i = 0; _commandsHelp[i][0]; i++)
    {
        const string category = _commandsHelp[i][0];
        const string cmd = _commandsHelp[i][1];
        const string help = _commandsHelp[i][2];
        _helpCommands[category][""] += help;
        _helpCommands[category][cmd] += help;
    }

#ifdef _WIN32
    if (!windowsConsoleConverter)
    {
        windowsConsoleConverter = Ice::createWindowsStringConverter(GetConsoleOutputCP());
    }
#endif
}

void
Parser::usage(const string& category, const string& command)
{
    if (_helpCommands.find(category) == _helpCommands.end())
    {
        invalidCommand("unknown command '" + category + "'");
    }
    else if (_helpCommands[category].find(command) == _helpCommands[category].end())
    {
        invalidCommand("unknown command '" + category + " " + command + "'");
    }
    else
    {
        consoleOut << _helpCommands[category][command];
    }
}

void
Parser::usage(const string& category, const list<string>& args)
{
    if (args.empty())
    {
        usage(category);
    }
    else if (args.size() > 1)
    {
        invalidCommand("'help' requires at most 1 argument");
    }
    else
    {
        usage(category, *args.begin());
    }
}

void
Parser::usage()
{
    consoleOut << "help                        Print this message.\n"
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
    lock_guard lock(_mutex);
    _interrupted = true;
    _condVar.notify_all();
}

bool
Parser::interrupted() const
{
    lock_guard lock(_mutex);
    return _interrupted;
}

void
Parser::resetInterrupt()
{
    lock_guard lock(_mutex);
    _interrupted = false;
}

void
Parser::checkInterrupted()
{
    if (!_interactive)
    {
        lock_guard lock(_mutex);
        if (_interrupted)
        {
            throw runtime_error("interrupted with Ctrl-C");
        }
    }
}

void
Parser::addApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.emplace_front("icegridadmin");

    IceInternal::Options opts;
    vector<string> args;
    try
    {
        for (const auto& arg : copyArgs)
        {
            args.push_back(arg);
        }
        args = opts.parse(args);
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        return;
    }

    if (args.size() < 1)
    {
        invalidCommand("application add", "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        auto p = args.begin();
        string desc = *p++;

        for (; p != args.end(); ++p)
        {
            string::size_type pos = p->find('=');
            if (pos != string::npos)
            {
                vars[p->substr(0, pos)] = p->substr(pos + 1);
            }
            else
            {
                targets.push_back(*p);
            }
        }

        // Add the application.
        assert(_admin);
        ApplicationDescriptor app = DescriptorParser::parseDescriptor(desc, targets, vars, _communicator, *_admin);
        _admin->addApplication(app);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::removeApplication(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("application remove", "requires exactly one argument");
        return;
    }

    try
    {
        auto p = args.begin();

        string name = *p++;

        _admin->removeApplication(name);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeApplication(const list<string>& args)
{
    if (args.size() < 1)
    {
        invalidCommand("application describe", "requires at least one argument");
        return;
    }

    try
    {
        auto p = args.begin();

        string name = *p++;
        ostringstream os;
        Output out(os);
        ApplicationInfo info = _admin->getApplicationInfo(name);
        ApplicationHelper helper(_communicator, info.descriptor);
        helper.print(out, info);
        out << nl;
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::diffApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.emplace_front("icegridadmin");

    IceInternal::Options opts;
    opts.addOpt("s", "servers");
    vector<string> args;
    try
    {
        for (const auto& arg : copyArgs)
        {
            args.push_back(arg);
        }
        args = opts.parse(args);
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        return;
    }

    if (args.size() < 1)
    {
        invalidCommand("application diff", "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        auto arg = args.begin();
        string desc = *arg++;

        for (; arg != args.end(); ++arg)
        {
            string::size_type pos = arg->find('=');
            if (pos != string::npos)
            {
                vars[arg->substr(0, pos)] = arg->substr(pos + 1);
            }
            else
            {
                targets.push_back(*arg);
            }
        }

        assert(_admin);
        ApplicationDescriptor newApp = DescriptorParser::parseDescriptor(desc, targets, vars, _communicator, *_admin);
        ApplicationInfo origApp = _admin->getApplicationInfo(newApp.name);

        ApplicationHelper newAppHelper(_communicator, newApp);
        ApplicationHelper oldAppHelper(_communicator, origApp.descriptor);

        ostringstream os;
        Output out(os);
        if (opts.isSet("servers"))
        {
            map<string, ServerInfo> oldServers = oldAppHelper.getServerInfos(origApp.uuid, origApp.revision);
            map<string, ServerInfo> newServers = newAppHelper.getServerInfos(origApp.uuid, origApp.revision);

            vector<string> messages;
            map<string, ServerInfo>::const_iterator p;
            for (p = oldServers.begin(); p != oldServers.end(); ++p)
            {
                auto q = newServers.find(p->first);
                if (q == newServers.end())
                {
                    messages.push_back("server '" + p->first + "': removed");
                }
            }

            for (p = newServers.begin(); p != newServers.end(); ++p)
            {
                auto q = oldServers.find(p->first);
                if (q == oldServers.end())
                {
                    messages.push_back("server '" + p->first + "': added");
                }
                else if (isServerUpdated(p->second, q->second))
                {
                    if (isServerUpdated(p->second, q->second, true)) // Ignore properties
                    {
                        messages.push_back("server '" + p->first + "': updated (restart required)");
                    }
                    else
                    {
                        messages.push_back("server '" + p->first + "': properties updated (no restart required)");
                    }
                }
            }

            out << "application '" << origApp.descriptor.name << "'";
            out << sb;
            sort(messages.begin(), messages.end());
            for (const auto& message : messages)
            {
                out << nl << message;
            }
            out << eb;
        }
        else
        {
            newAppHelper.printDiff(out, oldAppHelper);
        }
        out << nl;
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::updateApplication(const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.emplace_front("icegridadmin");

    IceInternal::Options opts;
    opts.addOpt("n", "no-restart");
    vector<string> args;
    try
    {
        for (const auto& copyArg : copyArgs)
        {
            args.push_back(copyArg);
        }
        args = opts.parse(args);
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        return;
    }

    if (args.size() < 1)
    {
        invalidCommand("application update", "requires at least one argument");
        return;
    }

    try
    {
        StringSeq targets;
        map<string, string> vars;

        auto p = args.begin();
        string xml = *p++;

        for (; p != args.end(); ++p)
        {
            string::size_type pos = p->find('=');
            if (pos != string::npos)
            {
                vars[p->substr(0, pos)] = p->substr(pos + 1);
            }
            else
            {
                targets.push_back(*p);
            }
        }

        assert(_admin);
        ApplicationDescriptor desc = DescriptorParser::parseDescriptor(xml, targets, vars, _communicator, *_admin);
        if (opts.isSet("no-restart"))
        {
            _admin->syncApplicationWithoutRestart(desc);
        }
        else
        {
            _admin->syncApplication(desc);
        }
    }
    catch (const Ice::OperationNotExistException&)
    {
        error("registry doesn't support updates without restart");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listAllApplications(const list<string>& args)
{
    if (!args.empty())
    {
        invalidCommand("application list", "doesn't require any argument");
        return;
    }

    try
    {
        Ice::StringSeq names = _admin->getAllApplicationNames();
        ostringstream os;
        copy(names.begin(), names.end(), ostream_iterator<string>(os, "\n"));
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeServerTemplate(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("server template describe", "requires exactly two arguments");
        return;
    }

    try
    {
        auto p = args.begin();

        string name = *p++;
        string templ = *p++;

        ApplicationInfo application = _admin->getApplicationInfo(name);

        ostringstream os;
        Output out(os);
        auto q = application.descriptor.serverTemplates.find(templ);
        if (q != application.descriptor.serverTemplates.end())
        {
            out << "server template '" << templ << "'";
            out << sb;

            out << nl << "parameters = '" << toString(q->second.parameters) << "'";
            out << nl;

            auto server = dynamic_pointer_cast<ServerDescriptor>(q->second.descriptor);
            auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(server);
            if (iceBox)
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
            error("no server template with id '" + templ + "'");
        }
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::instantiateServerTemplate(const list<string>& args)
{
    if (args.size() < 3)
    {
        invalidCommand("server template instantiate", "requires at least three arguments");
        return;
    }

    try
    {
        map<string, string> vars;

        auto p = args.begin();
        string application = *p++;
        string node = *p++;
        string templ = *p++;
        for (; p != args.end(); ++p)
        {
            string::size_type pos = p->find('=');
            if (pos != string::npos)
            {
                vars[p->substr(0, pos)] = p->substr(pos + 1);
            }
        }

        ServerInstanceDescriptor desc;
        desc.templateName = templ;
        desc.parameterValues = vars;
        _admin->instantiateServer(application, node, desc);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeServiceTemplate(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("service template describe", "requires exactly two arguments");
        return;
    }

    try
    {
        auto p = args.begin();

        string name = *p++;
        string templ = *p++;

        ApplicationInfo application = _admin->getApplicationInfo(name);

        ostringstream os;
        Output out(os);
        auto q = application.descriptor.serviceTemplates.find(templ);
        if (q != application.descriptor.serviceTemplates.end())
        {
            out << "service template '" << templ << "'";
            out << sb;

            out << nl << "parameters = '" << toString(q->second.parameters) << "'";
            out << nl;

            auto desc = dynamic_pointer_cast<ServiceDescriptor>(q->second.descriptor);
            ServiceHelper(desc).print(_communicator, out);
            out << eb;
            out << nl;
        }
        else
        {
            invalidCommand("no service template with id '" + templ + "'");
        }
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeNode(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("node describe", "requires exactly one argument");
        return;
    }

    try
    {
        NodeInfo info = _admin->getNodeInfo(args.front());
        ostringstream os;
        Output out(os);
        out << "node '" << args.front() << "'";
        out << sb;
        out << nl << "operating system = '" << info.os << "'";
        out << nl << "host name = '" << info.hostname << "'";
        out << nl << "release = '" << info.release << "'";
        out << nl << "version = '" << info.version << "'";
        out << nl << "machine type = '" << info.machine << "'";
        out << nl << "number of threads = '" << info.nProcessors << "'";
        out << eb;
        out << nl;
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::pingNode(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("node ping", "requires exactly one argument");
        return;
    }

    try
    {
        if (_admin->pingNode(args.front()))
        {
            consoleOut << "node is up" << endl;
        }
        else
        {
            consoleOut << "node is down" << endl;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::printLoadNode(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("node load", "requires exactly one argument");
        return;
    }

    try
    {
        LoadInfo load = _admin->getNodeLoad(args.front());
        consoleOut << "load average (1/5/15): " << load.avg1 << " / " << load.avg5 << " / " << load.avg15 << endl;
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::printNodeProcessorSockets(const list<string>& args)
{
    if (args.size() > 1)
    {
        invalidCommand("node sockets", "requires no more than one argument");
        return;
    }

    try
    {
        if (args.size() == 1)
        {
            try
            {
                consoleOut << _admin->getNodeProcessorSocketCount(args.front()) << endl;
            }
            catch (const Ice::OperationNotExistException&)
            {
                consoleOut << "not supported" << endl;
            }
        }
        else
        {
            Ice::StringSeq names = _admin->getAllNodeNames();
            map<string, pair<vector<string>, int>> processorSocketCounts;
            for (const auto& name : names)
            {
                try
                {
                    NodeInfo info = _admin->getNodeInfo(name);
                    processorSocketCounts[info.hostname].first.push_back(name);
                    try
                    {
                        processorSocketCounts[info.hostname].second = _admin->getNodeProcessorSocketCount(name);
                    }
                    catch (const Ice::OperationNotExistException&)
                    {
                        // Not supported.
                        processorSocketCounts[info.hostname].second = 0;
                    }
                }
                catch (const NodeNotExistException&)
                {
                }
                catch (const NodeUnreachableException&)
                {
                }
            }

            ostringstream os;
            os.flags(ios::left);
            os << setw(20) << "Hostname" << setw(20) << "| # of sockets" << setw(39) << "| Nodes" << endl;
            os << setw(79) << "=====================================================================" << endl;
            for (const auto& processorSocketCount : processorSocketCounts)
            {
                os << setw(20) << setiosflags(ios::left) << processorSocketCount.first;
                os << "| " << setw(18) << setiosflags(ios::left) << processorSocketCount.second.second;
                os << "| " << setw(37) << setiosflags(ios::left) << toString(processorSocketCount.second.first);
                os << endl;
            }
            consoleOut << os.str() << flush;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::shutdownNode(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("node shutdown", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->shutdownNode(args.front());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listAllNodes(const list<string>& args)
{
    if (!args.empty())
    {
        invalidCommand("node list", "doesn't require any argument");
        return;
    }

    try
    {
        ostringstream os;
        Ice::StringSeq names = _admin->getAllNodeNames();
        copy(names.begin(), names.end(), ostream_iterator<string>(os, "\n"));
        consoleOut << os.str();
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeRegistry(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("registry describe", "requires exactly one argument");
        return;
    }

    try
    {
        RegistryInfo info = _admin->getRegistryInfo(args.front());
        ostringstream os;
        Output out(os);
        out << "registry '" << args.front() << "'";
        out << sb;
        out << nl << "host name = '" << info.hostname << "'";
        out << eb;
        out << nl;
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::pingRegistry(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("registry ping", "requires exactly one argument");
        return;
    }

    try
    {
        if (_admin->pingRegistry(args.front()))
        {
            consoleOut << "registry is up" << endl;
        }
        else
        {
            consoleOut << "registry is down" << endl;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::shutdownRegistry(const list<string>& args)
{
    if (args.size() > 1)
    {
        invalidCommand("registry shutdown", "requires at most one argument");
        return;
    }

    try
    {
        if (args.empty())
        {
            _admin->shutdown();
        }
        else
        {
            _admin->shutdownRegistry(args.front());
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listAllRegistries(const list<string>& args)
{
    if (!args.empty())
    {
        invalidCommand("registry list", "doesn't require any argument");
        return;
    }

    try
    {
        ostringstream os;
        Ice::StringSeq names = _admin->getAllRegistryNames();
        copy(names.begin(), names.end(), ostream_iterator<string>(os, "\n"));
        consoleOut << os.str();
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::removeServer(const list<string>& args)
{
    if (args.size() != 1)
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
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::startServer(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("server start", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->startServer(args.front());
    }
    catch (const ServerStartException& ex)
    {
        error("the server didn't start successfully:\n" + ex.reason);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::stopServer(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("server stop", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->stopServer(args.front());
    }
    catch (const ServerStopException& ex)
    {
        error("the server didn't stop successfully:\n" + ex.reason);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::signalServer(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("server signal", "requires exactly two arguments");
        return;
    }

    try
    {
        auto p = args.begin();
        string server = *p++;
        _admin->sendSignal(server, *p);
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::writeMessage(const list<string>& args, int fd)
{
    if (args.size() != 2)
    {
        invalidCommand("server stdout or server stderr", "requires exactly two arguments");
        return;
    }

    try
    {
        auto p = args.begin();
        string server = *p++;

        auto serverAdmin = _admin->getServerAdmin(server);
        auto process = serverAdmin->ice_facet<ProcessPrx>("Process");

        process->writeMessage(*p, fd);
    }
    catch (const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch (const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'Process' facet");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeServer(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("server describe", "requires exactly one argument");
        return;
    }

    try
    {
        ServerInfo info = _admin->getServerInfo(args.front());
        ostringstream os;
        Output out(os);
        auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
        if (iceBox)
        {
            IceBoxHelper(iceBox).print(_communicator, out, info);
        }
        else
        {
            ServerHelper(info.descriptor).print(_communicator, out, info);
        }
        out << nl;
        outputString(os.str());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::stateServer(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("server state", "requires exactly one argument");
        return;
    }

    try
    {
        ServerState state = _admin->getServerState(args.front());
        string enabled = _admin->isServerEnabled(args.front()) ? "enabled" : "disabled";
        switch (state)
        {
            case ServerState::Inactive:
            {
                consoleOut << "inactive (" << enabled << ")" << endl;
                break;
            }
            case ServerState::Activating:
            {
                consoleOut << "activating (" << enabled << ")" << endl;
                break;
            }
            case ServerState::Active:
            {
                int pid = _admin->getServerPid(args.front());
                consoleOut << "active (pid = " << pid << ", " << enabled << ")" << endl;
                break;
            }
            case ServerState::ActivationTimedOut:
            {
                int pid = _admin->getServerPid(args.front());
                consoleOut << "activation timed out (pid = " << pid << ", " << enabled << ")" << endl;
                break;
            }
            case ServerState::Deactivating:
            {
                consoleOut << "deactivating (" << enabled << ")" << endl;
                break;
            }
            case ServerState::Destroying:
            {
                consoleOut << "destroying (" << enabled << ")" << endl;
                break;
            }
            case ServerState::Destroyed:
            {
                consoleOut << "destroyed (" << enabled << ")" << endl;
                break;
            }
            default:
                assert(false);
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::pidServer(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("server pid", "requires exactly one argument");
        return;
    }

    try
    {
        int pid = _admin->getServerPid(args.front());
        if (pid > 0)
        {
            consoleOut << pid << endl;
        }
        else
        {
            error("server is not running");
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::propertiesServer(const list<string>& args, bool single)
{
    if (single && args.size() != 2)
    {
        invalidCommand("server property", "requires exactly two arguments");
        return;
    }
    else if (!single && args.size() != 1)
    {
        invalidCommand("server properties", "requires exactly one argument");
        return;
    }

    try
    {
        auto serverAdmin = _admin->getServerAdmin(args.front());
        auto propAdmin = serverAdmin->ice_facet<Ice::PropertiesAdminPrx>("Properties");

        if (single)
        {
            string val = propAdmin->getProperty(*(++args.begin()));
            consoleOut << val << endl;
        }
        else
        {
            for (const auto& prop : propAdmin->getPropertiesForPrefix(""))
            {
                consoleOut << prop.first << "=" << prop.second << endl;
            }
        }
    }
    catch (const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch (const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'Properties' facet");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::enableServer(const list<string>& args, bool enable)
{
    if (args.size() != 1)
    {
        if (enable)
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
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listAllServers(const list<string>& args)
{
    if (!args.empty())
    {
        invalidCommand("server list", "doesn't require any argument");
        return;
    }

    try
    {
        ostringstream os;
        Ice::StringSeq ids = _admin->getAllServerIds();
        copy(ids.begin(), ids.end(), ostream_iterator<string>(os, "\n"));
        consoleOut << os.str();
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::startService(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("service start", "requires exactly two arguments");
        return;
    }

    const string& server = args.front();
    string service = *(++args.begin());
    try
    {
        auto admin = _admin->getServerAdmin(server);
        auto manager = admin->ice_facet<IceBox::ServiceManagerPrx>("IceBox.ServiceManager");
        manager->startService(service);
    }
    catch (const IceBox::AlreadyStartedException&)
    {
        error("the service '" + service + "' is already started");
    }
    catch (const IceBox::NoSuchServiceException&)
    {
        error("couldn't find service '" + service + "'");
    }
    catch (const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch (const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'IceBox.ServiceManager' facet");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::stopService(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("service stop", "requires exactly two arguments");
        return;
    }

    const string& server = args.front();
    string service = *(++args.begin());
    try
    {
        auto admin = _admin->getServerAdmin(server);
        auto manager = admin->ice_facet<IceBox::ServiceManagerPrx>("IceBox.ServiceManager");
        manager->stopService(service);
    }
    catch (const IceBox::AlreadyStoppedException&)
    {
        error("the service '" + service + "' is already stopped");
    }
    catch (const IceBox::NoSuchServiceException&)
    {
        error("couldn't find service '" + service + "'");
    }
    catch (const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch (const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide a 'IceBox.ServiceManager' facet");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeService(const list<string>& args)
{
    if (args.size() != 2)
    {
        invalidCommand("service describe", "requires exactly two arguments");
        return;
    }

    const string& server = args.front();
    string service = *(++args.begin());
    try
    {
        ServerInfo info = _admin->getServerInfo(server);
        auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
        if (!iceBox)
        {
            error("server '" + server + "' is not an IceBox server");
            return;
        }

        ostringstream os;
        Output out(os);
        bool found = false;
        for (const auto& s : iceBox->services)
        {
            if (s.descriptor && s.descriptor->name == service)
            {
                ServiceHelper(s.descriptor).print(_communicator, out);
                out << nl;
                found = true;
                break;
            }
        }
        outputString(os.str());

        if (!found)
        {
            error("couldn't find service '" + service + "'");
            return;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::propertiesService(const list<string>& args, bool single)
{
    if (single && args.size() != 3)
    {
        invalidCommand("service property", "requires exactly three arguments");
        return;
    }
    else if (!single && args.size() != 2)
    {
        invalidCommand("service properties", "requires exactly two argument");
        return;
    }

    auto a = args.begin();
    string server = *a++;
    string service = *a++;
    string property = single ? *a++ : string();

    try
    {
        //
        // First, we ensure that the service exists.
        //
        ServerInfo info = _admin->getServerInfo(server);
        auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
        if (!iceBox)
        {
            error("server '" + server + "' is not an IceBox server");
            return;
        }

        bool found = false;

        for (const auto& s : iceBox->services)
        {
            if (s.descriptor && s.descriptor->name == service)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            error("couldn't find service '" + service + "'");
            return;
        }

        auto admin = _admin->getServerAdmin(server);

        const bool useSharedCommunicator =
            getPropertyAsInt(info.descriptor->propertySet.properties, "IceBox.UseSharedCommunicator." + service) > 0;
        auto propAdmin = useSharedCommunicator
                             ? admin->ice_facet<Ice::PropertiesAdminPrx>("IceBox.SharedCommunicator.Properties")
                             : admin->ice_facet<Ice::PropertiesAdminPrx>("IceBox.Service." + service + ".Properties");

        if (single)
        {
            string val = propAdmin->getProperty(property);
            consoleOut << val << endl;
        }
        else
        {
            for (const auto& [key, value] : propAdmin->getPropertiesForPrefix(""))
            {
                consoleOut << key << "=" << value << endl;
            }
        }
    }
    catch (const Ice::ObjectNotExistException&)
    {
        error("couldn't reach the server's Admin object");
    }
    catch (const Ice::FacetNotExistException&)
    {
        error("the server's Admin object does not provide an 'IceBox.Service." + service + ".Properties' facet");
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listServices(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("service list", "requires exactly one argument");
        return;
    }

    const string& server = args.front();
    try
    {
        ServerInfo info = _admin->getServerInfo(server);
        auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
        if (!iceBox)
        {
            error("server '" + server + "' is not an IceBox server");
            return;
        }
        for (const auto& s : iceBox->services)
        {
            if (s.descriptor)
            {
                consoleOut << s.descriptor->name << endl;
            }
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::endpointsAdapter(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("adapter endpoints", "requires exactly one argument");
        return;
    }

    try
    {
        const string& adapterId = args.front();
        AdapterInfoSeq adpts = _admin->getAdapterInfo(adapterId);
        if (adpts.size() == 1 && adpts.begin()->id == adapterId)
        {
            auto proxy = adpts.begin()->proxy;
            if (proxy)
            {
                consoleOut << proxy << endl;
            }
            else
            {
                consoleOut << "<inactive>" << endl;
            }
        }
        else
        {
            for (const auto& adpt : adpts)
            {
                consoleOut << (adpt.id.empty() ? string("<empty>") : adpt.id) << ": ";
                auto proxy = adpt.proxy;
                if (proxy)
                {
                    consoleOut << proxy << endl;
                }
                else
                {
                    consoleOut << "<inactive>" << endl;
                }
            }
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::removeAdapter(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("adapter remove", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->removeAdapter(*args.begin());
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listAllAdapters(const list<string>& args)
{
    if (!args.empty())
    {
        invalidCommand("adapter list", "doesn't require any argument");
        return;
    }

    try
    {
        ostringstream os;
        Ice::StringSeq ids = _admin->getAllAdapterIds();
        copy(ids.begin(), ids.end(), ostream_iterator<string>(os, "\n"));
        consoleOut << os.str();
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::addObject(const list<string>& args)
{
    if (args.size() != 1 && args.size() != 2)
    {
        invalidCommand("object add", "requires one or two arguments");
        return;
    }

    try
    {
        auto p = args.begin();

        string proxy = *p++;

        if (p != args.end())
        {
            string type = *p++;
            _admin->addObjectWithType(_communicator->stringToProxy(proxy), type);
        }
        else
        {
            _admin->addObject(_communicator->stringToProxy(proxy));
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::removeObject(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("object remove", "requires exactly one argument");
        return;
    }

    try
    {
        _admin->removeObject(Ice::stringToIdentity((*(args.begin()))));
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::findObject(const list<string>& args)
{
    if (args.size() != 1)
    {
        invalidCommand("object find", "requires exactly one argument");
        return;
    }

    try
    {
        ObjectInfoSeq objects = _admin->getObjectInfosByType(*(args.begin()));
        for (const auto& object : objects)
        {
            consoleOut << object.proxy << endl;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::describeObject(const list<string>& args)
{
    if (args.size() > 1)
    {
        invalidCommand("object describe", "requires at most one argument");
        return;
    }

    try
    {
        ObjectInfoSeq objects;
        if (args.size() == 1)
        {
            string arg = *(args.begin());
            if (arg.find('*') == string::npos)
            {
                ObjectInfo info = _admin->getObjectInfo(Ice::stringToIdentity(arg));
                consoleOut << "proxy = '" << info.proxy << "'" << endl;
                consoleOut << "type = '" << info.type << "'" << endl;
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

        for (const auto& object : objects)
        {
            consoleOut << "proxy = '" << object.proxy << "' type = '" << object.type << "'" << endl;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::listObject(const list<string>& args)
{
    if (args.size() > 1)
    {
        invalidCommand("object list", "requires at most one argument");
        return;
    }

    try
    {
        ObjectInfoSeq objects;
        if (args.size() == 1)
        {
            objects = _admin->getAllObjectInfos(*(args.begin()));
        }
        else
        {
            objects = _admin->getAllObjectInfos("");
        }

        for (const auto& object : objects)
        {
            consoleOut << _communicator->identityToString(object.proxy->ice_getIdentity()) << endl;
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::show(const string& reader, const list<string>& origArgs)
{
    list<string> copyArgs = origArgs;
    copyArgs.emplace_front("icegridadmin");

    IceInternal::Options opts;
    opts.addOpt("f", "follow");
    opts.addOpt("h", "head", IceInternal::Options::NeedArg);
    opts.addOpt("t", "tail", IceInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        for (const auto& copyArg : copyArgs)
        {
            args.push_back(copyArg);
        }
        args = opts.parse(args);
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        return;
    }

    if (args.size() != 2)
    {
        invalidCommand(reader + " show", "requires two arguments");
        return;
    }

    try
    {
        auto p = args.begin();
        string id = *p++;
        string filename = *p++;

        consoleOut << reader << " '" << id << "' " << filename << ": " << flush;
        Ice::StringSeq lines;

        bool head = opts.isSet("head");
        bool tail = opts.isSet("tail");
        if (head && tail)
        {
            invalidCommand("can't specify both -h | --head and -t | --tail options");
            return;
        }
        if (head && reader == "log")
        {
            invalidCommand("can't specify -h | --head option with log");
            return;
        }

        int lineCount = 20;
        if (head || tail)
        {
            if (head)
            {
                istringstream is(opts.optArg("head"));
                is >> lineCount;
            }
            else
            {
                istringstream is(opts.optArg("tail"));
                is >> lineCount;
            }
            if (lineCount <= 0)
            {
                invalidCommand("invalid argument for -h | --head or -t | --tail option");
                return;
            }
        }

        bool follow = opts.isSet("follow");

        if (head && follow)
        {
            invalidCommand("can't use -f | --follow option with -h | --head option");
            return;
        }

        if (filename == "log")
        {
            showLog(id, reader, tail, follow, lineCount);
        }
        else
        {
            showFile(id, reader, filename, head, tail, follow, lineCount);
        }
    }
    catch (const Ice::Exception&)
    {
        exception(current_exception());
    }
}

void
Parser::showFile(
    const string& id,
    const string& reader,
    const string& filename,
    bool head,
    bool tail,
    bool follow,
    int lineCount)
{
    int maxBytes = _communicator->getProperties()->getIcePropertyAsInt("Ice.MessageSizeMax") * 1024;

    optional<FileIteratorPrx> it;
    try
    {
        if (reader == "node")
        {
            if (filename == "stderr")
            {
                it = _session->openNodeStdErr(id, tail ? lineCount : -1);
            }
            else if (filename == "stdout")
            {
                it = _session->openNodeStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                invalidCommand("invalid node log filename '" + filename + "'");
                return;
            }
        }
        else if (reader == "registry")
        {
            if (filename == "stderr")
            {
                it = _session->openRegistryStdErr(id, tail ? lineCount : -1);
            }
            else if (filename == "stdout")
            {
                it = _session->openRegistryStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                invalidCommand("invalid registry log filename '" + filename + "'");
                return;
            }
        }
        else if (reader == "server")
        {
            if (filename == "stderr")
            {
                it = _session->openServerStdErr(id, tail ? lineCount : -1);
            }
            else if (filename == "stdout")
            {
                it = _session->openServerStdOut(id, tail ? lineCount : -1);
            }
            else
            {
                it = _session->openServerLog(id, filename, tail ? lineCount : -1);
            }
        }

        resetInterrupt();
        Ice::StringSeq lines;
        if (head)
        {
            assert(!follow);

            int i = 0;
            bool eof = false;
            while (!interrupted() && !eof && i < lineCount)
            {
                eof = it->read(maxBytes, lines);
                for (const auto& line : lines)
                {
                    outputNewline();
                    outputString(line);
                    flushOutput();
                }
            }
        }
        else
        {
            bool eof = false;
            while (!interrupted() && !eof)
            {
                eof = it->read(maxBytes, lines);
                for (const auto& line : lines)
                {
                    outputNewline();
                    outputString(line);
                    flushOutput();
                }
            }
        }

        if (follow)
        {
            while (!interrupted())
            {
                bool eof = it->read(maxBytes, lines);
                for (auto p = lines.begin(); p != lines.end(); ++p)
                {
                    outputString(*p);
                    if ((p + 1) != lines.end())
                    {
                        outputNewline();
                    }
                    else
                    {
                        flushOutput();
                    }
                }

                if (eof)
                {
                    unique_lock lock(_mutex);
                    if (_interrupted)
                    {
                        break;
                    }
                    _condVar.wait_for(lock, 5s);
                }
            }
        }

        if (lines.empty() || !lines.back().empty())
        {
            outputNewline();
            flushOutput();
        }

        it->destroy();
    }
    catch (...)
    {
        if (it)
        {
            try
            {
                it->destroy();
            }
            catch (...)
            {
            }
        }
        throw;
    }
}

void
Parser::showLog(const string& id, const string& reader, bool tail, bool follow, int lineCount)
{
    outputNewline();

    optional<Ice::ObjectPrx> admin;

    if (reader == "server")
    {
        admin = _admin->getServerAdmin(id);
    }
    else if (reader == "node")
    {
        admin = _admin->getNodeAdmin(id);
    }
    else if (reader == "registry")
    {
        admin = _admin->getRegistryAdmin(id);
    }

    if (!admin)
    {
        error("cannot retrieve Admin proxy for " + reader + " '" + id + "'");
        return;
    }

    auto loggerAdmin = admin->ice_facet<Ice::LoggerAdminPrx>("Logger");
    if (follow)
    {
        auto adminCallbackTemplate = _session->getAdminCallbackTemplate();

        if (adminCallbackTemplate == nullopt)
        {
            error("cannot retriever Callback template from IceGrid registry");
            return;
        }

        const Ice::EndpointSeq endpoints = adminCallbackTemplate->ice_getEndpoints();
        string publishedEndpoints;

        for (const auto& endpoint : endpoints)
        {
            if (publishedEndpoints.empty())
            {
                publishedEndpoints = endpoint->toString();
            }
            else
            {
                publishedEndpoints += ":" + endpoint->toString();
            }
        }

        _communicator->getProperties()->setProperty("RemoteLoggerAdapter.PublishedEndpoints", publishedEndpoints);

        auto adapter = _communicator->createObjectAdapter("RemoteLoggerAdapter");

        _session->ice_getConnection()->setAdapter(adapter);

        ostringstream os;
        os << "RemoteLogger-" << loggerCallbackCount++;
        Ice::Identity ident = {os.str(), adminCallbackTemplate->ice_getIdentity().category};

        auto servant = make_shared<RemoteLoggerI>();
        auto prx = adapter->add<Ice::RemoteLoggerPrx>(servant, ident);
        adapter->activate();

        loggerAdmin->attachRemoteLogger(prx, Ice::LogMessageTypeSeq(), Ice::StringSeq(), tail ? lineCount : -1);

        resetInterrupt();
        {
            unique_lock lock(_mutex);
            _condVar.wait(lock, [this] { return _interrupted; });
        }

        servant->destroy();
        adapter->destroy();

        try
        {
            loggerAdmin->detachRemoteLogger(prx);
        }
        catch (const Ice::ObjectNotExistException&)
        {
            // ignored
        }
    }
    else
    {
        string prefix;
        const Ice::LogMessageSeq logMessages =
            loggerAdmin->getLog(Ice::LogMessageTypeSeq(), Ice::StringSeq(), tail ? lineCount : -1, prefix);

        for (const auto& logMessage : logMessages)
        {
            printLogMessage(prefix, logMessage);
        }
    }
}

void
Parser::showBanner()
{
    consoleOut << "Ice " << ICE_STRING_VERSION << "  Copyright (c) ZeroC, Inc." << endl;
}

void
Parser::showCopying()
{
    consoleOut << "This command is not implemented." << endl;
}

void
Parser::showWarranty()
{
    consoleOut << "This command is not implemented." << endl;
}

//
// With older flex version <= 2.5.35 YY_INPUT second
// paramenter is of type int&, in newer versions it
// changes to size_t&
//
void
Parser::getInput(char* buf, int& result, size_t maxSize)
{
    auto r = static_cast<size_t>(result);
    getInput(buf, r, maxSize);
    result = static_cast<int>(r);
}

void
Parser::getInput(char* buf, size_t& result, size_t maxSize)
{
    if (!_commands.empty())
    {
        if (_commands == ";")
        {
            result = 0;
        }
        else
        {
            result = min(maxSize, _commands.length());
            strncpy(buf, _commands.c_str(), result);
            _commands.erase(0, result);
            if (_commands.empty())
            {
                _commands = ";";
            }
        }
    }
    else
    {
#if defined(__APPLE__) || defined(__linux__)
        if (isatty(fileno(stdin)) == 1)
        {
            const char* prompt = parser->getPrompt();
            char* line = readline(const_cast<char*>(prompt));
            if (!line)
            {
                result = 0;
            }
            else
            {
                if (*line)
                {
                    add_history(line);
                }

                result = strlen(line) + 1;
                if (result > maxSize)
                {
                    free(line);
                    error("input line too long");
                    result = 0;
                }
                else
                {
                    strcpy(buf, line); // NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    strcat(buf, "\n"); // NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    free(line);
                }
            }
        }
        else
        {
#endif
            consoleOut << parser->getPrompt() << flush;
            string line;
            while (true)
            {
                int c = getc(yyin);
                if (c == EOF)
                {
                    if (line.size())
                    {
                        line += '\n';
                    }
                    break;
                }

                line += static_cast<char>(c);
                if (c == '\n')
                {
                    break;
                }
            }
#ifdef _WIN32
            if (windowsConsoleConverter)
            {
                line = nativeToUTF8(line, windowsConsoleConverter);
            }
#endif
            result = line.length();
            if (result > maxSize)
            {
                error("input line too long");
                buf[0] = EOF;
                result = 1;
            }
            else
            {
                strcpy(buf, line.c_str()); // NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
#if defined(__APPLE__) || defined(__linux__)
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

    if (_continue)
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
    error("'" + command + "' " + msg + "\n('" + command + " help' for more info)");
}

void
Parser::invalidCommand(const list<string>& s)
{
    if (s.empty())
    {
        return;
    }

    string cat = *s.begin();
    if (_helpCommands.find(cat) == _helpCommands.end())
    {
        consoleErr << "unknown '" << cat << "' command (see 'help' for more info)" << endl;
    }
    else if (s.size() == 1)
    {
        consoleErr << "invalid '" << cat << "' command (see '" << cat << " help' for more info)" << endl;
    }
    else
    {
        string cmd = *(++s.begin());
        if (_helpCommands[cat].find(cmd) == _helpCommands[cat].end())
        {
            cmd = cat + " " + cmd;
            consoleErr << "unknown '" << cmd << "' command (see '" << cat << " help' for more info)" << endl;
        }
        else
        {
            cmd = cat + " " + cmd;
            consoleErr << "invalid '" << cmd << "' command (see '" << cmd << " help' for more info)" << endl;
        }
    }
}

void
Parser::error(const char* s)
{
    consoleErr << "error: " << s << endl;
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
    consoleErr << "warning: " << s << endl;
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
    _commands.clear();
    yyin = file;
    assert(yyin);

    _continue = false;

    int status = yyparse();
    if (_errors)
    {
        status = EXIT_FAILURE;
    }

    parser = nullptr;
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
    yyin = nullptr;

    _continue = false;

    int status = yyparse();
    if (_errors)
    {
        status = EXIT_FAILURE;
    }

    parser = nullptr;
    return status;
}

void
Parser::exception(std::exception_ptr pex)
{
    try
    {
        rethrow_exception(pex);
    }
    catch (const ApplicationNotExistException& ex)
    {
        error("couldn't find application '" + ex.name + "'");
    }
    catch (const NodeNotExistException& ex)
    {
        error("couldn't find node '" + ex.name + "'");
    }
    catch (const RegistryNotExistException& ex)
    {
        error("couldn't find registry '" + ex.name + "'");
    }
    catch (const ServerNotExistException& ex)
    {
        error("couldn't find server '" + ex.id + "'");
    }
    catch (const AdapterNotExistException& ex)
    {
        error("couldn't find adapter '" + ex.id + "'");
    }
    catch (const ObjectNotRegisteredException& ex)
    {
        error("couldn't find object '" + _communicator->identityToString(ex.id) + "'");
    }
    catch (const ObjectExistsException& ex)
    {
        error("object '" + _communicator->identityToString(ex.id) + "' already exists");
    }
    catch (const DeploymentException& ex)
    {
        ostringstream s;
        s << ex << ":\n" << ex.reason;
        error(s.str());
    }
    catch (const BadSignalException& ex)
    {
        ostringstream s;
        s << ex.reason;
        error(s.str());
    }
    catch (const NodeUnreachableException& ex)
    {
        error("node '" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch (const RegistryUnreachableException& ex)
    {
        error("registry '" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch (const ServerUnreachableException& ex)
    {
        error("server '" + ex.name + "' couldn't be reached:\n" + ex.reason);
    }
    catch (const AccessDeniedException& ex)
    {
        error("couldn't update the registry, the session from '" + ex.lockUserId + "' is updating the registry");
    }
    catch (const FileNotAvailableException& ex)
    {
        error("couldn't access file:\n" + ex.reason);
    }
    catch (const XMLParserException& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream s;
        s << "couldn't reach the IceGrid registry:\n" << ex;
        error(s.str());
    }
    catch (const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}
