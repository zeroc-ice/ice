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

#include <GrammerUtil.h>

using namespace std;
using namespace Slice;

void
yyerror(const char* s)
{
    unit -> error(s);
}

%}

%token ICE_SCOPE_DELIMITOR
%token ICE_MODULE
%token ICE_LOCAL
%token ICE_CLASS
%token ICE_INTERFACE
%token ICE_EXTENDS
%token ICE_IMPLEMENTS
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
%token ICE_LOCAL_OBJECT
%token ICE_NATIVE
%token ICE_VECTOR
%token ICE_ENUM
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
: module_def
{
}
| class_decl
{
}
| class_def
{
}
| interface_decl
{
}
| interface_def
{
}
| native_def
{
}
| vector_def
{
}
| enum_def
{
}
;

// ----------------------------------------------------------------------
class_exports
// ----------------------------------------------------------------------
: class_export ';' class_exports
{
}
| error ';' class_exports
{
}
| class_export
{
    yyerror("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
class_export
// ----------------------------------------------------------------------
: operation
{
}
| data_member
{
}
;

// ----------------------------------------------------------------------
interface_exports
// ----------------------------------------------------------------------
: interface_export ';' interface_exports
{
}
| error ';' interface_exports
{
}
| interface_export
{
    yyerror("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
interface_export
// ----------------------------------------------------------------------
: operation
{
}
;

// ----------------------------------------------------------------------
module_def
// ----------------------------------------------------------------------
: ICE_MODULE ICE_IDENTIFIER
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    Container_ptr cont = unit -> currentContainer();
    Module_ptr module = cont -> createModule(ident -> v);
    unit -> pushContainer(module);
}
'{' definitions '}'
{
    unit -> popContainer();
}
;

// ----------------------------------------------------------------------
local
// ----------------------------------------------------------------------
: ICE_LOCAL
{
    BoolTok_ptr local = new BoolTok;
    local -> v = true;
    $$ = local;
}
|
{
    BoolTok_ptr local = new BoolTok;
    local -> v = false;
    $$ = local;
}
;

// ----------------------------------------------------------------------
class_decl
// ----------------------------------------------------------------------
: local ICE_CLASS ICE_IDENTIFIER
{
    BoolTok_ptr local = BoolTok_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($3);
    Container_ptr cont = unit -> currentContainer();
    ClassDecl_ptr cl = cont -> createClassDecl(ident -> v, local -> v);
}
;

// ----------------------------------------------------------------------
class_def
// ----------------------------------------------------------------------
: local ICE_CLASS ICE_IDENTIFIER class_extends implements
{
    BoolTok_ptr local = BoolTok_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($3);
    Container_ptr cont = unit -> currentContainer();
    ClassDef_ptr base = ClassDef_ptr::dynamicCast($4);
    ClassListTok_ptr intfs = ClassListTok_ptr::dynamicCast($5);
    ClassDef_ptr derived = cont -> createClassDef(ident -> v, base,
						  intfs -> v, local -> v);
    unit -> pushContainer(derived);
}
'{' class_exports '}'
{
    unit -> popContainer();
}
;

// ----------------------------------------------------------------------
class_extends
// ----------------------------------------------------------------------
: ICE_EXTENDS scoped_name
{
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($2);
    Container_ptr cont = unit -> currentContainer();
    list<Type_ptr> types = cont -> lookupType(scoped -> v);
    assert(!types.empty()); // TODO
    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(types.front());
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
implements
// ----------------------------------------------------------------------
: ICE_IMPLEMENTS interface_list
{
    $$ = $2;
}
|
{
    $$ = new ClassListTok;
}
;

// ----------------------------------------------------------------------
interface_decl
// ----------------------------------------------------------------------
: local ICE_INTERFACE ICE_IDENTIFIER
{
    BoolTok_ptr local = BoolTok_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($3);
    Container_ptr cont = unit -> currentContainer();
    ClassDecl_ptr cl = cont -> createClassDecl(ident -> v, local -> v);
}
;

// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: local ICE_INTERFACE ICE_IDENTIFIER interface_extends
{
    BoolTok_ptr local = BoolTok_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($3);
    Container_ptr cont = unit -> currentContainer();
    ClassListTok_ptr intfs = ClassListTok_ptr::dynamicCast($4);
    ClassDef_ptr derived = cont -> createClassDef(ident -> v, 0,
						  intfs -> v, local -> v);
    unit -> pushContainer(derived);
}
'{' interface_exports '}'
{
    unit -> popContainer();
}
;

// ----------------------------------------------------------------------
interface_list
// ----------------------------------------------------------------------
: scoped_name ',' interface_list
{
    ClassListTok_ptr intfs = ClassListTok_ptr::dynamicCast($3);
    $$ = intfs;
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    list<Type_ptr> types = cont -> lookupType(scoped -> v);
    assert(!types.empty()); // TODO
    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(types.front());
    if(!cl && !cl -> isInterface())
    {
	string msg = "`";
	msg += scoped -> v;
	msg += "' is not an interface";
	yyerror(msg.c_str());
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
	}
	else
	{
	    intfs -> v.push_front(def);
	}
    }
}
| scoped_name
{
    ClassListTok_ptr intfs = new ClassListTok;
    $$ = intfs;
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    list<Type_ptr> types = cont -> lookupType(scoped -> v);
    assert(!types.empty()); // TODO
    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(types.front());
    if(!cl && !cl -> isInterface())
    {
	string msg = "`";
	msg += scoped -> v;
	msg += "' is not an interface";
	yyerror(msg.c_str());
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
	}
	else
	{
	    intfs -> v.push_front(def);
	}
    }
}
;

// ----------------------------------------------------------------------
interface_extends
// ----------------------------------------------------------------------
: ICE_EXTENDS interface_list
{
    $$ = $2;
}
|
{
    $$ = new ClassListTok;
}
;

// ----------------------------------------------------------------------
operation
// ----------------------------------------------------------------------
: return_type ICE_OP_IDENTIFIER parameters output_parameters ')' throws
{
    Type_ptr returnType = Type_ptr::dynamicCast($1);
    StringTok_ptr name = StringTok_ptr::dynamicCast($2);
    TypeStringListTok_ptr inParms = TypeStringListTok_ptr::dynamicCast($3);
    TypeStringListTok_ptr outParms = TypeStringListTok_ptr::dynamicCast($4);
    TypeListTok_ptr throws = TypeListTok_ptr::dynamicCast($6);
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(unit -> currentContainer());
    cl -> createOperation(name -> v, returnType, inParms -> v, outParms -> v,
			  throws -> v);
}

// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER ',' parameters
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    TypeStringListTok_ptr parms = TypeStringListTok_ptr::dynamicCast($4);
    parms -> v.push_front(make_pair(type, ident -> v));
    $$ = parms;
}
| type ICE_IDENTIFIER
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    TypeStringListTok_ptr parms = new TypeStringListTok;
    parms -> v.push_front(make_pair(type, ident -> v));
    $$ = parms;
}
|
{
    $$ = new TypeStringListTok;
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
    $$ = new TypeStringListTok;
}

// ----------------------------------------------------------------------
throws
// ----------------------------------------------------------------------
: ICE_THROWS type_list
{
    $$ = $2;
}
|
{
    $$ = new TypeListTok;
}
;

// ----------------------------------------------------------------------
data_member
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(unit -> currentContainer());
    assert(!cl -> isInterface());
    cl -> createDataMember(ident -> v, type);
}
;

// ----------------------------------------------------------------------
type
// ----------------------------------------------------------------------
: ICE_BYTE
{
    $$ = unit -> builtin(Builtin::KindByte);
}
| ICE_BOOL
{
    $$ = unit -> builtin(Builtin::KindBool);
}
| ICE_SHORT
{
    $$ = unit -> builtin(Builtin::KindShort);
}
| ICE_INT
{
    $$ = unit -> builtin(Builtin::KindInt);
}
| ICE_LONG
{
    $$ = unit -> builtin(Builtin::KindLong);
}
| ICE_FLOAT
{
    $$ = unit -> builtin(Builtin::KindFloat);
}
| ICE_DOUBLE
{
    $$ = unit -> builtin(Builtin::KindDouble);
}
| ICE_STRING
{
    $$ = unit -> builtin(Builtin::KindString);
}
| ICE_WSTRING
{
    $$ = unit -> builtin(Builtin::KindWString);
}
| ICE_OBJECT
{
    $$ = unit -> builtin(Builtin::KindObject);
}
| ICE_OBJECT '*'
{
    $$ = unit -> builtin(Builtin::KindObjectProxy);
}
| ICE_LOCAL_OBJECT
{
    $$ = unit -> builtin(Builtin::KindLocalObject);
}
| scoped_name
{
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    list<Type_ptr> types = cont -> lookupType(scoped -> v);
    if(types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    $$ = types.front();
}
| scoped_name '*'
{
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    list<Type_ptr> types = cont -> lookupType(scoped -> v);
    if(types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    for(list<Type_ptr>::iterator p = types.begin();
	p != types.end();
	++p)
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(*p);
	assert(cl); // TODO: Only classes can be passed as proxy
	*p = new Proxy(cl);
    }
    $$ = types.front();
}
;

// ----------------------------------------------------------------------
type_list
// ----------------------------------------------------------------------
: type ',' type_list
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    TypeListTok_ptr typeList = TypeListTok_ptr::dynamicCast($3);
    typeList -> v.push_front(type);
    $$ = typeList;
}
| type
{
    Type_ptr type = Type_ptr::dynamicCast($1);
    TypeListTok_ptr typeList = new TypeListTok;
    typeList -> v.push_front(type);
    $$ = typeList;
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
native_def
// ----------------------------------------------------------------------
: ICE_NATIVE ICE_IDENTIFIER
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    Container_ptr cont = unit -> currentContainer();
    cont -> createNative(ident -> v);
}
;

// ----------------------------------------------------------------------
vector_def
// ----------------------------------------------------------------------
: ICE_VECTOR '<' type '>' ICE_IDENTIFIER
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($5);
    Type_ptr type = Type_ptr::dynamicCast($3);
    Container_ptr cont = unit -> currentContainer();
    cont -> createVector(ident -> v, type);
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: ICE_ENUM ICE_IDENTIFIER '{' identifier_list '}'
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    StringListTok_ptr enumerators = StringListTok_ptr::dynamicCast($4);
    Container_ptr cont = unit -> currentContainer();
    Enum_ptr en = cont -> createEnum(ident -> v, enumerators -> v);
}
;

// ----------------------------------------------------------------------
identifier_list
// ----------------------------------------------------------------------
: ICE_IDENTIFIER ',' identifier_list
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    Enumerator_ptr en = cont -> createEnumerator(ident -> v);
    StringListTok_ptr ens = StringListTok_ptr::dynamicCast($3);
    ens -> v.push_front(ident -> v);
    $$ = ens;
}
| ICE_IDENTIFIER
{
    StringTok_ptr ident = StringTok_ptr::dynamicCast($1);
    Container_ptr cont = unit -> currentContainer();
    Enumerator_ptr en = cont -> createEnumerator(ident -> v);
    StringListTok_ptr ens = new StringListTok;
    ens -> v.push_front(ident -> v);
    $$ = ens;
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
    StringTok_ptr ident = StringTok_ptr::dynamicCast($2);
    ident -> v = "::" + ident -> v;
    $$ = ident;
}
| scoped_name ICE_SCOPE_DELIMITOR ICE_IDENTIFIER
{
    StringTok_ptr scoped = StringTok_ptr::dynamicCast($1);
    StringTok_ptr ident = StringTok_ptr::dynamicCast($3);
    scoped -> v += "::";
    scoped -> v += ident -> v;
    $$ = scoped;
}
;

%%
