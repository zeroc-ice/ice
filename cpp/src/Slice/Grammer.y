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
    unit->error(s);
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
    unit->error("`;' missing after definition");
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
    unit->error("`;' missing after definition");
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
    unit->error("`;' missing after definition");
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
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ModulePtr module = cont->createModule(ident->v);
    if (!module)
	YYERROR; // Can't continue, jump to next yyerrok
    unit->pushContainer(module);
}
'{' definitions '}'
{
    unit->popContainer();
}
;

// ----------------------------------------------------------------------
local
// ----------------------------------------------------------------------
: ICE_LOCAL
{
    BoolTokPtr local = new BoolTok;
    local->v = true;
    $$ = local;
}
|
{
    BoolTokPtr local = new BoolTok;
    local->v = false;
    $$ = local;
}
;

// ----------------------------------------------------------------------
class_decl
// ----------------------------------------------------------------------
: local ICE_CLASS ICE_IDENTIFIER
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v,
					       local->v,
					       false);
}
;

// ----------------------------------------------------------------------
class_def
// ----------------------------------------------------------------------
: local ICE_CLASS ICE_IDENTIFIER class_extends implements
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast($4);
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($5);
    if (base)
	bases->v.push_front(base);
    ClassDefPtr cl = cont->createClassDef(ident->v,
					     local->v,
					     false,
					     bases->v);
    if (!cl)
	YYERROR; // Can't continue, jump to next yyerrok
    unit->pushContainer(cl);
}
'{' class_exports '}'
{
    unit->popContainer();
}
;

// ----------------------------------------------------------------------
class_extends
// ----------------------------------------------------------------------
: ICE_EXTENDS scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    $$ = 0;
    if (!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if (!cl)
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not a class";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if (!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg);
	    }
	    else
	    {
		$$ = def;
	    }
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
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v,
					       local->v,
					       true);
}
;

// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: local ICE_INTERFACE ICE_IDENTIFIER interface_extends
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($4);
    ClassDefPtr cl = cont->createClassDef(ident->v,
					     local->v,
					     true,
					     bases->v);
    if (!cl)
	YYERROR; // Can't continue, jump to next yyerrok
    unit->pushContainer(cl);
}
'{' interface_exports '}'
{
    unit->popContainer();
}
;

// ----------------------------------------------------------------------
interface_list
// ----------------------------------------------------------------------
: scoped_name ',' interface_list
{
    ClassListTokPtr intfs = ClassListTokPtr::dynamicCast($3);
    $$ = intfs;
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if (!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if (!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg);
	    }
	    else
	    {
		intfs->v.push_front(def);
	    }
	}
    }
}
| scoped_name
{
    ClassListTokPtr intfs = new ClassListTok;
    $$ = intfs;
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if (!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if (!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg);
	    }
	    else
	    {
		intfs->v.push_front(def);
	    }
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
    TypePtr returnType = TypePtr::dynamicCast($1);
    StringTokPtr name = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($3);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($4);
    TypeListTokPtr throws = TypeListTokPtr::dynamicCast($6);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    cl->createOperation(name->v, returnType, inParms->v, outParms->v,
			  throws->v);
}

// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER ',' parameters
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr parms = TypeStringListTokPtr::dynamicCast($4);
    parms->v.push_front(make_pair(type, ident->v));
    $$ = parms;
}
| type ICE_IDENTIFIER
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr parms = new TypeStringListTok;
    parms->v.push_front(make_pair(type, ident->v));
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
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(!cl->isInterface());
    cl->createDataMember(ident->v, type);
}
;

// ----------------------------------------------------------------------
type
// ----------------------------------------------------------------------
: ICE_BYTE
{
    $$ = unit->builtin(Builtin::KindByte);
}
| ICE_BOOL
{
    $$ = unit->builtin(Builtin::KindBool);
}
| ICE_SHORT
{
    $$ = unit->builtin(Builtin::KindShort);
}
| ICE_INT
{
    $$ = unit->builtin(Builtin::KindInt);
}
| ICE_LONG
{
    $$ = unit->builtin(Builtin::KindLong);
}
| ICE_FLOAT
{
    $$ = unit->builtin(Builtin::KindFloat);
}
| ICE_DOUBLE
{
    $$ = unit->builtin(Builtin::KindDouble);
}
| ICE_STRING
{
    $$ = unit->builtin(Builtin::KindString);
}
| ICE_WSTRING
{
    $$ = unit->builtin(Builtin::KindWString);
}
| ICE_OBJECT
{
    $$ = unit->builtin(Builtin::KindObject);
}
| ICE_OBJECT '*'
{
    $$ = unit->builtin(Builtin::KindObjectProxy);
}
| ICE_LOCAL_OBJECT
{
    $$ = unit->builtin(Builtin::KindLocalObject);
}
| scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    if (types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    $$ = types.front();
}
| scoped_name '*'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    if (types.empty())
	YYERROR; // Can't continue, jump to next yyerrok
    for (list<TypePtr>::iterator p = types.begin();
	p != types.end();
	++p)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	if (!cl)
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' must be class or interface";
	    unit->error(msg);
	    YYERROR; // Can't continue, jump to next yyerrok
	}
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
    TypePtr type = TypePtr::dynamicCast($1);
    TypeListTokPtr typeList = TypeListTokPtr::dynamicCast($3);
    typeList->v.push_front(type);
    $$ = typeList;
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    TypeListTokPtr typeList = new TypeListTok;
    typeList->v.push_front(type);
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
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    cont->createNative(ident->v);
}
;

// ----------------------------------------------------------------------
vector_def
// ----------------------------------------------------------------------
: ICE_VECTOR '<' type '>' ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($5);
    TypePtr type = TypePtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    cont->createVector(ident->v, type);
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: ICE_ENUM ICE_IDENTIFIER '{' identifier_list '}'
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    StringListTokPtr enumerators = StringListTokPtr::dynamicCast($4);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, enumerators->v);
}
;

// ----------------------------------------------------------------------
identifier_list
// ----------------------------------------------------------------------
: ICE_IDENTIFIER ',' identifier_list
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    StringListTokPtr ens = StringListTokPtr::dynamicCast($3);
    $$ = ens;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if (en)
	ens->v.push_front(ident->v);
}
| ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    StringListTokPtr ens = new StringListTok;
    $$ = ens;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if (en)
	ens->v.push_front(ident->v);
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
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ident->v = "::" + ident->v;
    $$ = ident;
}
| scoped_name ICE_SCOPE_DELIMITOR ICE_IDENTIFIER
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    scoped->v += "::";
    scoped->v += ident->v;
    $$ = scoped;
}
;

%%
