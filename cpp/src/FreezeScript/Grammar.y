%{

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/GrammarUtil.h>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

//
// Avoid old style cast warnings in generated grammar
//
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

using namespace std;
using namespace FreezeScript;

void
freeze_script_error(const char* s)
{
    // yacc and recent versions of Bison use "syntax error" instead
    // of "parse error".

    if(strcmp(s, "parse error") == 0)
    {
        parseErrorReporter->expressionSyntaxError("syntax error");
    }
    else
    {
        parseErrorReporter->expressionSyntaxError(s);
    }
}

%}

%pure_parser

%token TOK_AND
%token TOK_OR
%token TOK_NOT
%token TOK_ADD
%token TOK_SUB
%token TOK_MUL
%token TOK_DIV
%token TOK_MOD
%token TOK_LPAREN
%token TOK_RPAREN
%token TOK_LBRACKET
%token TOK_RBRACKET
%token TOK_LESS_THAN
%token TOK_GREATER_THAN
%token TOK_LESS_EQUAL
%token TOK_GREATER_EQUAL
%token TOK_EQUAL
%token TOK_NEQ
%token TOK_TRUE
%token TOK_FALSE
%token TOK_NIL
%token TOK_SCOPE_DELIMITER
%token TOK_IDENTIFIER
%token TOK_STRING_LITERAL
%token TOK_INTEGER_LITERAL
%token TOK_FLOATING_POINT_LITERAL

%left TOK_OR
%left TOK_AND
%nonassoc TOK_LESS_THAN TOK_GREATER_THAN TOK_LESS_EQUAL TOK_GREATER_EQUAL TOK_EQUAL TOK_NEQ
%left TOK_ADD TOK_SUB
%left TOK_MUL TOK_DIV TOK_MOD
%right UNARY_OP

%%

// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: expr
{
    parseResult = $1;
}
;

// ----------------------------------------------------------------------
expr
// ----------------------------------------------------------------------
: binary
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
binary
// ----------------------------------------------------------------------
: binary TOK_LESS_THAN binary
{
    $$ = new BinaryNode(BinOpLess, parseDataFactory, $1, $3);
}
| binary TOK_GREATER_THAN binary
{
    $$ = new BinaryNode(BinOpGreater, parseDataFactory, $1, $3);
}
| binary TOK_LESS_EQUAL binary
{
    $$ = new BinaryNode(BinOpLessEq, parseDataFactory, $1, $3);
}
| binary TOK_GREATER_EQUAL binary
{
    $$ = new BinaryNode(BinOpGrEq, parseDataFactory, $1, $3);
}
| binary TOK_EQUAL binary
{
    $$ = new BinaryNode(BinOpEq, parseDataFactory, $1, $3);
}
| binary TOK_NEQ binary
{
    $$ = new BinaryNode(BinOpNotEq, parseDataFactory, $1, $3);
}
| binary TOK_OR binary
{
    $$ = new BinaryNode(BinOpOr, parseDataFactory, $1, $3);
}
| binary TOK_AND binary
{
    $$ = new BinaryNode(BinOpAnd, parseDataFactory, $1, $3);
}
| binary TOK_MUL binary
{
    $$ = new BinaryNode(BinOpMul, parseDataFactory, $1, $3);
}
| binary TOK_DIV binary
{
    $$ = new BinaryNode(BinOpDiv, parseDataFactory, $1, $3);
}
| binary TOK_MOD binary
{
    $$ = new BinaryNode(BinOpMod, parseDataFactory, $1, $3);
}
| binary TOK_ADD binary
{
    $$ = new BinaryNode(BinOpAdd, parseDataFactory, $1, $3);
}
| binary TOK_SUB binary
{
    $$ = new BinaryNode(BinOpSub, parseDataFactory, $1, $3);
}
| unary
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
unary
// ----------------------------------------------------------------------
: TOK_LPAREN expr TOK_RPAREN
{
    $$ = $2;
}
| TOK_SUB unary       %prec UNARY_OP
{
    $$ = new UnaryNode(UnaryOpNeg, parseDataFactory, $2);
}
| TOK_NOT unary       %prec UNARY_OP
{
    $$ = new UnaryNode(UnaryOpNot, parseDataFactory, $2);
}
| TOK_INTEGER_LITERAL
{
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast($1);
    assert(intVal);
    $$ = new DataNode(parseDataFactory->createInteger(intVal->v, true));
}
| TOK_FLOATING_POINT_LITERAL
{
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast($1);
    assert(floatVal);
    $$ = new DataNode(parseDataFactory->createDouble(floatVal->v, true));
}
| TOK_STRING_LITERAL
{
    StringTokPtr stringVal = StringTokPtr::dynamicCast($1);
    assert(stringVal);
    $$ = new DataNode(parseDataFactory->createString(stringVal->v, true));
}
| TOK_TRUE
{
    $$ = new DataNode(parseDataFactory->createBoolean(true, true));
}
| TOK_FALSE
{
    $$ = new DataNode(parseDataFactory->createBoolean(false, true));
}
| TOK_NIL
{
    $$ = new DataNode(parseDataFactory->createNil(true));
}
| entity
{
    $$ = $1;
}
| entity '.' function
{
    EntityNodePtr entity = EntityNodePtr::dynamicCast($1);
    assert(entity);
    FunctionNodePtr func = FunctionNodePtr::dynamicCast($3);
    assert(func);
    func->setTarget(entity);
    $$ = $3;
}
| function
{
    $$ = $1;
}
| constant
{
    StringTokPtr stringVal = StringTokPtr::dynamicCast($1);
    assert(stringVal);
    $$ = new ConstantNode(stringVal->v);
}
;

// ----------------------------------------------------------------------
entity
// ----------------------------------------------------------------------
: entity TOK_LBRACKET expr TOK_RBRACKET
{
    EntityNodePtr entity = EntityNodePtr::dynamicCast($1);
    assert(entity);
    entity->append(new ElementNode($3));
    $$ = $1;
}
| entity '.' TOK_IDENTIFIER
{
    StringTokPtr stringVal = StringTokPtr::dynamicCast($3);
    assert(stringVal);
    EntityNodePtr entity = EntityNodePtr::dynamicCast($1);
    assert(entity);
    entity->append(new IdentNode(stringVal->v));
    $$ = $1;
}
| TOK_IDENTIFIER
{
    StringTokPtr stringVal = StringTokPtr::dynamicCast($1);
    assert(stringVal);
    $$ = new IdentNode(stringVal->v);
}
;

// ----------------------------------------------------------------------
function
// ----------------------------------------------------------------------
: TOK_IDENTIFIER TOK_LPAREN arg_list TOK_RPAREN
{
    StringTokPtr func = StringTokPtr::dynamicCast($1);
    assert(func);
    NodeListTokPtr args = NodeListTokPtr::dynamicCast($3);
    assert(args);
    $$ = new FunctionNode(func->v, args->v);
}
;

// ----------------------------------------------------------------------
arg_list
// ----------------------------------------------------------------------
: arg_list ',' expr
{
    NodeListTokPtr l = NodeListTokPtr::dynamicCast($1);
    assert(l);
    l->v.push_back($3);
    $$ = $1;
}
| expr
{
    NodeListTokPtr result = new NodeListTok;
    result->v.push_back($1);
    $$ = result;
}
|
{
    $$ = new NodeListTok;
}
;

// ----------------------------------------------------------------------
constant
// ----------------------------------------------------------------------
: constant TOK_SCOPE_DELIMITER TOK_IDENTIFIER
{
    StringTokPtr stringVal = StringTokPtr::dynamicCast($1);
    assert(stringVal);
    StringTokPtr idVal = StringTokPtr::dynamicCast($3);
    assert(idVal);
    stringVal->v.append("::" + idVal->v);
    $$ = $1;
}
| TOK_SCOPE_DELIMITER TOK_IDENTIFIER
{
    StringTokPtr idVal = StringTokPtr::dynamicCast($2);
    assert(idVal);
    StringTokPtr stringVal = new StringTok;
    stringVal->v.append("::" + idVal->v);
    $$ = stringVal;
}
;

%%
