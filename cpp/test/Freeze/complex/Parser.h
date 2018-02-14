// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PARSER_H
#define PARSER_H

#include <Complex.h>

#ifdef _WIN32
#   include <io.h>
#   ifdef _MSC_VER
#     define isatty _isatty
#     define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#     pragma warning( disable : 4273 )
#   endif
#endif

//
// Stuff for flex and bison
//

#define YYSTYPE Complex::NodePtr
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
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

//
// unput() isn't needed. This prevents the function being defined, and
// the resulting compiler warning.
//
#define YY_NO_UNPUT

class Parser
{
public:

    Parser();
    ~Parser();

    Complex::NodePtr parse(const std::string&, bool = false);

    void error(const char*);
    void error(const std::string&);

    //
    // With older flex version <= 2.5.35 YY_INPUT second 
    // paramenter is of type int&, in newer versions it
    // changes to size_t&
    // 
    void getInput(char*, int&, size_t);
    void getInput(char*, size_t&, size_t);
    void setResult(const Complex::NodePtr&);

private:

    std::string _buf;
    Complex::NodePtr _result;
    int _errors;
};

extern Parser* parser; // Current parser for bison/flex

#endif
