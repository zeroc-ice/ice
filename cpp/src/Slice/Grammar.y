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

#ifdef _WIN32
// I get this warning from some bison version:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
#endif

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
// file if the list of keywords is changed. Also make sure to add the
// keyword to the keyword table in Scanner.l.
//
%token ICE_MODULE
%token ICE_CLASS
%token ICE_INTERFACE
%token ICE_EXCEPTION
%token ICE_STRUCT
%token ICE_SEQUENCE
%token ICE_DICTIONARY
%token ICE_ENUM
%token ICE_OUT
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
%token ICE_OBJECT
%token ICE_LOCAL_OBJECT
%token ICE_LOCAL

//
// Other tokens.
//
%token ICE_SCOPE_DELIMITOR
%token ICE_IDENTIFIER
%token ICE_STRING_LITERAL

//
// One shift/reduce conflict is caused by the presence of ICE_OUT
// in the "keyword" production. That's because when the parser sees
// something like
// 
// void op(out long l);
//            ^
// and has just consumed the ICE_OUT, it can either reduce ICE_OUT via
// the "keyword" production or continue to shift via the "out_param_decl"
// production. We could remove ICE_OUT from the "keyword" production,
// but then we wouldn't detect other incorrect uses of ICE_OUT, such as
// using "out" as the name of an operation. Overall, it's better to live
// with the shift/reduce conflict (and the default shift action here is
// what we want anyway).
//
%expect 1

%%


// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: definitions
{
}
;

// ----------------------------------------------------------------------
meta_data
// ----------------------------------------------------------------------
: '[' string_list ']'
{
    $$ = $2;
}
| '[' ']'
{
    $$ = new StringListTok;
}
|
{
    $$ = new StringListTok;
}
;

// ----------------------------------------------------------------------
definitions
// ----------------------------------------------------------------------
: meta_data definition ';' definitions
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
| error ';' definitions
{
    yyerrok;
}
| meta_data definition
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
| exception_decl
{
}
| exception_def
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
module_def
// ----------------------------------------------------------------------
: ICE_MODULE ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ModulePtr module = cont->createModule(ident->v);
    if(!module)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(module);
    $$ = module;
}
'{' definitions '}'
{
    unit->popContainer();
    $$ = $3;
}
;

// ----------------------------------------------------------------------
exception_id
// ----------------------------------------------------------------------
: ICE_EXCEPTION ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_EXCEPTION keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
exception_decl
// ----------------------------------------------------------------------
: local exception_id
{
    unit->error("exceptions cannot be forward declared");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
exception_def
// ----------------------------------------------------------------------
: local exception_id exception_extends
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ExceptionPtr base = ExceptionPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base, local->v);
    if(!ex)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(ex);
    $$ = ex;
}
'{' exception_exports '}'
{
    unit->popContainer();
    $$ = $4;
}
;

// ----------------------------------------------------------------------
exception_extends
// ----------------------------------------------------------------------
: ICE_EXTENDS scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->lookupException(scoped->v);
}
|
{
    $$ = 0;
}
;

// ----------------------------------------------------------------------
exception_exports
// ----------------------------------------------------------------------
: meta_data exception_export ';' exception_exports
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
| error ';' exception_exports
{
}
| meta_data exception_export
{
    unit->error("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
exception_export
// ----------------------------------------------------------------------
: type_id
{
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($1);
    TypePtr type = tsp->v.first;
    string ident = tsp->v.second;
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    assert(ex);
    $$ = ex->createDataMember(ident, type);
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    $$ = ex->createDataMember(ident->v, type);
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    unit->error("missing data member name");
    $$ = ex->createDataMember("", type);
}
;

// ----------------------------------------------------------------------
struct_id
// ----------------------------------------------------------------------
: ICE_STRUCT ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_STRUCT keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
struct_decl
// ----------------------------------------------------------------------
: local struct_id
{
    unit->error("structs cannot be forward declared");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
struct_def
// ----------------------------------------------------------------------
: local struct_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v, local->v);
    if(!st)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(st);
    $$ = st;
}
'{' struct_exports '}'
{
    unit->popContainer();
    $$ = $3;

    //
    // Empty structures are not allowed
    //
    StructPtr st = StructPtr::dynamicCast($$);
    assert(st);
    if(st->dataMembers().empty())
    {
    	unit->error("struct `" + st->name() + "' must have at least one member");
    }
}
;

// ----------------------------------------------------------------------
struct_exports
// ----------------------------------------------------------------------
: meta_data struct_export ';' struct_exports
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
| error ';' struct_exports
{
}
| meta_data struct_export
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
: type_id
{
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($1);
    TypePtr type = tsp->v.first;
    string ident = tsp->v.second;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    $$ = st->createDataMember(ident, type);
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    unit->error("keyword `" + ident->v + "' cannot be used as data member name");
    $$ = st->createDataMember(ident->v, type);
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    unit->error("missing data member name");
    $$ = st->createDataMember("", type);
}
;

// ----------------------------------------------------------------------
class_id
// ----------------------------------------------------------------------
: ICE_CLASS ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_CLASS keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
class_decl
// ----------------------------------------------------------------------
: local class_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    $$ = cl;
}
;

// ----------------------------------------------------------------------
class_def
// ----------------------------------------------------------------------
: local class_id class_extends implements
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast($3);
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($4);
    if(base)
    {
	bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, false, bases->v, local->v);
    if(!cl)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(cl);
    $$ = cl;
}
'{' class_exports '}'
{
    unit->popContainer();
    $$ = $6;
}
;

// ----------------------------------------------------------------------
class_extends
// ----------------------------------------------------------------------
: ICE_EXTENDS scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    $$ = 0;
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl)
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not a class";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
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
class_exports
// ----------------------------------------------------------------------
: meta_data class_export ';' class_exports
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
| error ';' class_exports
{
}
| meta_data class_export
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
| type_id
{
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($1);
    TypePtr type = tsp->v.first;
    string ident = tsp->v.second;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    $$ = cl->createDataMember(ident, type);
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    unit->error("keyword `" + ident->v + "' cannot be used as data member name");
    $$ = cl->createDataMember(ident->v, type);
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    unit->error("missing data member name");
    $$ = cl->createDataMember("", type);
}
;

// ----------------------------------------------------------------------
interface_id
// ----------------------------------------------------------------------
: ICE_INTERFACE ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_INTERFACE keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
interface_decl
// ----------------------------------------------------------------------
: local interface_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    $$ = cl;
}
;

// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: local interface_id interface_extends
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($3);
    ClassDefPtr cl = cont->createClassDef(ident->v, true, bases->v, local->v);
    if(!cl)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    unit->pushContainer(cl);
    $$ = cl;
}
'{' interface_exports '}'
{
    unit->popContainer();
    $$ = $4;
}
;

// ----------------------------------------------------------------------
interface_list
// ----------------------------------------------------------------------
: scoped_name ',' interface_list
{
    ClassListTokPtr intfs = ClassListTokPtr::dynamicCast($3);
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
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
    $$ = intfs;
}
| scoped_name
{
    ClassListTokPtr intfs = new ClassListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
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
    $$ = intfs;
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
interface_exports
// ----------------------------------------------------------------------
: meta_data interface_export ';' interface_exports
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
| error ';' interface_exports
{
}
| meta_data interface_export
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
exception_list
// ----------------------------------------------------------------------
: exception ',' exception_list
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast($1);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast($3);
    exceptionList->v.push_front(exception);
    $$ = exceptionList;
}
| exception
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast($1);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    $$ = exceptionList;
}
;

// ----------------------------------------------------------------------
exception
// ----------------------------------------------------------------------
/* TODO: builtin exceptions "Exception" and "LocalException"*/
// ML: Not sure if these should be treated as builtin by the
// parser. They have no meaning in a Slice definition, so why make
// them builtin, if they cannot be used anywhere in Slice?
: scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    $$ = exception;
}
;

// ----------------------------------------------------------------------
sequence_def
// ----------------------------------------------------------------------
: local ICE_SEQUENCE '<' type '>' ICE_IDENTIFIER
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($6);
    TypePtr type = TypePtr::dynamicCast($4);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, local->v);
}
| local ICE_SEQUENCE '<' type '>' keyword
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($6);
    TypePtr type = TypePtr::dynamicCast($4);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, local->v);
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
;

// ----------------------------------------------------------------------
dictionary_def
// ----------------------------------------------------------------------
: local ICE_DICTIONARY '<' type ',' type '>' ICE_IDENTIFIER
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($8);
    TypePtr keyType = TypePtr::dynamicCast($4);
    TypePtr valueType = TypePtr::dynamicCast($6);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, valueType, local->v);
}
| local ICE_DICTIONARY '<' type ',' type '>' keyword
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($8);
    TypePtr keyType = TypePtr::dynamicCast($4);
    TypePtr valueType = TypePtr::dynamicCast($6);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, valueType, local->v);
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
;

// ----------------------------------------------------------------------
enum_id
// ----------------------------------------------------------------------
: ICE_ENUM ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_ENUM keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: local enum_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    $$ = en;
}
'{' enumerator_list '}'
{
    EnumPtr en = EnumPtr::dynamicCast($3);
    if(en)
    {
	EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast($5);
	en->setEnumerators(enumerators->v);
    }
    $$ = $3;
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
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if(en)
    {
	ens->v.push_front(en);
    }
    $$ = ens;
}
| keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    $$ = ens;
}
;

// ----------------------------------------------------------------------
type_id
// ----------------------------------------------------------------------
: type ICE_IDENTIFIER
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    $$ = typestring;
}
;

// ----------------------------------------------------------------------
operation
// ----------------------------------------------------------------------
: type_id '(' parameters output_parameters ')' throws
{
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($1);
    TypePtr returnType = tsp->v.first;
    string name = tsp->v.second;
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($3);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($4);
    ExceptionListTokPtr throws = ExceptionListTokPtr::dynamicCast($6);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    $$ = cl->createOperation(name, returnType, inParms->v, outParms->v, throws->v);
}
| ICE_VOID ICE_IDENTIFIER '(' parameters output_parameters ')' throws
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($5);
    ExceptionListTokPtr throws = ExceptionListTokPtr::dynamicCast($7);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    $$ = cl->createOperation(ident->v, 0, inParms->v, outParms->v, throws->v);
}
| type keyword '(' parameters output_parameters ')' throws
{
    TypePtr returnType = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($5);
    ExceptionListTokPtr throws = ExceptionListTokPtr::dynamicCast($7);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    unit->error("keyword `" + ident->v + "' cannot be used as operation name");
    $$ = cl->createOperation(ident->v, returnType, inParms->v, outParms->v, throws->v);
}
| ICE_VOID keyword '(' parameters output_parameters ')' throws
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr inParms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringListTokPtr outParms = TypeStringListTokPtr::dynamicCast($5);
    ExceptionListTokPtr throws = ExceptionListTokPtr::dynamicCast($7);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    assert(cl);
    unit->error("keyword `" + ident->v + "' cannot be used as operation name");
    $$ = cl->createOperation(ident->v, 0, inParms->v, outParms->v, throws->v);
}
;
 
// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
:  type_id ',' parameters
{
    TypeStringListTokPtr parms = TypeStringListTokPtr::dynamicCast($3);
    TypeStringTokPtr typestring = TypeStringTokPtr::dynamicCast($1);
    parms->v.push_front(typestring->v);
    $$ = parms;
}
| type_id
{
    TypeStringTokPtr typestring = TypeStringTokPtr::dynamicCast($1);
    TypeStringListTokPtr parms = new TypeStringListTok;
    parms->v.push_front(typestring->v);
    $$ = parms;
}
|
{
    $$ = new TypeStringListTok;
}
| type keyword ',' parameters
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringListTokPtr parms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    parms->v.push_front(typestring->v);
    unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    $$ = parms;
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    TypeStringListTokPtr parms = new TypeStringListTok;
    parms->v.push_front(typestring->v);
    unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    $$ = parms;
}
| type
{
    unit->error("missing parameter name");
    YYERROR; // Can't continue, jump to next yyerrok
}
;

// ----------------------------------------------------------------------
output_parameters
// ----------------------------------------------------------------------

//
// TODO: remove the first rule before releasing stable_39 -- the syntax
// for out parameters has changed to use the out keyword, and the
// semicolon syntax is deprecated for stable_38, to be removed with
// stable_39.
//
: ';' parameters
{
    unit->warning("deprecated use of semicolon to indicate out parameters");
    unit->warning("use the out keyword instead");
    $$ = $2
}
| ICE_OUT type_id ',' output_parameters
{
    TypeStringListTokPtr parms = TypeStringListTokPtr::dynamicCast($4);
    TypeStringTokPtr typestring = TypeStringTokPtr::dynamicCast($2);
    parms->v.push_front(typestring->v);
    $$ = parms;
}
| ICE_OUT type_id
{
    TypeStringTokPtr typestring = TypeStringTokPtr::dynamicCast($2);
    TypeStringListTokPtr parms = new TypeStringListTok;
    parms->v.push_front(typestring->v);
    $$ = parms;
}
|
{
    $$ = new TypeStringListTok;
}
| ICE_OUT type_id ',' type_id
{
    unit->error("in parameters cannot follow out parameters");
    YYERROR; // Can't continue, jump to next yyerrok
}
| ICE_OUT type keyword ',' output_parameters
{
    TypePtr type = TypePtr::dynamicCast($2);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    TypeStringListTokPtr parms = TypeStringListTokPtr::dynamicCast($5);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    parms->v.push_front(typestring->v);
    unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    $$ = parms;
}
| ICE_OUT type keyword
{
    TypePtr type = TypePtr::dynamicCast($2);
    StringTokPtr ident = StringTokPtr::dynamicCast($3);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    TypeStringListTokPtr parms = new TypeStringListTok;
    parms->v.push_front(typestring->v);
    unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    $$ = parms;
}
| ICE_OUT type
{
    unit->error("missing parameter name");
    YYERROR; // Can't continue, jump to next yyerrok
}
;

// ----------------------------------------------------------------------
throws
// ----------------------------------------------------------------------
: ICE_THROWS exception_list
{
    $$ = $2;
}
|
{
    $$ = new ExceptionListTok;
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
    TypeList types = cont->lookupType(scoped->v);
    if(types.empty())
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    $$ = types.front();
}
| scoped_name '*'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(types.empty())
    {
	YYERROR; // Can't continue, jump to next yyerrok
    }
    for(TypeList::iterator p = types.begin(); p != types.end(); ++p)
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	if(!cl)
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
string_list
// ----------------------------------------------------------------------
: ICE_STRING_LITERAL ',' string_list
{
    StringTokPtr str = StringTokPtr::dynamicCast($1);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast($3);
    stringList->v.push_back(str->v);
    $$ = stringList;
}
| ICE_STRING_LITERAL
{
    StringTokPtr str = StringTokPtr::dynamicCast($1);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    $$ = stringList;
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
| ICE_EXCEPTION
{
}
| ICE_STRUCT
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
| ICE_OUT
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
| ICE_OBJECT
{
}
| ICE_LOCAL_OBJECT
{
}
| ICE_LOCAL
{
}
;

%%
