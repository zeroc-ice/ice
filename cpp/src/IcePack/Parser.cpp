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
#include <IcePack/Parser.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IcePack;

extern FILE* yyin;

namespace IcePack
{

Parser* parser;

}

ParserPtr
IcePack::Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin)
{
    return new Parser(communicator, admin);
}

void
IcePack::Parser::usage()
{
    cout <<
        "help                        Print this message.\n"
        "exit, quit                  Exit this program.\n"
        "server add NAME PATH LIBRARY_PATH DESCRIPTOR\n"
        "                            Add server NAME with PATH.\n"
	"server describe NAME        Get server NAME description.\n"
	"server state NAME           Get server NAME state.\n"
	"server start NAME           Starts server NAME.\n"
        "server remove NAME          Remove server NAME.\n"
        "server list                 List all server names.\n"
	"adapter add NAME ENDPOINTS  Add adapter NAME with ENDPOINTS.\n"
        "adapter list                List all adapter names.\n"
        "adapter remove NAME         Remove adapter NAME.\n"
	"adapter endpoints NAME      Get adapter NAME endpoints.\n"
        "shutdown                    Shut the IcePack server down.\n";
}

void
IcePack::Parser::addServer(const list<string>& args, const std::list<std::string>& adapters, 
			   const std::list<std::string>& options)
{
    if(args.size() < 2)
    {
	error("`server add' requires at least two arguments (type `help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string path = *p++;
	string ldpath = *p++;
	string descriptor = *p++;

	_admin->addServer(name, path, ldpath, descriptor);

    }
    catch(const Exception& ex)
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
	error("`server start' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	if(!_admin->startServer(args.front()))
	{
	    error("The server didn't start successfully");
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
IcePack::Parser::describeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server describe' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	ServerDescription desc = _admin->getServerDescription(args.front());

	cout << "name = " << desc.name << endl;
	cout << "path = " << desc.path << endl;
	cout << "pwd = " << desc.pwd << endl;

	cout << "args = ";
	copy(desc.args.begin(), desc.args.end(), ostream_iterator<string>(cout," "));
	cout << endl;

	cout << "adapters = ";
	copy(desc.adapters.begin(), desc.adapters.end(), ostream_iterator<string>(cout," "));
	cout << endl;
    }
    catch(const Exception& ex)
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
	error("`server state' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	ServerState state = _admin->getServerState(args.front());

	switch(state)
	{
	case Inactive:
	    cout << "Inactive" << endl;
	    break;
	case Activating:
	    cout << "Activating" << endl;
	    break;
	case Active:
	    cout << "Active" << endl;
	    break;
	case Deactivating:
	    cout << "Deactivating" << endl;
	    break;
	case Destroyed:
	    cout << "Destroyed" << endl;
	    break;
	default:
	    assert(false);
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
IcePack::Parser::removeServer(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`server remove' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_admin->removeServer(args.front());
    }
    catch(const Exception& ex)
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
	ServerNames names = _admin->getAllServerNames();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
IcePack::Parser::addAdapter(const list<string>& args)
{
    if(args.size() < 2)
    {
	error("`adapter add' requires at least two arguments (type `help' for more info)");
	return;
    }

    try
    {
	list<string>::const_iterator p = args.begin();

	string name = *p++;
	string endpoints = *p++;

	_admin->addAdapterWithEndpoints(name, endpoints);
    }
    catch(const Exception& ex)
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
	error("`adapter endpoints' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	string endpoints = _admin->getAdapterEndpoints(args.front());
	cout << endpoints << endl;
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
IcePack::Parser::removeAdapter(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`adapter remove' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_admin->removeAdapter(args.front());
    }
    catch(const Exception& ex)
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
	AdapterNames names = _admin->getAllAdapterNames();
	copy(names.begin(), names.end(), ostream_iterator<string>(cout,"\n"));
    }
    catch(const Exception& ex)
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
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
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

	char* line = readline(parser->getPrompt());
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
    else
    {
	if(((result = fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin))
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

char*
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
    extern int yydebug;
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
    extern int yydebug;
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

IcePack::Parser::Parser(const CommunicatorPtr& communicator, const AdminPrx& admin) :
    _communicator(communicator),
    _admin(admin)
{
}
