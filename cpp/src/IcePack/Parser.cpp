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

using namespace std;
using namespace Ice;
using namespace IcePack;

extern FILE* yyin;
extern const char* yycommands;

namespace IcePack
{

Parser* parser;

}

void IceInternal::incRef(Parser* p) { p->__incRef(); }
void IceInternal::decRef(Parser* p) { p->__decRef(); }

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

ParserPtr
IcePack::Parser::createParser(const CommunicatorPtr& communicator, const AdminPrx& admin)
{
    return new Parser(communicator, admin);
}

void
IcePack::Parser::add(const list<string>& references)
{
    for (list<string>::const_iterator p = references.begin(); p != references.end(); ++p)
    {
	try
	{
	    ServerDescriptionPtr desc = new ServerDescription;
	    desc->object = _communicator->stringToProxy(*p);
	    _admin->add(desc);
	}
	catch(const LocalException& ex)
	{
	    error(ex.toString());
	}
    }
}

void
IcePack::Parser::remove(const list<string>& references)
{
    for (list<string>::const_iterator p = references.begin(); p != references.end(); ++p)
    {
	try
	{
	    _admin->remove(_communicator->stringToProxy(*p));
	}
	catch(const LocalException& ex)
	{
	    error(ex.toString());
	}
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
    if (!yycommands && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ' ';
    }
    cerr << "error: " << s << endl;
    yynerrs++;
}

void
IcePack::Parser::error(const string& s)
{
    error(s.c_str());
}

void
IcePack::Parser::warning(const char* s)
{
    if (!yycommands && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ' ';
    }
    cerr << "warning: " << s << endl;
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

    _currentFile = "<standard input>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    yyin = file;
    yycommands = 0;
    int status = yyparse();

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

    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    yyin = 0;
    yycommands = commands.c_str();
    int status = yyparse();

    parser = 0;
    return status;
}

IcePack::Parser::Parser(const CommunicatorPtr& communicator, const AdminPrx& admin) :
    _communicator(communicator),
    _admin(admin)
{
}
