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

#define YYSTYPE std::list<std::string>

extern int yynerrs;

int yyparse();
int yylex();

namespace IcePack
{

class Parser;

}

namespace IceInternal
{

void incRef(::IcePack::Parser*);
void decRef(::IcePack::Parser*);

}

namespace IcePack
{

typedef ::IceInternal::Handle<Parser> ParserPtr;

}

namespace IcePack
{

class Parser : public ::IceInternal::SimpleShared
{
public:

    static ParserPtr createParser(const Ice::CommunicatorPtr&, const IcePack::AdminPrx&);

    void add(const std::list<std::string>&);
    void remove(const std::list<std::string>&);
    void getAll();
    void shutdown();

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

    Ice::CommunicatorPtr _communicator;
    IcePack::AdminPrx _admin;
    bool _continue;
    int _currentLine;
    std::string _currentFile;
};

extern Parser* parser; // The current parser for bison/flex

}

#endif
