// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PACK_PARSER_H
#define ICE_PACK_PARSER_H

#include <IceUtil/Handle.h>
#include <IcePack/Admin.h>
#include <IcePack/Query.h>
#include <list>

#ifdef _WIN32
#   include <io.h>
#   define isatty _isatty
#   define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#   pragma warning( disable : 4273 )
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

namespace IcePack
{

class Parser;
typedef ::IceUtil::Handle<Parser> ParserPtr;

}

namespace IcePack
{

class Parser : public ::IceUtil::SimpleShared
{
public:

    static ParserPtr createParser(const Ice::CommunicatorPtr&, const IcePack::AdminPrx&, const IcePack::QueryPrx&);

    void usage();

    void addApplication(const std::list<std::string>&);
    void removeApplication(const std::list<std::string>&);

    void pingNode(const std::list<std::string>&);
    void shutdownNode(const std::list<std::string>&);
    void listAllNodes();

    void addServer(const std::list<std::string>&);
    void startServer(const std::list<std::string>&);
    void stopServer(const std::list<std::string>&);
    void signalServer(const std::list<std::string>&);
    void writeMessage(const std::list<std::string>&, int fd);
    void describeServer(const std::list<std::string>&);
    void stateServer(const std::list<std::string>&);
    void activationServer(const std::list<std::string>&);
    void pidServer(const std::list<std::string>&);
    void removeServer(const std::list<std::string>&);
    void listAllServers();

    void endpointsAdapter(const std::list<std::string>&);
    void listAllAdapters();

    void addObject(const std::list<std::string>&);
    void removeObject(const std::list<std::string>&);
    void findObject(const std::list<std::string>&);

    void shutdown();

    void getInput(char*, int&, int);
    void nextLine();
    void continueLine();
    const char* getPrompt();
    void scanPosition(const char*);

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

private:

    Parser(const Ice::CommunicatorPtr&, const IcePack::AdminPrx&, const IcePack::QueryPrx&);

    std::string _commands;
    Ice::CommunicatorPtr _communicator;
    IcePack::AdminPrx _admin;
    IcePack::QueryPrx _query;
    bool _continue;
    int _errors;
    int _currentLine;
    std::string _currentFile;
};

extern Parser* parser; // The current parser for bison/flex

}

#endif
