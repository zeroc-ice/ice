// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_PARSER_H
#define ICE_STORM_PARSER_H

#include <IceUtil/Handle.h>
#include <IceStorm/IceStorm.h>
#include <list>
#include <stdio.h>

#ifdef _WIN32
#   include <io.h>
#   ifdef _MSC_VER
#       define isatty _isatty
#       define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#       pragma warning( disable : 4273 )
#   endif
#endif

//
// Stuff for flex and bison
//

#define YYSTYPE std::list<std::string>
#define YY_DECL int yylex(YYSTYPE* yylvalp)
YY_DECL;
int yyparse();

//
// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
//
#define YYMAXDEPTH  20000 // 20000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

namespace IceStorm
{

//
// Forward declaration.
//
class Parser;
typedef ::IceUtil::Handle<Parser> ParserPtr;

class Parser : public ::IceUtil::SimpleShared
{
public:

    static ParserPtr createParser(const Ice::CommunicatorPtr&, const TopicManagerPrx&, 
                                  const std::map<Ice::Identity, TopicManagerPrx>&);

    void usage();

    void create(const std::list<std::string>&);
    void destroy(const std::list<std::string>&);
    void link(const std::list<std::string>&);
    void unlink(const std::list<std::string>&);
    void links(const std::list<std::string>&);
    void topics(const std::list<std::string>&);
    void current(const std::list<std::string>&);

    void showBanner();

    void getInput(char*, int&, int);
    void nextLine();
    void continueLine();
    const char* getPrompt();
    void scanPosition(const char*);

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    void invalidCommand(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

private:

    TopicManagerPrx findManagerById(const std::string&, std::string&) const;
    TopicManagerPrx findManagerByCategory(const std::string&) const;
    TopicPrx findTopic(const std::string&) const;

    Parser(const Ice::CommunicatorPtr&, const TopicManagerPrx&, const std::map<Ice::Identity, TopicManagerPrx>&);
    
    void exception(const Ice::Exception&, bool = false);

    const Ice::CommunicatorPtr _communicator;
    TopicManagerPrx _defaultManager;
    const std::map<Ice::Identity, TopicManagerPrx> _managers;
    std::string _commands;
    bool _continue;
    int _errors;
    int _currentLine;
    std::string _currentFile;
};

extern Parser* parser; // The current parser for bison/flex

} // End namespace IceStorm

#endif
