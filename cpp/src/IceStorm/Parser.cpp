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
#include <IceStorm/Parser.h>

#include <algorithm>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

extern FILE* yyin;

namespace IceStorm
{

Parser* parser;

}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const TopicManagerPrx& admin)
{
    return new Parser(communicator, admin);
}

void
Parser::usage()
{
    cout <<
        "help                        Print this message.\n"
        "exit, quit                  Exit this program.\n"
        "create TOPICS               Add TOPICS.\n"
        "destroy TOPICS              Remove TOPICS.\n"
        "list                        List all server descriptions.\n"
        "shutdown                    Shut the IceStorm server down.\n";
}

#include <IceUtil/Functional.h>
#include <functional>

void
Parser::create(const list<string>& args)
{
    if (args.empty())
    {
	error("`create' requires at least one argument (type `help' for more info)");
	return;
    }

    try
    {
	for (list<string>::const_iterator i = args.begin(); i != args.end() ; ++i)
	{
	    _admin->create(*i);
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
    if (args.empty())
    {
	error("`destroy' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	for (list<string>::const_iterator i = args.begin(); i != args.end() ; ++i)
	{
	    TopicPrx topic = _admin->retrieve(*i);
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
Parser::listAll()
{
    try
    {
	TopicDict d = _admin->retrieveAll();
	if (!d.empty())
	{
	    for (TopicDict::iterator i = d.begin(); i != d.end(); ++i)
	    {
		if (i != d.begin())
		{
		    cout << ", ";
		}
		cout << i->first;
	    }
	    cout << endl;
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
Parser::shutdown()
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
Parser::getInput(char* buf, int& result, int maxSize)
{
    if (!_commands.empty())
    {
	if (_commands == ";")
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
	    if (c == EOF)
	    {
		if (line.size())
		{
		    line += '\n';
		}
		break;
	    }

	    line += c;

	    if (c == '\n')
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
Parser::nextLine()
{
    _currentLine++;
}

void
Parser::continueLine()
{
    _continue = true;
}

char*
Parser::getPrompt()
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
Parser::scanPosition(const char* s)
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
Parser::error(const char* s)
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
Parser::error(const string& s)
{
    error(s.c_str());
}

void
Parser::warning(const char* s)
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
Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
Parser::parse(FILE* file, bool debug)
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
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

int
Parser::parse(const std::string& commands, bool debug)
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
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

Parser::Parser(const CommunicatorPtr& communicator, const TopicManagerPrx& admin) :
    _communicator(communicator),
    _admin(admin)
{
}
