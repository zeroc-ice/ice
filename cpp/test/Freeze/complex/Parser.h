// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PARSER_H
#define PARSER_H

#include <Complex.h>

#ifdef WIN32
#   include <io.h>
#   define isatty _isatty
#   define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#   pragma warning( disable : 4273 )
#endif

//
// Stuff for flex and bison
//

#define YYSTYPE Complex::NodePtr
#define YY_DECL int yylex(YYSTYPE* yylvalp)
YY_DECL;
int yyparse();

class Parser
{
public:

    Parser();
    ~Parser();

    Complex::NodePtr parse(const std::string&, bool = false);

    void error(const char*);
    void error(const std::string&);

    void getInput(char*, int&, int);
    void setResult(const Complex::NodePtr&);

private:

    std::string _buf;
    Complex::NodePtr _result;
    int _errors;
};

extern Parser* parser; // Current parser for bison/flex

#endif
