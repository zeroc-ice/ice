// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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

using namespace std;
using namespace Ice;
using namespace IcePack;

extern FILE* yyin;

namespace IcePack
{

Parser* parser;

}

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

ParserPtr
IcePack::Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin)
{
    return new Parser(communicator, admin);
}

void
IcePack::Parser::add(const list<string>& args)
{
    if (args.empty())
    {
	error("`add' requires at least an object reference as argument");
	return;
    }

    try
    {
	ServerDescriptionPtr desc = new ServerDescription;
	list<string>::const_iterator p = args.begin();
	desc->object = _communicator->stringToProxy(*p);
	desc->regex = false;
	if (++p != args.end())
	{
	    desc->path = *p;
	    while (++p != args.end())
	    {
		desc->args.push_back(*p);
	    }
	}
	_admin->add(desc);

    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
IcePack::Parser::remove(const list<string>& args)
{
    if (args.size() != 1)
    {
	error("`remove' takes exactly one object reference as argument");
	return;
    }

    try
    {
	_admin->remove(args.front());
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
IcePack::Parser::getAll()
{
    try
    {
	ServerDescriptions descriptions = _admin->getAll();
	ServerDescriptions::iterator p = descriptions.begin();
	while(p != descriptions.end())
	{
	    cout << "object = " << _communicator->proxyToString((*p)->object) << endl;
	    cout << "regex = " << boolalpha << (*p)->regex << endl;
	    cout << "host = " << (*p)->host << endl;
	    cout << "path = " << (*p)->path << endl;
	    cout << "args =";
	    for (Args::iterator q = (*p)->args.begin(); q != (*p)->args.end(); ++q)
	    {
		cout << ' ' << *q;
	    }
	    cout << endl;
	    if (++p != descriptions.end())
	    {
		cout << endl;
	    }
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
IcePack::Parser::shutdown()
{
    try
    {
	_admin->shutdown();
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
IcePack::Parser::getInput(char* buf, int& result, int maxSize)
{
    if (!_commands.empty())
    {
	if (_commands == ";")
	{
	    buf[0] = EOF;
	    result = 1;
	}
	else
	{
	    // COMPILERBUG: Stupid Visual C++ defined min and max as macros
	    result = _MIN(maxSize, static_cast<int>(_commands.length()));
	    strncpy(buf, _commands.c_str(), result);
	    _commands.erase(0, result);
	    if (_commands.empty())
	    {
		_commands = ";";
	    }
	}
    }
    else if (isatty(fileno(yyin)))
    {
#ifdef HAVE_READLINE

	char* line = readline(parser->getPrompt());
	if (line && *line)
	{
	    add_history(line);
	}
	if (!line)
	{
	    buf[0] = EOF;
	    result = 1;
	}
	else
	{
	    result = strlen(line) + 1;
	    if (result > maxSize)
	    {
		free(line);
		error("input line too long");
		buf[0] = EOF;
		result = 1;
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
	while (true)
	{
	    char c = static_cast<char>(getc(yyin));
	    line += c;
	    if (c == '\n' || c == EOF)
	    {
		break;
	    }
	}
	
	result = line.length();
	if (result > maxSize)
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
	if (((result = fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin))
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
IcePack::Parser::scanPosition(const char* s)
{
    string line(s);
    string::size_type idx;

    idx = line.find("line");
    if (idx != string::npos)
    {
	line.erase(0, idx + 4);
    }

    idx = line.find_first_not_of(" \t\r#");
    if (idx != string::npos)
    {
	line.erase(0, idx);
    }

    _currentLine = atoi(line.c_str()) - 1;

    idx = line.find_first_of(" \t\r");
    if (idx != string::npos)
    {
	line.erase(0, idx);
    }

    idx = line.find_first_not_of(" \t\r\"");
    if (idx != string::npos)
    {
	line.erase(0, idx);

	idx = line.find_first_of(" \t\r\"");
	if (idx != string::npos)
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
    if (_commands.empty() && !isatty(fileno(yyin)))
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
    if (_commands.empty() && !isatty(fileno(yyin)))
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

    _currentFile = "<standard input>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    int status = yyparse();
    if (_errors)
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

    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    int status = yyparse();
    if (_errors)
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
