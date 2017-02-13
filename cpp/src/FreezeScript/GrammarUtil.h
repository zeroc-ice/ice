// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_GRAMMAR_UTIL_H
#define FREEZE_SCRIPT_GRAMMAR_UTIL_H

#include <FreezeScript/Parser.h>

namespace FreezeScript
{

extern DataFactoryPtr parseDataFactory;
extern ErrorReporterPtr parseErrorReporter;
extern NodePtr parseResult;
extern int parseLine;

int getInput(char*, int);

class StringTok;
class IdentifierTok;
class BoolTok;
class IntegerTok;
class FloatingTok;
class NodeListTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<IdentifierTok> IdentifierTokPtr;
typedef ::IceUtil::Handle<BoolTok> BoolTokPtr;
typedef ::IceUtil::Handle<IntegerTok> IntegerTokPtr;
typedef ::IceUtil::Handle<FloatingTok> FloatingTokPtr;
typedef ::IceUtil::Handle<NodeListTok> NodeListTokPtr;

// ----------------------------------------------------------------------
// Token
// ----------------------------------------------------------------------

class Token : public Node
{
public:

    Token() { }
    virtual DataPtr evaluate(const SymbolTablePtr&) { return 0; }
    virtual void print(std::ostream&) const {}
};

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class StringTok : public Token
{
public:

    StringTok() { }

    std::string v;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class BoolTok : public Token
{
public:

    BoolTok() { }
    bool v;
};

// ----------------------------------------------------------------------
// IntegerTok
// ----------------------------------------------------------------------

class IntegerTok : public Token
{
public:

    IntegerTok() { }
    IceUtil::Int64 v;
};

// ----------------------------------------------------------------------
// FloatingTok
// ----------------------------------------------------------------------

class FloatingTok : public Token
{
public:

    FloatingTok() { }
    double v;
};

class NodeListTok : public Token
{
public:

    NodeListTok() { }
    NodeList v;
};

} // End of namespace FreezeScript

//
// Stuff for flex and bison
//

#define YYSTYPE FreezeScript::NodePtr
#define YY_DECL int freeze_script_lex(YYSTYPE* yylvalp)
YY_DECL;
int freeze_script_parse();

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
#define yyoverflow(a, b, c, d, e, f) freeze_script_error(a)

#endif
