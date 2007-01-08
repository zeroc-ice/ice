%{

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/GrammarUtil.h>
#include <IceUtil/UUID.h>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Slice;

void
slice_error(const char* s)
{
    // yacc and recent versions of Bison use "syntax error" instead
    // of "parse error".

    if (strcmp(s, "parse error") == 0)
    {
	unit->error("syntax error");
    }
    else
    {
	unit->error(s);
    }
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
%token ICE_CONST
%token ICE_FALSE
%token ICE_TRUE
%token ICE_NONMUTATING
%token ICE_IDEMPOTENT

//
// Other tokens.
//
%token ICE_SCOPE_DELIMITER
%token ICE_IDENTIFIER
%token ICE_STRING_LITERAL
%token ICE_INTEGER_LITERAL
%token ICE_FLOATING_POINT_LITERAL
%token ICE_IDENT_OP
%token ICE_KEYWORD_OP
%token ICE_METADATA_OPEN
%token ICE_METADATA_CLOSE
%token ICE_GLOBAL_METADATA_OPEN
%token ICE_GLOBAL_METADATA_CLOSE

%token BAD_CHAR

%%


// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: definitions
{
}
;

// ----------------------------------------------------------------------
global_meta_data
// ----------------------------------------------------------------------
: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE
{
    $$ = $2;
}
;

// ----------------------------------------------------------------------
meta_data
// ----------------------------------------------------------------------
: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE
{
    $$ = $2;
}
|
{
    $$ = new StringListTok;
}
;

// ----------------------------------------------------------------------
definitions
// ----------------------------------------------------------------------
: global_meta_data
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
definitions
| meta_data definition
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($1);
    ContainedPtr contained = ContainedPtr::dynamicCast($2);
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
    unit->setSeenDefinition();
}
';' definitions
| error ';'
{
    yyerrok;
}
definitions
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
    assert($1 == 0 || ModulePtr::dynamicCast($1));
}
| class_decl
{
    assert($1 == 0 || ClassDeclPtr::dynamicCast($1));
}
| class_def
{
    assert($1 == 0 || ClassDefPtr::dynamicCast($1));
}
| interface_decl
{
    assert($1 == 0 || ClassDeclPtr::dynamicCast($1));
}
| interface_def
{
    assert($1 == 0 || ClassDefPtr::dynamicCast($1));
}
| exception_decl
{
    assert($1 == 0);
}
| exception_def
{
    assert($1 == 0 || ExceptionPtr::dynamicCast($1));
}
| struct_decl
{
    assert($1 == 0);
}
| struct_def
{
    assert($1 == 0 || StructPtr::dynamicCast($1));
}
| sequence_def
{
    assert($1 == 0 || SequencePtr::dynamicCast($1));
}
| dictionary_def
{
    assert($1 == 0 || DictionaryPtr::dynamicCast($1));
}
| enum_def
{
    assert($1 == 0 || EnumPtr::dynamicCast($1));
}
| const_def
{
    assert($1 == 0 || ConstPtr::dynamicCast($1));
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
    if(module)
    {
	cont->checkIntroduced(ident->v, module);
	unit->pushContainer(module);
	$$ = module;
    }
    else
    {
        $$ = 0;
    }
}
'{' definitions '}'
{
    if($3)
    {
	unit->popContainer();
	$$ = $3;
    }
    else
    {
        $$ = 0;
    }
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
: local_qualifier exception_id
{
    unit->error("exceptions cannot be forward declared");
    $$ = 0;
}
;

// ----------------------------------------------------------------------
exception_def
// ----------------------------------------------------------------------
: local_qualifier exception_id exception_extends
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ExceptionPtr base = ExceptionPtr::dynamicCast($3);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base, local->v);
    if(ex)
    {
	cont->checkIntroduced(ident->v, ex);
	unit->pushContainer(ex);
    }
    $$ = ex;
}
'{' exception_exports '}'
{
    if($4)
    {
	unit->popContainer();
    }
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
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    $$ = contained;
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
exception_export
// ----------------------------------------------------------------------
: data_member
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
: local_qualifier struct_id
{
    unit->error("structs cannot be forward declared");
    $$ = 0;
}
;

// ----------------------------------------------------------------------
struct_def
// ----------------------------------------------------------------------
: local_qualifier struct_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v, local->v);
    if(st)
    {
	cont->checkIntroduced(ident->v, st);
	unit->pushContainer(st);
    }
    $$ = st;
}
'{' struct_exports '}'
{
    if($3)
    {
	unit->popContainer();
    }
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
: data_member
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
: local_qualifier class_id
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
: local_qualifier class_id class_extends implements
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
    if(cl)
    {
	cont->checkIntroduced(ident->v, cl);
	unit->pushContainer(cl);
	$$ = cl;
    }
    else
    {
        $$ = 0;
    }
}
'{' class_exports '}'
{
    if($5)
    {
	unit->popContainer();
	$$ = $5;
    }
    else
    {
        $$ = 0;
    }
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
	    	cont->checkIntroduced(scoped->v);
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
data_member
// ----------------------------------------------------------------------
: type_id
{
    TypePtr type = TypeStringTokPtr::dynamicCast($1)->v.first;
    string name = TypeStringTokPtr::dynamicCast($1)->v.second;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
	dm = cl->createDataMember(name, type);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
	dm = st->createDataMember(name, type);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
	dm = ex->createDataMember(name, type);
    }
    unit->currentContainer()->checkIntroduced(name, dm);
    $$ = dm;
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	$$ = cl->createDataMember(name, type);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
	$$ = st->createDataMember(name, type);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
	$$ = ex->createDataMember(name, type);
    }
    assert($$);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        $$ = cl->createDataMember(IceUtil::generateUUID(), type);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
	$$ = st->createDataMember(IceUtil::generateUUID(), type);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
	$$ = ex->createDataMember(IceUtil::generateUUID(), type);
    }
    assert($$);
    unit->error("missing data member name");
}
;

// ----------------------------------------------------------------------
return_type
// ----------------------------------------------------------------------
: type
| ICE_VOID
{
    $$ = 0;
}
;

// ----------------------------------------------------------------------
operation_preamble
// ----------------------------------------------------------------------
: return_type ICE_IDENT_OP
{
    TypePtr returnType = TypePtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType);
	if(op)
	{
	    cl->checkIntroduced(name, op);
	    unit->pushContainer(op);
	    $$ = op;
	}
	else
	{
	    $$ = 0;
	}
    }
    else
    {
        $$ = 0;
    }
}
| ICE_NONMUTATING return_type ICE_IDENT_OP
{
    TypePtr returnType = TypePtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType, Operation::Nonmutating);
	if(op)
	{
	    cl->checkIntroduced(name, op);
	    unit->pushContainer(op);
	    static bool firstWarning = true;  
	    
	    string msg = "the keyword 'nonmutating' is deprecated";
	    if(firstWarning)
	    {
		msg += ";\n";
		msg += "You should use instead 'idempotent' plus:\n";
		msg += " - Freeze metadata ([\"freeze:read\"], [\"freeze:write\"]) if you implement your objects with a Freeze evictor\n";
		msg += " - [\"nonmutating\"], if you need to maintain compatibility with operations that expect ";
		msg += "'Nonmutating' as operation-mode. With this metadata, the generated code sends ";
		msg += "'Nonmutating' instead of 'Idempotent'\n";
		msg += " - [\"cpp:const\"], to get a const member function on the generated C++ servant base class";

		firstWarning = false;
	    }
	    
	    unit->warning(msg); 
	    $$ = op;
	}
	else
	{
	    $$ = 0;
	}
    }
    else
    {
        $$ = 0;
    }
}
| ICE_IDEMPOTENT return_type ICE_IDENT_OP
{
    TypePtr returnType = TypePtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType, Operation::Idempotent);
	if(op)
	{
	    cl->checkIntroduced(name, op);
	    unit->pushContainer(op);
	    $$ = op;
	}
	else
	{
	    $$ = 0;
	}
    }
    else
    {
        $$ = 0;
    }
}
| return_type ICE_KEYWORD_OP
{
    TypePtr returnType = TypePtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType);
	if(op)
	{
	    unit->pushContainer(op);
	    unit->error("keyword `" + name + "' cannot be used as operation name");
	    $$ = op;
	}
	else
	{
	    $$ = 0;
	}
    }
    else
    {
        $$ = 0;
    }
}
| ICE_NONMUTATING return_type ICE_KEYWORD_OP
{
    TypePtr returnType = TypePtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType, Operation::Nonmutating);
	if(op)
	{
	    unit->pushContainer(op);
	    unit->error("keyword `" + name + "' cannot be used as operation name");
	    $$ = op;
	}
	else
	{
	    $$ = 0;
	}
    }
    else
    {
    	$$ = 0;
    }
}
| ICE_IDEMPOTENT return_type ICE_KEYWORD_OP
{
    TypePtr returnType = TypePtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType, Operation::Idempotent);
	if(op)
	{
	    unit->pushContainer(op);
	    unit->error("keyword `" + name + "' cannot be used as operation name");
	    $$ = op;
	}
	else
	{
	    return 0;
	}
    }
    else
    {
    	$$ = 0;
    }
}
;

// ----------------------------------------------------------------------
operation
// ----------------------------------------------------------------------
: operation_preamble parameters ')'
{
    if($1)
    {
	unit->popContainer();
	$$ = $1;
    }
    else
    {
        $$ = 0;
    }
}
throws
{
    OperationPtr op = OperationPtr::dynamicCast($4);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast($5);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
| operation_preamble error ')'
{
    if($1)
    {
	unit->popContainer();
    }
    yyerrok;
}
throws
{
    OperationPtr op = OperationPtr::dynamicCast($4);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast($5);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
;

// ----------------------------------------------------------------------
class_export
// ----------------------------------------------------------------------
: data_member
| operation
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
: local_qualifier interface_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    $$ = cl;
}
;

// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: local_qualifier interface_id interface_extends
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($3);
    ClassDefPtr cl = cont->createClassDef(ident->v, true, bases->v, local->v);
    if(cl)
    {
	cont->checkIntroduced(ident->v, cl);
	unit->pushContainer(cl);
	$$ = cl;
    }
    else
    {
        $$ = 0;
    }
}
'{' interface_exports '}'
{
    if($4)
    {
	unit->popContainer();
	$$ = $4;
    }
    else
    {
	$$ = 0;
    }
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
	    	cont->checkIntroduced(scoped->v);
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
	    	cont->checkIntroduced(scoped->v);
		intfs->v.push_front(def);
	    }
	}
    }
    $$ = intfs;
}
| ICE_OBJECT
{
    unit->error("illegal inheritance from type Object");
    $$ = new ClassListTok;
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
: scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
	exception = cont->createException(IceUtil::generateUUID(), 0, false);
    }
    cont->checkIntroduced(scoped->v, exception);
    $$ = exception;
}
| keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    $$ = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false);
}
;

// ----------------------------------------------------------------------
sequence_def
// ----------------------------------------------------------------------
: local_qualifier ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($7);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($4);
    TypePtr type = TypePtr::dynamicCast($5);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, metaData->v, local->v);
}
| local_qualifier ICE_SEQUENCE '<' meta_data type '>' keyword
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($7);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($4);
    TypePtr type = TypePtr::dynamicCast($5);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, metaData->v, local->v);
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
;

// ----------------------------------------------------------------------
dictionary_def
// ----------------------------------------------------------------------
: local_qualifier ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($10);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast($4);
    TypePtr keyType = TypePtr::dynamicCast($5);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast($7);
    TypePtr valueType = TypePtr::dynamicCast($8);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
}
| local_qualifier ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($10);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast($4);
    TypePtr keyType = TypePtr::dynamicCast($5);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast($7);
    TypePtr valueType = TypePtr::dynamicCast($8);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
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
: local_qualifier enum_id
{
    BoolTokPtr local = BoolTokPtr::dynamicCast($1);
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    $$ = en;
}
'{' enumerator_list '}'
{
    EnumPtr en = EnumPtr::dynamicCast($3);
    if(en)
    {
	EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast($5);
	if(enumerators->v.empty())
	{
	    unit->error("enum `" + en->name() + "' must have at least one enumerator");
	}
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
|
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    $$ = ens;
}
;

// ----------------------------------------------------------------------
out_qualifier
// ----------------------------------------------------------------------
: ICE_OUT
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    $$ = out;
}
|
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    $$ = out;
}
;

// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
: // empty
{
}
| out_qualifier meta_data type_id
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($1);
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($3);
    TypePtr type = tsp->v.first;
    string ident = tsp->v.second;
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(ident, type, isOutParam->v);
	unit->currentContainer()->checkIntroduced(ident, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast($2);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
| parameters ',' out_qualifier meta_data type_id
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($3);
    TypeStringTokPtr tsp = TypeStringTokPtr::dynamicCast($5);
    TypePtr type = tsp->v.first;
    string ident = tsp->v.second;
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(ident, type, isOutParam->v);
	unit->currentContainer()->checkIntroduced(ident, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast($4);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
| out_qualifier meta_data type keyword
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($1);
    TypePtr type = TypePtr::dynamicCast($3);
    StringTokPtr ident = StringTokPtr::dynamicCast($4);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(ident->v, type, isOutParam->v);
	unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
| parameters ',' out_qualifier meta_data type keyword
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($3);
    TypePtr type = TypePtr::dynamicCast($5);
    StringTokPtr ident = StringTokPtr::dynamicCast($6);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(ident->v, type, isOutParam->v);
	unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
| out_qualifier meta_data type
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($1);
    TypePtr type = TypePtr::dynamicCast($3);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v);
	unit->error("missing parameter name");
    }
}
| parameters ',' out_qualifier meta_data type
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($3);
    TypePtr type = TypePtr::dynamicCast($5);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v);
	unit->error("missing parameter name");
    }
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
}
| ICE_SCOPE_DELIMITER ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    ident->v = "::" + ident->v;
    $$ = ident;
}
| scoped_name ICE_SCOPE_DELIMITER ICE_IDENTIFIER
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
    if(cont)
    {
	TypeList types = cont->lookupType(scoped->v);
	if(types.empty())
	{
	    YYERROR; // Can't continue, jump to next yyerrok
	}
	cont->checkIntroduced(scoped->v);
	$$ = types.front();
    }
    else
    {
        $$ = 0;
    }
}
| scoped_name '*'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    if(cont)
    {
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
	    cont->checkIntroduced(scoped->v);
	    if(cl->isLocal())
	    {
		unit->error("cannot create proxy for " + cl->kindOf() + " `" + cl->name() + "'");
	    }
	    *p = new Proxy(cl);
	}
	$$ = types.front();
    }
    else
    {
        $$ = 0;
    }
}
;

// ----------------------------------------------------------------------
string_literal
// ----------------------------------------------------------------------
: ICE_STRING_LITERAL string_literal // Adjacent string literals are concatenated
{
    StringTokPtr str1 = StringTokPtr::dynamicCast($1);
    StringTokPtr str2 = StringTokPtr::dynamicCast($2);
    str1->v += str2->v;
}
| ICE_STRING_LITERAL
{
}
;

// ----------------------------------------------------------------------
string_list
// ----------------------------------------------------------------------
: string_list ',' string_literal
{
    StringTokPtr str = StringTokPtr::dynamicCast($3);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast($1);
    stringList->v.push_back(str->v);
    $$ = stringList;
}
| string_literal
{
    StringTokPtr str = StringTokPtr::dynamicCast($1);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    $$ = stringList;
}
;

// ----------------------------------------------------------------------
local_qualifier
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
const_initializer
// ----------------------------------------------------------------------
: ICE_INTEGER_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast($1);
    ostringstream sstr;
    sstr << intVal->v;
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    basestring->v = pair<SyntaxTreeBasePtr,string>(type, sstr.str());
    $$ = basestring;
}
| ICE_FLOATING_POINT_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast($1);
    ostringstream sstr;
    sstr << floatVal->v;
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    basestring->v = pair<SyntaxTreeBasePtr,string>(type, sstr.str());
    $$ = basestring;
}
| scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v);
    if(cl.empty())
    {
    	basestring->v = pair<SyntaxTreeBasePtr,string>(TypePtr(0), scoped->v);
    }
    else
    {
	EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
	if(!enumerator)
	{
	    string msg = "illegal initializer: `" + scoped->v + "' is a";
	    static const string vowels = "aeiou";
	    string kindOf = cl.front()->kindOf();
	    if(vowels.find_first_of(kindOf[0]) != string::npos)
	    {
	    	msg += "n";
	    }
	    msg += " " + kindOf;
	    unit->error(msg);
	}
	unit->currentContainer()->checkIntroduced(scoped->v, enumerator);
	basestring->v = pair<SyntaxTreeBasePtr,string>(enumerator, scoped->v);
    }
    $$ = basestring;
}
| ICE_STRING_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    basestring->v = pair<SyntaxTreeBasePtr,string>(type, literal->v);
    $$ = basestring;
}
| ICE_FALSE
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    basestring->v = pair<SyntaxTreeBasePtr,string>(type, literal->v);
    $$ = basestring;
}
| ICE_TRUE
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    SyntaxTreeBaseStringTokPtr basestring = new SyntaxTreeBaseStringTok;
    basestring->v = pair<SyntaxTreeBasePtr,string>(type, literal->v);
    $$ = basestring;
}
;

// ----------------------------------------------------------------------
const_def
// ----------------------------------------------------------------------
: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($2);
    TypePtr const_type = TypePtr::dynamicCast($3);
    StringTokPtr ident = StringTokPtr::dynamicCast($4);
    SyntaxTreeBaseStringTokPtr value = SyntaxTreeBaseStringTokPtr::dynamicCast($6);
    $$ = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.first, value->v.second);
}
| ICE_CONST meta_data type '=' const_initializer
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($2);
    TypePtr const_type = TypePtr::dynamicCast($3);
    SyntaxTreeBaseStringTokPtr value = SyntaxTreeBaseStringTokPtr::dynamicCast($5);
    unit->error("missing constant name");
    $$ = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.first,
    					       value->v.second);
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
| ICE_CONST
{
}
| ICE_FALSE
{
}
| ICE_TRUE
{
}
| ICE_NONMUTATING
{
}
| ICE_IDEMPOTENT
{
}
;

%%
