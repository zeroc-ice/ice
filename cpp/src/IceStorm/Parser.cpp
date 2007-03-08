// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/Ice.h>
#include <IceStorm/Parser.h>
#include <algorithm>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

extern FILE* yyin;
extern int yydebug;

using namespace std;
using namespace Ice;
using namespace IceStorm;

namespace IceStorm
{

Parser* parser;

}

namespace
{

class UnknownManagerException : public Exception
{
public:
    
    UnknownManagerException(const string& name, const char* file, int line) :
        Exception(file, line),
        name(name)
    {
    }

    virtual
    ~UnknownManagerException() throw()
    {
    }
    virtual string
    ice_name() const
    {
        return "UnknownManagerException";
    }
    virtual Exception*
    ice_clone() const
    {
        return new UnknownManagerException(*this);
    }
    
    virtual void
    ice_throw() const
    {
        throw *this;
    }
    const string name;
};

}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const TopicManagerPrx& admin,
                     const map<Ice::Identity, IceStorm::TopicManagerPrx>& managers)
{
    return new Parser(communicator, admin, managers);
}

void
Parser::usage()
{
    cout <<
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
        ;
}

void
Parser::create(const list<string>& args)
{
    if(args.size() == 0)
    {
        error("`create' requires an argument (type `help' for more info)");
        return;
    }

    try
    {
        for(list<string>::const_iterator i = args.begin(); i != args.end() ; ++i)
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(*i, arg);
            manager->create(arg);
        }
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::destroy(const list<string>& args)
{
    try
    {
        for(list<string>::const_iterator i = args.begin(); i != args.end() ; ++i)
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(*i, arg);
            TopicPrx topic = manager->retrieve(arg);
            topic->destroy();
        }
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::link(const list<string>& _args)
{
    list<string> args = _args;

    if(args.size() < 2)
    {
        error("`link' requires at least two arguments (type `help' for more info)");
        return;
    }

    try
    {    
        TopicPrx fromTopic;
        TopicPrx toTopic;
        
        try
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(args.front(), arg);
            fromTopic = manager->retrieve(arg);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            ostringstream s;
            s << args.front() << ": topic doesn't exist";
            error(s.str());
            return;
        }
        args.pop_front();
        
        try
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(args.front(), arg);
            toTopic = manager->retrieve(arg);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            ostringstream s;
            s << args.front() << ": topic doesn't exist";
            error(s.str());
            return;
        }
        args.pop_front();
        Ice::Int cost = 0;
        if(!args.empty())
        {
            cost = atoi(args.front().c_str());
        }
        fromTopic->link(toTopic, cost);
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::unlink(const list<string>& _args)
{
    list<string> args = _args;

    if(args.size() != 2)
    {
        error("`unlink' requires exactly two arguments (type `help' for more info)");
        return;
    }

    try
    {    
        TopicPrx fromTopic;
        TopicPrx toTopic;
        
        try
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(args.front(), arg);
            fromTopic = manager->retrieve(arg);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            ostringstream s;
            s << args.front() << ": topic doesn't exist";
            error(s.str());
            return;
        }
        args.pop_front();
        
        try
        {
            string arg;
            IceStorm::TopicManagerPrx manager = findManagerById(args.front(), arg);
            toTopic = manager->retrieve(arg);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            ostringstream s;
            s << args.front() << ": topic doesn't exist";
            error(s.str());
            return;
        }

        fromTopic->unlink(toTopic);
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
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
        IceStorm::TopicManagerPrx manager;
        if(args.size() == 0)
        {
            manager = _defaultManager;
        }
        else
        {
            manager = findManagerByCategory(args.front());
        }
        TopicDict d = manager->retrieveAll();
        for(TopicDict::iterator i = d.begin(); i != d.end(); ++i)
        {
            LinkInfoSeq links = i->second->getLinkInfoSeq();
            for(LinkInfoSeq::const_iterator p = links.begin(); p != links.end(); ++p)
            {
                cout << i->first << " to " << (*p).name << " with cost " << (*p).cost << endl;
            }
        }
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
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
        IceStorm::TopicManagerPrx manager;
        if(args.size() == 0)
        {
            manager = _defaultManager;
        }
        else
        {
            manager = findManagerByCategory(args.front());
        }
        TopicDict d = manager->retrieveAll();
        for(TopicDict::iterator i = d.begin(); i != d.end(); ++i)
        {
            cout << i->first << endl;
        }
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::current(const list<string>& _args)
{
    list<string> args = _args;

    if(args.size() == 0)
    {
        cout << _communicator->identityToString(_defaultManager->ice_getIdentity()) << endl;
        return;
    }

    try
    {
        IceStorm::TopicManagerPrx manager = findManagerByCategory(args.front());
        manager->ice_ping();
        _defaultManager = manager;
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << args.front() << ": " << ex;
        error(s.str());
        return;
    }
}

void
Parser::showBanner()
{
    cout << "Ice " << ICE_STRING_VERSION << "  Copyright 2003-2007 ZeroC, Inc." << endl;
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

IceStorm::TopicManagerPrx
Parser::findManagerById(const string& full, string& arg) const
{
    Ice::Identity id = _communicator->stringToIdentity(full);
    arg = id.name;
    if(id.category.empty())
    {
        return _defaultManager;
    }
   id.name = "TopicManager";
    map<Ice::Identity, IceStorm::TopicManagerPrx>::const_iterator p = _managers.find(id);
    if(p == _managers.end())
    {
        throw UnknownManagerException(id.category, __FILE__, __LINE__);
    }
    return p->second;
}

IceStorm::TopicManagerPrx
Parser::findManagerByCategory(const string& full) const
{
    Ice::Identity id;
    id.category = full;
    id.name = "TopicManager";
    map<Ice::Identity, IceStorm::TopicManagerPrx>::const_iterator p = _managers.find(id);
    if(p == _managers.end())
    {
        throw UnknownManagerException(id.category, __FILE__, __LINE__);
    }
    return p->second;
}

Parser::Parser(const CommunicatorPtr& communicator, const TopicManagerPrx& admin,
               const map<Ice::Identity, IceStorm::TopicManagerPrx>& managers) :
    _communicator(communicator),
    _defaultManager(admin),
    _managers(managers)
{
}
