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

#include <Slice/GrammarUtil.h>

using namespace std;
using namespace Slice;

void
yyerror(const char* s)
{
    unit->error(s);
}

%}

%pure_parser

//
// All keyword tokens. Make sure to modify the "keyword" rule in this
// file if the list of keywords is changed.
//
%token ICE_MODULE
%token ICE_CLASS
%token ICE_INTERFACE
%token ICE_STRUCT
%token ICE_LOCAL
%token ICE_EXTENDS
%token ICE_IMPLEMENTS
%token ICE_THROWS
%token ICE_VOID
%token ICE_BYTE
%token ICE_BOOL
%token ICE_SHORT
%token ICE_INT
%token ICE_LONG
%token ICE_FLOAT
%token ICE_DOUBLE
%token ICE_STRING
%token ICE_WSTRING
%token ICE_OBJECT
%token ICE_LOCAL_OBJECT
%token ICE_SEQUENCE
%token ICE_DICTIONARY
%token ICE_ENUM
%token ICE_NONMUTATING

//
// Other tokens.
//
%token ICE_SCOPE_DELIMITOR
%token ICE_IDENTIFIER
%token ICE_OP_IDENTIFIER
%token ICE_OP_KEYWORD

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
| struct_decl
{
}
| struct_def
{
}
| sequence_def
{
}
| dictionary_def
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
struct_decl
// ----------------------------------------------------------------------
: ICE_STRUCT ICE_IDENTIFIER
{
    unit->error("structs can not be forward declared");
}
| ICE_STRUCT keyword
{
    unit->error("keyword can not be used as struct name");
}
;

// ----------------------------------------------------------------------
struct_def
// ----------------------------------------------------------------------
: ICE_STRUCT ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v);
    if (!st)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(st);
}
'{' struct_exports '}'
{
    unit->popContainer();
}
| ICE_STRUCT keyword
{
    unit->error("keyword can not be used as struct name");
}
'{' struct_exports '}'
{
}
;

// ----------------------------------------------------------------------
struct_exports
// ----------------------------------------------------------------------
: struct_export ';' struct_exports
{
}
| error ';' struct_exports
{
}
| struct_export
{
    unit->error("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
struct_export
// ----------------------------------------------------------------------
: data_member
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
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
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
    ClassDeclPtr cl = cont->createClassDecl(ident->v, local->v, false);
}
| local ICE_CLASS keyword
{
    unit->error("keyword can not be used as class name");
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
    {
	bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, local->v, false, bases->v);
    if (!cl)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(cl);
}
'{' class_exports '}'
{
    unit->popContainer();
}
| local ICE_CLASS keyword class_extends implements
{
    unit->error("keyword can not be used as class name");
}
'{' class_exports '}'
{
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
    ClassDeclPtr cl = cont->createClassDecl(ident->v, local->v, true);
}
| local ICE_INTERFACE keyword
{
    unit->error("keyword can not be used as interface name");
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
    ClassDefPtr cl = cont->createClassDef(ident->v, local->v, true, bases->v);
    if (!cl)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(cl);
}
'{' interface_exports '}'
{
    unit->popContainer();
}
| local ICE_INTERFACE keyword interface_extends
{
    unit->error("keyword can not be used as interface name");
}
'{' interface_exports '}'
{
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
    if (cl)
    {
	cl->createOperation(name->v, returnType, inParms->v, outParms->v, throws->v, false);
    }
}
| ICE_NONMUTATING return_type ICE_OP_IDENTIFIER parameters output_parameters ')' throws
{
    TypePtr returnType = TypePtr::dynamicCast($2);
    StringTokPtr name = StringTokPtr::dynamicCast($3);
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($5);
    TypeListTokPtr throws = TypeListTokPtr::dynamicCast($7);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if (cl)
    {
	cl->createOperation(name->v, returnType, inParms->v, outParms->v, throws->v, true);
    }
}
| return_type ICE_OP_KEYWORD parameters output_parameters ')' throws
{
    unit->error("keyword can not be used as operation name");
}
| ICE_NONMUTATING return_type ICE_OP_KEYWORD parameters output_parameters ')' throws
{
    unit->error("keyword can not be used as operation name");
}
;
 
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
| type ',' parameters
{
    unit->error("missing declarator");
    $$ = $3
}
| type
{
    unit->error("missing declarator");
    $$ = new TypeStringListTok;
}
| type keyword ',' parameters
{
    unit->error("keyword can not be used as declarator");
    $$ = $4
}
| type keyword
{
    unit->error("keyword can not be used as declarator");
    $$ = new TypeStringListTok;
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
    if (cl)
    {
	cl->createDataMember(ident->v, type);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if (st)
    {
	st->createDataMember(ident->v, type);
    }
}
| type keyword
{
    unit->error("keyword can not be used as data member name");
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
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    $$ = types.front();
}
| scoped_name '*'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    list<TypePtr> types = cont->lookupType(scoped->v);
    if (types.empty())
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    for (list<TypePtr>::iterator p = types.begin(); p != types.end(); ++p)
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
sequence_def
// ----------------------------------------------------------------------
: ICE_SEQUENCE '<' type '>' ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($5);
    TypePtr type = TypePtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    cont->createSequence(ident->v, type);
}
| ICE_SEQUENCE '<' type '>' keyword
{
    unit->error("keyword can not be used as sequence name");
}
;

// ----------------------------------------------------------------------
dictionary_def
// ----------------------------------------------------------------------
: ICE_DICTIONARY '<' type ',' type '>' ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($7);
    TypePtr keyType = TypePtr::dynamicCast($3);
    TypePtr valueType = TypePtr::dynamicCast($5);
    ContainerPtr cont = unit->currentContainer();
    cont->createDictionary(ident->v, keyType, valueType);
}
| ICE_DICTIONARY '<' type ',' type '>' keyword
{
    unit->error("keyword can not be used as dictionary name");
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: ICE_ENUM ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v);
    $$ = en;
}
'{' enumerator_list '}'
{
    EnumPtr en = EnumPtr::dynamicCast($3);
    if (en)
    {
	EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast($5);
	en->setEnumerators(enumerators->v);
    }
}
| ICE_ENUM keyword
{
    unit->error("keyword can not be used as enum name");
}
'{' enumerator_list '}'
{
}
;

// ----------------------------------------------------------------------
enumerator_list
// ----------------------------------------------------------------------
: enumerator ',' enumerator_list
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast($1);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast($3)->v);
    $$ = ens;
}
| enumerator
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
enumerator
// ----------------------------------------------------------------------
: ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    $$ = ens;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if (en)
    {
	ens->v.push_front(en);
    }
}
| keyword
{
    unit->error("keyword can not be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok;
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

// ----------------------------------------------------------------------
keyword
// ----------------------------------------------------------------------
: ICE_MODULE
{
}
| ICE_CLASS
{
}
| ICE_INTERFACE
{
}
| ICE_STRUCT
{
}
| ICE_LOCAL
{
}
| ICE_EXTENDS
{
}
| ICE_IMPLEMENTS
{
}
| ICE_THROWS
{
}
| ICE_VOID
{
}
| ICE_BYTE
{
}
| ICE_BOOL
{
}
| ICE_SHORT
{
}
| ICE_INT
{
}
| ICE_LONG
{
}
| ICE_FLOAT
{
}
| ICE_DOUBLE
{
}
| ICE_STRING
{
}
| ICE_WSTRING
{
}
| ICE_OBJECT
{
}
| ICE_LOCAL_OBJECT
{
}
| ICE_SEQUENCE
{
}
| ICE_DICTIONARY
{
}
| ICE_ENUM
{
}
| ICE_NONMUTATING
{
}
;

%%
