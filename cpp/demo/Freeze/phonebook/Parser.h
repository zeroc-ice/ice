// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PARSER_H
#define PARSER_H

#include <Ice/Ice.h>
#include <PhoneBook.h>
#include <list>
#include <stdio.h>

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
#define YYMAXDEPTH  10000 // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

class Parser;
typedef ::IceUtil::Handle<Parser> ParserPtr;

class Parser : public ::IceUtil::SimpleShared
{
public:

    Parser(const Demo::PhoneBookPrx&);

    void usage();

    void addContacts(const std::list<std::string>&);
    void findContacts(const std::list<std::string>&);
    void nextFoundContact();
    void printCurrent();
    void setCurrentName(const std::list<std::string>&);
    void setCurrentAddress(const std::list<std::string>&);
    void setCurrentPhone(const std::list<std::string>&);
    void removeCurrent();
    void setEvictorSize(const std::list<std::string>&);
    void shutdown();

    void getInput(char*, int&, int);
    void continueLine();
    const char* getPrompt();

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    int parse(bool = false);

private:


    Demo::Contacts _foundContacts;
    Demo::Contacts::iterator _current;

    const Demo::PhoneBookPrx _phoneBook;
    bool _continue;
    int _errors;
};

extern Parser* parser; // The current parser for bison/flex

#endif
