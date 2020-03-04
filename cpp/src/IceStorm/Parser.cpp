//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <IceStorm/Parser.h>
#include <IceStorm/IceStormInternal.h>
#include <algorithm>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

extern FILE* yyin;
extern int yydebug;

int yyparse();

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceStorm;

namespace IceStorm
{

Parser* parser;

#ifdef _WIN32
shared_ptr<Ice::StringConverter> windowsConsoleConverter = nullptr;
#endif

}

namespace
{

class UnknownManagerException : public std::exception
{
public:

    explicit UnknownManagerException(const std::string& name) : _name(name)
    {
    }

    const char* what() const noexcept override
    {
        return _name.c_str();
    }

private:
    const string _name;
};

}

Parser::Parser(shared_ptr<Communicator> communicator, shared_ptr<TopicManagerPrx> admin,
               map<Ice::Identity, shared_ptr<TopicManagerPrx>> managers) :
    _communicator(move(communicator)),
    _defaultManager(move(admin)),
    _managers(move(managers))
{
#ifdef _WIN32
    if(!windowsConsoleConverter)
    {
        windowsConsoleConverter = Ice::createWindowsStringConverter(GetConsoleOutputCP());
    }
#endif
}

void
Parser::usage()
{
    consoleOut <<
        "help                     Print this message.\n"
        "exit, quit               Exit this program.\n"
        "create TOPICS            Add TOPICS.\n"
        "destroy TOPICS           Remove TOPICS.\n"
        "link FROM TO [COST]      Link FROM to TO with the optional given COST.\n"
        "unlink FROM TO           Unlink TO from FROM.\n"
        "links [INSTANCE-NAME]    Display all links for the topics in the current topic\n"
        "                         manager, or in the given INSTANCE-NAME.\n"
        "topics [INSTANCE-NAME]   Display the names of all topics in the current topic\n"
        "                         manager, or in the given INSTANCE-NAME.\n"
        "current [INSTANCE-NAME]  Display the current topic manager, or change it to\n"
        "                         INSTANCE-NAME.\n"
        "replica [INSTANCE-NAME]  Display replication information for the given INSTANCE-NAME.\n"
        "subscribers TOPICS       List TOPICS subscribers.\n"
        ;
}

void
Parser::create(const list<string>& args)
{
    if(args.empty())
    {
        error("`create' requires at least one argument (type `help' for more info)");
        return;
    }

    for(const auto& arg : args)
    {
        try
        {
            string topicName;
            auto manager = findManagerById(arg, topicName);
            manager->create(topicName);
        }
        catch(const std::exception&)
        {
            exception(current_exception(), args.size() > 1); // Print a warning if we're creating multiple topics, an error otherwise.
        }
    }
}

void
Parser::destroy(const list<string>& args)
{
    if(args.empty())
    {
        error("`destroy' requires at least one argument (type `help' for more info)");
        return;
    }

    for(const auto& arg : args)
    {
        try
        {
            findTopic(arg)->destroy();
        }
        catch(const std::exception&)
        {
            exception(current_exception(), args.size() > 1); // Print a warning if we're destroying multiple topics, an error otherwise.
        }
    }
}

void
Parser::link(const list<string>& args)
{
    if(args.size() != 2 && args.size() != 3)
    {
        error("`link' requires two or three arguments (type `help' for more info)");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();

        auto fromTopic = findTopic(*p++);
        auto toTopic = findTopic(*p++);
        auto cost = p != args.end() ? atoi(p->c_str()) : 0;

        fromTopic->link(toTopic, cost);
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::unlink(const list<string>& args)
{
    if(args.size() != 2)
    {
        error("`unlink' requires exactly two arguments (type `help' for more info)");
        return;
    }

    try
    {
        list<string>::const_iterator p = args.begin();

        auto fromTopic = findTopic(*p++);
        auto toTopic = findTopic(*p++);

        fromTopic->unlink(toTopic);
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::links(const list<string>& args)
{
    if(args.size() > 1)
    {
        error("`links' requires at most one argument (type `help' for more info)");
        return;
    }

    try
    {
        shared_ptr<TopicManagerPrx> manager;
        if(args.size() == 0)
        {
            manager = _defaultManager;
        }
        else
        {
            manager = findManagerByCategory(args.front());
        }

        for(const auto& topic : manager->retrieveAll())
        {
            for(const auto& linkInfo : topic.second->getLinkInfoSeq())
            {
                consoleOut << topic.first << " to " << linkInfo.name << " with cost " << linkInfo.cost << endl;
            }
        }
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::topics(const list<string>& args)
{
    if(args.size() > 1)
    {
        error("`topics' requires at most one argument (type `help' for more info)");
        return;
    }

    try
    {
        shared_ptr<TopicManagerPrx> manager;
        if(args.size() == 0)
        {
            manager = _defaultManager;
        }
        else
        {
            manager = findManagerByCategory(args.front());
        }

        for(const auto& topic : manager->retrieveAll())
        {
            consoleOut << topic.first << endl;
        }
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::replica(const list<string>& args)
{
    if(args.size() > 1)
    {
        error("`replica' requires at most one argument (type `help' for more info)");
        return;
    }

    try
    {
        shared_ptr<TopicManagerPrx> m;
        if(args.size() == 0)
        {
            m = _defaultManager;
        }
        else
        {
            m = findManagerByCategory(args.front());
        }
        auto manager = Ice::uncheckedCast<TopicManagerInternalPrx>(m);
        auto node = manager->getReplicaNode();
        if(!node)
        {
            error("This topic is not replicated");
        }
        auto nodes = node->nodes();
        consoleOut << "replica count: " << nodes.size() << endl;
        for(const auto& n : nodes)
        {
            try
            {
                auto info = n.n->query();
                consoleOut << n.id << ": id:         " << info.id << endl;
                consoleOut << n.id << ": coord:      " << info.coord << endl;
                consoleOut << n.id << ": group name: " << info.group << endl;
                consoleOut << n.id << ": state:      ";
                switch(info.state)
                {
                case IceStormElection::NodeState::NodeStateInactive:
                    consoleOut << "inactive";
                    break;
                case IceStormElection::NodeState::NodeStateElection:
                    consoleOut << "election";
                    break;
                case IceStormElection::NodeState::NodeStateReorganization:
                    consoleOut << "reorganization";
                    break;
                case IceStormElection::NodeState::NodeStateNormal:
                    consoleOut << "normal";
                    break;
                default:
                    consoleOut << "unknown";
                }
                consoleOut << endl;
                consoleOut << n.id << ": group:      ";
                for(auto q = info.up.cbegin(); q != info.up.cend(); ++q)
                {
                    if(q != info.up.cbegin())
                    {
                        consoleOut << ",";
                    }
                    consoleOut << q->id;
                }
                consoleOut << endl;
                consoleOut << n.id << ": max:        " << info.max
                           << endl;
            }
            catch(const Exception& ex)
            {
                consoleOut << n.id << ": " << ex.ice_id() << endl;
            }
        }
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::subscribers(const list<string>& args)
{
    if(args.empty())
    {
        error("subscribers' requires at least one argument (type `help' for more info) ");
        return;
    }
    try
    {
        for(const auto& arg : args)
        {
            auto topic = _defaultManager->retrieve(arg);
            consoleOut << arg << ": subscribers:" << endl;
            for(const auto& subscriber : topic->getSubscribers())
            {
                consoleOut << "\t" << _communicator->identityToString(subscriber) << endl;
            }
        }
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::current(const list<string>& args)
{
    if(args.empty())
    {
        consoleOut << _communicator->identityToString(_defaultManager->ice_getIdentity()) << endl;
        return;
    }
    else if(args.size() > 1)
    {
        error("`current' requires at most one argument (type `help' for more info)");
        return;
    }

    try
    {
        auto manager = findManagerByCategory(args.front());
        manager->ice_ping();
        _defaultManager = manager;
    }
    catch(const std::exception&)
    {
        exception(current_exception());
    }
}

void
Parser::showBanner()
{
    consoleOut << "Ice " << ICE_STRING_VERSION << "  Copyright (c) ZeroC, Inc." << endl;
}

//
// With older flex version <= 2.5.35 YY_INPUT second
// paramenter is of type int&, in newer versions it
// changes to size_t&
//
void
Parser::getInput(char* buf, int& result, size_t maxSize)
{
    size_t r = static_cast<size_t>(result);
    getInput(buf, r, maxSize);
    result = static_cast<int>(r);
}

void
Parser::getInput(char* buf, size_t& result, size_t maxSize)
{
    if(!_commands.empty())
    {
        if(_commands == ";")
        {
            result = 0;
        }
        else
        {
            result = min(maxSize, _commands.length());
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

        consoleOut << parser->getPrompt() << flush;

        string line;
        while(true)
        {
            int c = getc(yyin);
            if(c == EOF)
            {
                if(line.size())
                {
                    line += '\n';
                }
                break;
            }

            line += static_cast<char>(c);
            if(c == '\n')
            {
                break;
            }
        }
#ifdef _WIN32
        if(windowsConsoleConverter)
        {
            line = nativeToUTF8(line, windowsConsoleConverter);
        }
#endif
        result = line.length();
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

void
Parser::invalidCommand(const string& s)
{
    consoleErr << s << endl;
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

shared_ptr<TopicManagerPrx>
Parser::findManagerById(const string& full, string& arg) const
{
    auto id = Ice::stringToIdentity(full);
    arg = id.name;
    if(id.category.empty())
    {
        return _defaultManager;
    }
    id.name = "TopicManager";
    auto p = _managers.find(id);
    if(p == _managers.end())
    {
        throw UnknownManagerException(id.category);
    }
    return p->second;
}

shared_ptr<TopicManagerPrx>
Parser::findManagerByCategory(const string& full) const
{
    Ice::Identity id = {"TopicManager", full};
    auto p = _managers.find(id);
    if(p == _managers.end())
    {
        throw UnknownManagerException(id.category);
    }
    return p->second;
}

shared_ptr<TopicPrx>
Parser::findTopic(const string& full) const
{
    string topicName;
    auto manager = findManagerById(full, topicName);
    return manager->retrieve(topicName);
}

void
Parser::exception(exception_ptr pex, bool warn)
{
    ostringstream os;
    try
    {
        rethrow_exception(pex);
    }
    catch(const LinkExists& ex)
    {
        os << "link `" << ex.name << "' already exists";
    }
    catch(const NoSuchLink& ex)
    {
        os << "couldn't find link `" << ex.name << "'";
    }
    catch(const TopicExists& ex)
    {
        os << "topic `" << ex.name << "' exists";
    }
    catch(const NoSuchTopic& ex)
    {
        os << "couldn't find topic `" << ex.name << "'";
    }
    catch(const UnknownManagerException& ex)
    {
        os << "couldn't find IceStorm service `" << ex.what() << "'";
    }
    catch(const IdentityParseException& ex)
    {
        os << "invalid identity `" << ex.str << "'";
    }
    catch(const Ice::LocalException& ex)
    {
        os << "couldn't reach IceStorm service:\n" << ex;
    }
    catch(const Ice::Exception& ex)
    {
        os << ex;
    }
    catch(const std::exception& ex)
    {
        os << ex.what();
    }

    if(warn)
    {
        warning(os.str());
    }
    else
    {
        error(os.str());
    }
}
