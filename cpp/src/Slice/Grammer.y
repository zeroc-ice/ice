%{

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Parser.h>

using namespace std;
using namespace Slice;

void
yyerror(const char* s)
{
    parser -> error(s);
}

%}

%token ICE_SCOPE_DELIMITOR
%token ICE_MODULE
%token ICE_CLASS
%token ICE_LOCAL
%token ICE_EXTENDS
%token ICE_THROWS
%token ICE_VOID
%token ICE_BOOL
%token ICE_BYTE
%token ICE_SHORT
%token ICE_INT
%token ICE_LONG
%token ICE_FLOAT
%token ICE_DOUBLE
%token ICE_STRING
%token ICE_WSTRING
%token ICE_OBJECT
%token ICE_NATIVE
%token ICE_VECTOR
%token ICE_IDENTIFIER
%token ICE_OP_IDENTIFIER

%%

// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: definitions
{
}
;

// ----------------------------------------------------------------------
definitions
// ----------------------------------------------------------------------
: definition ';' definitions
{
}
| error ';' definitions
{
    yyerrok;
}
| definition
{
    yyerror("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
definition
// ----------------------------------------------------------------------
: module
{
}
| class
{
}
| class_decl
{
}
| native
{
}
| vector
{
}
;

// ----------------------------------------------------------------------
exports
// ----------------------------------------------------------------------
: export ';' exports
{
}
| error ';' exports
{
}
| export
{
    yyerror("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
export
// ----------------------------------------------------------------------
: operation
{
}
| data_member
{
}
;

// ----------------------------------------------------------------------
module
// ----------------------------------------------------------------------
: ICE_MODULE ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($2);
    Container_ptr cont = parser -> currentContainer();
    Module_ptr module = cont -> createModule(ident -> v);
    parser -> pushContainer(module);
}
'{' definitions '}'
{
    parser -> popContainer();
}
;

// ----------------------------------------------------------------------
class
// ----------------------------------------------------------------------
: ICE_CLASS ICE_IDENTIFIER extends
{
    String_ptr ident = String_ptr::dynamicCast($2);
    Container_ptr cont = parser -> currentContainer();
    ClassDef_ptr base = ClassDef_ptr::dynamicCast($3);
    ClassDef_ptr derived = cont -> createClassDef(ident -> v, base, false);
    parser -> pushContainer(derived);
}
'{' exports '}'
{
    parser -> popContainer();
}
| ICE_LOCAL ICE_CLASS ICE_IDENTIFIER extends
{
    String_ptr ident = String_ptr::dynamicCast($3);
    Container_ptr cont = parser -> currentContainer();
    ClassDef_ptr base = ClassDef_ptr::dynamicCast($4);
    ClassDef_ptr derived = cont -> createClassDef(ident -> v, base, true);
    parser -> pushContainer(derived);
}
'{' exports '}'
{
    parser -> popContainer();
}
;

// ----------------------------------------------------------------------
class_decl
// ----------------------------------------------------------------------
: ICE_CLASS ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($2);
    Container_ptr cont = parser -> currentContainer();
    ClassDecl_ptr cl = cont -> createClassDecl(ident -> v, false);
}
| ICE_LOCAL ICE_CLASS ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($3);
    Container_ptr cont = parser -> currentContainer();
    ClassDecl_ptr cl = cont -> createClassDecl(ident -> v, true);
}
;

// ----------------------------------------------------------------------
extends
// ----------------------------------------------------------------------
: ICE_EXTENDS scoped_name
{
    String_ptr scoped = String_ptr::dynamicCast($2);
    Container_ptr cont = parser -> currentContainer();
    vector<Type_ptr> types = cont -> lookupType(scoped -> v);
    if(types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(types[0]);
    if(!cl)
    {
	string msg = "`";
	msg += scoped -> v;
	msg += "' is not a class";
	yyerror(msg.c_str());
	$$ = 0;
    }
    else
    {
	ClassDef_ptr def = cl -> definition();
	if(!def)
	{
	    string msg = "`";
	    msg += scoped -> v;
	    msg += "' has been declared but not defined";
	    yyerror(msg.c_str());
	    $$ = 0;
	}
	else
	{
	    $$ = def;
	}
    }
}
|
{
    $$ = 0;
}
;

// ----------------------------------------------------------------------
operation
// ----------------------------------------------------------------------
: return_type ICE_OP_IDENTIFIER parameters output_parameters ')' throws
{
    Type_ptr returnType = Type_ptr::dynamicCast($1);
    String_ptr name = String_ptr::dynamicCast($2);
    Parameters_ptr inParms = Parameters_ptr::dynamicCast($3);
    Parameters_ptr outParms = Parameters_ptr::dynamicCast($4);
    Throws_ptr throws = Throws_ptr::dynamicCast($6);
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(parser -> currentContainer());
    cl -> createOperation(name -> v, returnType, inParms -> v, outParms -> v,
			  throws -> v);
}

// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER ',' parameters
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    String_ptr ident = String_ptr::dynamicCast($2);
    Parameters_ptr parms = Parameters_ptr::dynamicCast($4);
    parms -> v.push_front(make_pair(type, ident -> v));
    $$ = parms;
}
| type ICE_IDENTIFIER
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    String_ptr ident = String_ptr::dynamicCast($2);
    Parameters_ptr parms = new Parameters;
    parms -> v.push_front(make_pair(type, ident -> v));
    $$ = parms;
}
|
{
    $$ = new Parameters;
}
;

// ----------------------------------------------------------------------
output_parameters
// ----------------------------------------------------------------------
: ';' parameters
{
    $$ = $2
}
|
{
    $$ = new Parameters;
}

// ----------------------------------------------------------------------
throws
// ----------------------------------------------------------------------
: ICE_THROWS throw_list
{
    $$ = $2;
}
|
{
    $$ = new Throws;
}
;

// ----------------------------------------------------------------------
throw_list
// ----------------------------------------------------------------------
: type ',' throw_list
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    Throws_ptr throws = Throws_ptr::dynamicCast($3);
    throws -> v.push_front(type);
    $$ = throws;
}
| type
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    Throws_ptr throws = new Throws;
    throws -> v.push_front(type);
    $$ = throws;
}
;

// ----------------------------------------------------------------------
data_member
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    String_ptr ident = String_ptr::dynamicCast($2);
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(parser -> currentContainer());
    cl -> createDataMember(ident -> v, type);
}
;

// ----------------------------------------------------------------------
type
// ----------------------------------------------------------------------
: ICE_BYTE
{
    $$ = parser -> builtin(Builtin::KindByte);
}
| ICE_BOOL
{
    $$ = parser -> builtin(Builtin::KindBool);
}
| ICE_SHORT
{
    $$ = parser -> builtin(Builtin::KindShort);
}
| ICE_INT
{
    $$ = parser -> builtin(Builtin::KindInt);
}
| ICE_LONG
{
    $$ = parser -> builtin(Builtin::KindLong);
}
| ICE_FLOAT
{
    $$ = parser -> builtin(Builtin::KindFloat);
}
| ICE_DOUBLE
{
    $$ = parser -> builtin(Builtin::KindDouble);
}
| ICE_STRING
{
    $$ = parser -> builtin(Builtin::KindString);
}
| ICE_WSTRING
{
    $$ = parser -> builtin(Builtin::KindWString);
}
| ICE_OBJECT
{
    $$ = parser -> builtin(Builtin::KindObject);
}
| ICE_OBJECT '*'
{
    $$ = parser -> builtin(Builtin::KindObjectProxy);
}
| ICE_LOCAL ICE_OBJECT
{
    $$ = parser -> builtin(Builtin::KindLocalObject);
}
| scoped_name
{
    String_ptr scoped = String_ptr::dynamicCast($1);
    Container_ptr cont = parser -> currentContainer();
    vector<Type_ptr> types = cont -> lookupType(scoped -> v);
    if(types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    $$ = types[0];
}
| scoped_name '*'
{
    String_ptr scoped = String_ptr::dynamicCast($1);
    Container_ptr cont = parser -> currentContainer();
    vector<Type_ptr> types = cont -> lookupType(scoped -> v);
    if(types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    for(vector<Type_ptr>::iterator p = types.begin();
	p != types.end();
	++p)
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*p);
	assert(cl); // TODO: Only classes can be passed as proxy
	*p = new Proxy(cl);
    }
    $$ = types[0];
}
;

// ----------------------------------------------------------------------
return_type
// ----------------------------------------------------------------------
: ICE_VOID
{
    $$ = 0;
}
| type
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
native
// ----------------------------------------------------------------------
: ICE_NATIVE ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($2);
    Container_ptr cont = parser -> currentContainer();
    cont -> createNative(ident -> v);
}
;

// ----------------------------------------------------------------------
vector
// ----------------------------------------------------------------------
: ICE_VECTOR '<' type '>' ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($5);
    Type_ptr type = Type_ptr::dynamicCast($3);
    Container_ptr cont = parser -> currentContainer();
    cont -> createVector(ident -> v, type);
}
;

// ----------------------------------------------------------------------
scoped_name
// ----------------------------------------------------------------------
: ICE_IDENTIFIER
{
    $$ = $1;
}
| ICE_SCOPE_DELIMITOR ICE_IDENTIFIER
{
    String_ptr ident = String_ptr::dynamicCast($2);
    ident -> v = "::" + ident -> v;
    $$ = ident;
}
| scoped_name ICE_SCOPE_DELIMITOR ICE_IDENTIFIER
{
    String_ptr scoped = String_ptr::dynamicCast($1);
    String_ptr ident = String_ptr::dynamicCast($3);
    scoped -> v += "::";
    scoped -> v += ident -> v;
    $$ = scoped;
}
;

%%
