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
#include <Parser.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

using namespace std;
using namespace Ice;

extern FILE* yyin;

Parser* parser;

void
Parser::usage()
{
    cout <<
        "help             Print this message.\n"
        "exit, quit       Exit this program.\n"
        "add NAMES...     Create new entries for NAMES in the phonebook.\n"
        "find NAME        Find all entries in the phonebook that match NAME.\n"
        "                 Set the current entry to the first one found.\n"
        "next             Set the current entry to the next one that was found.\n"
        "current          Display the current entry.\n"
        "name NAME        Set the name for the current entry to NAME.\n"
        "address ADDRESS  Set the address for the current entry to ADDRESS.\n"
        "phone PHONE      Set the phone number for the current entry to PHONE.\n"
        "remove           Permanently remove the current entry from the phonebook.\n"
        "list             List all names in the phonebook.\n"
        "shutdown         Shut the phonebook server down.\n";
}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const PhoneBookPrx& phoneBook)
{
    return new Parser(communicator, phoneBook);
}

void
Parser::addEntries(const std::list<std::string>& args)
{
    if (args.empty())
    {
	error("`add' requires at least one rgument (type `help' for more info)");
	return;
    }

    try
    {
	for(list<string>::const_iterator p = args.begin(); p != args.end(); ++p)
	{
	    EntryPrx entry = _phoneBook->createEntry();
	    entry -> setName(*p);
	    cout << "added new entry for `" << *p << "'" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::findEntries(const std::list<std::string>& args)
{
    if (args.size() != 1)
    {
	error("`find' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_foundEntries = _phoneBook->findEntries(args.front());
	_current = _foundEntries.begin();
	cout << "number of entries found: " << _foundEntries.size() << endl;
	printCurrent();
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::nextFoundEntry()
{
    if (_current != _foundEntries.end())
    {
	++_current;
    }
    printCurrent();
}

void
Parser::printCurrent()
{
    try
    {
	if (_current != _foundEntries.end())
	{
	    cout << "current entry is:" << endl;
	    cout << "name: " << (*_current)->getName() << endl;
	    cout << "address: " << (*_current)->getAddress() << endl;
	    cout << "phone: " << (*_current)->getPhone() << endl;
	}
	else
	{
	    cout << "no current entry" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::setCurrentName(const std::list<std::string>& args)
{
    if (args.size() != 1)
    {
	error("`name' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	if (_current != _foundEntries.end())
	{
	    (*_current)->setName(args.front());
	    cout << "changed name to `" << args.front() << "'" << endl;
	}
	else
	{
	    cout << "no current entry" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::setCurrentAddress(const std::list<std::string>& args)
{
    if (args.size() != 1)
    {
	error("`address' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	if (_current != _foundEntries.end())
	{
	    (*_current)->setAddress(args.front());
	    cout << "changed address to `" << args.front() << "'" << endl;
	}
	else
	{
	    cout << "no current entry" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::setCurrentPhone(const std::list<std::string>& args)
{
    if (args.size() != 1)
    {
	error("`phone' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	if (_current != _foundEntries.end())
	{
	    (*_current)->setPhone(args.front());
	    cout << "changed phone number to `" << args.front() << "'" << endl;
	}
	else
	{
	    cout << "no current entry" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::removeCurrent()
{
    try
    {
	if (_current != _foundEntries.end())
	{
	    (*_current)->destroy();
	    cout << "removed current entry" << endl;
	}
	else
	{
	    cout << "no current entry" << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::listNames()
{
    try
    {
	Names names = _phoneBook->getAllNames();
	for (Names::iterator p = names.begin(); p != names.end(); ++p)
	{
	    cout << *p << endl;
	}
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
    }
}

void
Parser::shutdown()
{
    try
    {
	_phoneBook->shutdown();
    }
    catch(const LocalException& ex)
    {
	error(ex.toString());
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
#ifdef WIN32
	    // COMPILERBUG: Stupid Visual C++ defined min and max as macros
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

    _currentFile = "<standard input>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    _foundEntries.clear();
    _current = _foundEntries.end();

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

    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    _foundEntries.clear();
    _current = _foundEntries.end();

    int status = yyparse();
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

Parser::Parser(const CommunicatorPtr& communicator, const PhoneBookPrx& phoneBook) :
    _communicator(communicator),
    _phoneBook(phoneBook)
{
}
