// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/Parser.h>
#include <IceStorm/WeightedGraph.h>
#include <algorithm>

#ifdef GPL_BUILD
#   include <IcePack/GPL.h>
#endif

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
        "link FROM TO COST           Link FROM to TO with the given COST.\n"
        "unlink FROM TO              Unlink TO from FROM.\n"
        "graph DATA COST             Construct the link graph as described in DATA with COST\n"
        "list [TOPICS]               Display information on TOPICS or all topics.\n"
#ifdef GPL_BUILD
	"show copying                Show conditions for redistributing copies of this program.\n"
	"show warranty               Show the warranty for this program.\n"
#endif
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
    try
    {
	for(list<string>::const_iterator i = args.begin(); i != args.end() ; ++i)
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
Parser::link(const list<string>& _args)
{
    list<string> args = _args;

    if(args.size() != 3)
    {
	error("`link' requires exactly three arguments (type `help' for more info)");
	return;
    }

    try
    {    
	TopicPrx fromTopic;
	TopicPrx toTopic;
	
	try
	{
	    fromTopic = _admin->retrieve(args.front());
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
	    toTopic = _admin->retrieve(args.front());
	}
	catch(const IceStorm::NoSuchTopic&)
	{
	    ostringstream s;
	    s << args.front() << ": topic doesn't exist";
	    error(s.str());
	    return;
	}
	args.pop_front();

	Ice::Int cost = atoi(args.front().c_str());
	
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
	    fromTopic = _admin->retrieve(args.front());
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
	    toTopic = _admin->retrieve(args.front());
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
Parser::dolist(const list<string>& _args)
{
    list<string> args = _args;

    try
    {
	if(args.size() == 0)
	{
	    TopicDict d = _admin->retrieveAll();
	    if(!d.empty())
	    {
		for(TopicDict::iterator i = d.begin(); i != d.end(); ++i)
		{
		    if(i != d.begin())
		    {
			cout << ", ";
		    }
		    cout << i->first;
		}
		cout << endl;
	    }
	}
	else
	{
	    while(args.size() != 0)
	    {
		string name = args.front();
		args.pop_front();
		cout << name << endl;
		try
		{
		    TopicPrx topic = _admin->retrieve(name);
		    LinkInfoSeq links = topic->getLinkInfoSeq();
		    for(LinkInfoSeq::const_iterator p = links.begin(); p != links.end(); ++p)
		    {
			cout << "\t" << (*p).name << " with cost " << (*p).cost << endl;
		    }
		}
		catch(const NoSuchTopic&)
		{
		    cout << "\tNo such topic" << endl;
		}
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
Parser::graph(const list<string>& _args)
{
    list<string> args = _args;

    if(args.size() != 2)
    {
	error("`graph' requires exactly two arguments (type `help' for more info)");
	return;
    }

    string file = args.front();
    args.pop_front();
    int maxCost = atoi(args.front().c_str());
    if(maxCost == 0)
    {
	error("`graph': cost must be a positive number");
	return;
    }
    
    try
    {
	WeightedGraph graph;
	if(!graph.parse(file))
	{
	    cerr << file << ": parse failed" << endl;
	    return;
	}
	
	//
	// Compute the new edge set.
	//
	{
	    vector<int> edges;
	    graph.compute(edges, maxCost);
	    graph.swap(edges);
	}

	//
	// Ensure each vertex is present.
	//
	vector<string> vertices = graph.getVertices();
	TopicDict d = _admin->retrieveAll();
	vector<string>::const_iterator p;

	for(p = vertices.begin(); p != vertices.end(); ++p)
	{
	    if(d.find(*p) == d.end())
	    {
		cout << *p << ": referenced topic not found" << endl;
		return;
	    }
	}

	int links = 0;
	int unlinks = 0;

	//
	// Get the edge set for reach vertex.
	//
	for(p = vertices.begin(); p != vertices.end(); ++p)
	{
	    TopicPrx topic = d[*p];
	    assert(topic);
	    LinkInfoSeq seq = topic->getLinkInfoSeq();

	    vector<pair<string, int> > edges = graph.getEdgesFor(*p);
	    for(vector<pair<string, int> >::const_iterator q = edges.begin(); q != edges.end(); ++q)
	    {
		bool link = true;
		for(LinkInfoSeq::iterator r = seq.begin(); r != seq.end(); ++r)
		{
		    //
		    // Found the link element.
		    //
		    if((*r).name == (*q).first)
		    {
			//
			// If the cost is the same, then there is
			// nothing to do.
			//
			if((*r).cost == (*q).second)
			{
			    link = false;
			}
			seq.erase(r);
			break;
		    }
		}

		//
		// Else, need to rebind the link.
		//
		if(link)
		{
		    TopicPrx target = d[(*q).first];
		    ++links;
		    topic->link(target, (*q).second);
		}
	    }

	    //
	    // The remainder of the links are obsolete.
	    //
	    for(LinkInfoSeq::const_iterator r = seq.begin(); r != seq.end(); ++r)
	    {
		++unlinks;
		topic->unlink((*r).theTopic);
	    }
	}
	cout << "graph: " << links << " new or changed links. " << unlinks << " unlinks." << endl;
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::showBanner()
{
    cout << "Ice " << ICE_STRING_VERSION << "  Copyright 2003-2004 ZeroC, Inc." << endl;
#ifdef GPL_BUILD
    cout << gplBanner << endl;
#endif
}

void
Parser::showCopying()
{
#if defined(GPL_BUILD)
    cout << gplCopying << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
}

void
Parser::showWarranty()
{
#if defined(GPL_BUILD)
    cout << gplWarranty << endl;
#else
    cout << "This command is not implemented yet." << endl;
#endif
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

Parser::Parser(const CommunicatorPtr& communicator, const TopicManagerPrx& admin) :
    _communicator(communicator),
    _admin(admin)
{
}
