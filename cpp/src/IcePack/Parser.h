// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_PARSER_H
#define ICE_PACK_PARSER_H

#include <IcePack/Admin.h>
#include <list>

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

namespace IcePack
{

class Parser;
typedef ::Ice::Handle<Parser> ParserPtr;

}

namespace IcePack
{

class Parser : public ::Ice::SimpleShared
{
public:

    static ParserPtr createParser(const Ice::CommunicatorPtr&, const IcePack::AdminPrx&);

    void add(const std::list<std::string>&);
    void remove(const std::list<std::string>&);
    void getAll();
    void shutdown();

    void getInput(char*, int, int);
    void nextLine();
    void continueLine();
    char* getPrompt();
    void scanPosition(const char*);

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

private:

    Parser(const Ice::CommunicatorPtr&, const IcePack::AdminPrx&);

    const char* _commands;
    Ice::CommunicatorPtr _communicator;
    IcePack::AdminPrx _admin;
    bool _continue;
    int _errors;
    int _currentLine;
    std::string _currentFile;
};

extern Parser* parser; // The current parser for bison/flex

}

#endif
