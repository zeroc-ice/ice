// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Yellow/Parser.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#include <iterator>

using namespace std;
using namespace Ice;
using namespace Yellow;

extern FILE* yyin;

namespace Yellow
{

Parser* parser;

}

ParserPtr
Yellow::Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query)
{
    return new Parser(communicator, admin, query);
}

void
Yellow::Parser::usage()
{
    cout <<
        "help                        Print this message.\n"
        "exit, quit                  Exit this program.\n"
        "add interface proxy         Add this offer.\n"
        "remove interface proxy      Remove this offer.\n"
        "lookup interface            Lookup an offer for an interface.\n"
	"lookupAll interface         Retrieve all offers for an interface.\n";
}

void
Yellow::Parser::addOffer(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`add' requires exactly two arguments (type `help' for more info)");
	return;
    }
    string intf = args.front();
    string offer = args.back();

    _admin->add(intf, _communicator->stringToProxy(offer));
}

void
Yellow::Parser::removeOffer(const list<string>& args)
{
    if(args.size() != 2)
    {
	error("`remove' requires exactly two arguments (type `help' for more info)");
	return;
    }
    string intf = args.front();
    string offer = args.back();

    try
    {
	_admin->remove(intf, _communicator->stringToProxy(offer));
    }
    catch(const NoSuchOfferException&)
    {
	cerr << intf << ": no offer" << endl;
    }
}

void
Yellow::Parser::lookup(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`lookup' requires exactly one argument (type `help' for more info)");
	return;
    }
    string intf = args.front();

    try
    {
	Ice::ObjectPrx offer = _query->lookup(intf);
	cout << _communicator->proxyToString(offer) << endl;
    }
    catch(const NoSuchOfferException&)
    {
	cerr << intf << ": no offer" << endl;
    }
}

void
Yellow::Parser::lookupAll(const list<string>& args)
{
    if(args.size() != 1)
    {
	error("`lookup' requires exactly one argument (type `help' for more info)");
	return;
    }
    string intf = args.front();

    try
    {
	Ice::ObjectProxySeq offers = _query->lookupAll(intf);
	cout << offers.size() << " offers" << endl;
	for (Ice::ObjectProxySeq::const_iterator p = offers.begin(); p != offers.end(); ++p)
	{
	    cout << "\t" << _communicator->proxyToString(*p) << endl;
	}
    }
    catch(const NoSuchOfferException&)
    {
	cerr << intf << ": no offer" << endl;
    }
}

void
Yellow::Parser::getInput(char* buf, int& result, int maxSize)
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
Yellow::Parser::nextLine()
{
    _currentLine++;
}

void
Yellow::Parser::continueLine()
{
    _continue = true;
}

char*
Yellow::Parser::getPrompt()
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
Yellow::Parser::scanPosition(const char* s)
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
Yellow::Parser::error(const char* s)
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
Yellow::Parser::error(const string& s)
{
    error(s.c_str());
}

void
Yellow::Parser::warning(const char* s)
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
Yellow::Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
Yellow::Parser::parse(FILE* file, bool debug)
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
Yellow::Parser::parse(const std::string& commands, bool debug)
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

Yellow::Parser::Parser(const CommunicatorPtr& communicator, const AdminPrx& admin, const QueryPrx& query) :
    _communicator(communicator),
    _admin(admin),
    _query(query)
{
}
