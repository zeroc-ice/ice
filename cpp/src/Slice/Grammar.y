%{

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/GrammarUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/UUID.h>
#include <cstring>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning(disable:4102)
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning(disable:4065)
// warning C4244: '=': conversion from 'int' to 'yytype_int16', possible loss of data
#   pragma warning(disable:4244)
#endif

//
// Avoid old style cast warnings in generated grammar
//
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

//
// Avoid clang conversion warnings
//
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
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

%pure-parser

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
%token ICE_CONST
%token ICE_FALSE
%token ICE_TRUE
%token ICE_IDEMPOTENT
%token ICE_OPTIONAL
%token ICE_VALUE

//
// Other tokens.
//
%token ICE_IDENTIFIER
%token ICE_SCOPED_IDENTIFIER
%token ICE_STRING_LITERAL
%token ICE_INTEGER_LITERAL
%token ICE_FLOATING_POINT_LITERAL
%token ICE_IDENT_OP
%token ICE_KEYWORD_OP
%token ICE_OPTIONAL_OP
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
opt_semicolon
// ----------------------------------------------------------------------
: ';'
{
}
|
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
}
definitions
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
opt_semicolon
| class_decl
{
    assert($1 == 0 || ClassDeclPtr::dynamicCast($1));
}
';'
| class_decl
{
    unit->error("`;' missing after class forward declaration");
}
| class_def
{
    assert($1 == 0 || ClassDefPtr::dynamicCast($1));
}
opt_semicolon
| interface_decl
{
    assert($1 == 0 || ClassDeclPtr::dynamicCast($1));
}
';'
| interface_decl
{
    unit->error("`;' missing after interface forward declaration");
}
| interface_def
{
    assert($1 == 0 || ClassDefPtr::dynamicCast($1));
}
opt_semicolon
| exception_decl
{
    assert($1 == 0);
}
';'
| exception_decl
{
    unit->error("`;' missing after exception forward declaration");
}
| exception_def
{
    assert($1 == 0 || ExceptionPtr::dynamicCast($1));
}
opt_semicolon
| struct_decl
{
    assert($1 == 0);
}
';'
| struct_decl
{
    unit->error("`;' missing after struct forward declaration");
}
| struct_def
{
    assert($1 == 0 || StructPtr::dynamicCast($1));
}
opt_semicolon
| sequence_def
{
    assert($1 == 0 || SequencePtr::dynamicCast($1));
}
';'
| sequence_def
{
    unit->error("`;' missing after sequence definition");
}
| dictionary_def
{
    assert($1 == 0 || DictionaryPtr::dynamicCast($1));
}
';'
| dictionary_def
{
    unit->error("`;' missing after dictionary definition");
}
| enum_def
{
    assert($1 == 0 || EnumPtr::dynamicCast($1));
}
opt_semicolon
| const_def
{
    assert($1 == 0 || ConstPtr::dynamicCast($1));
}
';'
| const_def
{
    unit->error("`;' missing after const definition");
}
| error ';'
{
    yyerrok;
}
;

// ----------------------------------------------------------------------
module_def
// ----------------------------------------------------------------------
: ICE_MODULE ICE_IDENTIFIER
{
    unit->setSeenDefinition();
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
    $$ = $2; // Dummy
}
;

// ----------------------------------------------------------------------
exception_decl
// ----------------------------------------------------------------------
: exception_id
{
    unit->error("exceptions cannot be forward declared");
    $$ = 0;
}
;

// ----------------------------------------------------------------------
exception_def
// ----------------------------------------------------------------------
: exception_id exception_extends
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ExceptionPtr base = ExceptionPtr::dynamicCast($2);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base);
    if(ex)
    {
        cont->checkIntroduced(ident->v, ex);
        unit->pushContainer(ex);
    }
    $$ = ex;
}
'{' exception_exports '}'
{
    if($3)
    {
        unit->popContainer();
    }
    $$ = $3;
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
optional
// ----------------------------------------------------------------------
: ICE_OPTIONAL_OP ICE_INTEGER_LITERAL ')'
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast($2);

    int tag;
    if(i->v < 0 || i->v > Int32Max)
    {
        unit->error("tag for optional is out of range");
        tag = -1;
    }
    else
    {
        tag = static_cast<int>(i->v);
    }

    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = tag >= 0;
    m->v.tag = tag;
    $$ = m;
}
| ICE_OPTIONAL_OP scoped_name ')'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($2);

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if(enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
            unit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
        }
        else if(enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for(EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << (*p)->scoped() << "'";
            }
            unit->error(os.str());
        }
        else
        {
            unit->error(string("`") + scoped->v + "' is not defined");
        }
    }

    if(cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int tag = -1;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
    ConstPtr constant = ConstPtr::dynamicCast(cl.front());
    if(constant)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
        if(b)
        {
            switch(b->kind())
            {
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
                if(l < 0 || l > Int32Max)
                {
                    unit->error("tag for optional is out of range");
                }
                tag = static_cast<int>(l);
                break;
            }
            default:
                break;
            }
        }
    }
    else if(enumerator)
    {
        tag = enumerator->value();
    }

    if(tag < 0)
    {
        unit->error("invalid tag `" + scoped->v + "' for optional");
    }

    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = tag >= 0;
    m->v.tag = tag;
    $$ = m;
}
| ICE_OPTIONAL_OP ')'
{
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    $$ = m;
}
| ICE_OPTIONAL
{
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    $$ = m;
}
;

// ----------------------------------------------------------------------
optional_type_id
// ----------------------------------------------------------------------
: optional type_id
{
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast($1);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($2);
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    $$ = m;
}
| type_id
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($1);
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    $$ = m;
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
    $$ = $2; // Dummy
}
;

// ----------------------------------------------------------------------
struct_decl
// ----------------------------------------------------------------------
: struct_id
{
    unit->error("structs cannot be forward declared");
    $$ = 0; // Dummy
}
;

// ----------------------------------------------------------------------
struct_def
// ----------------------------------------------------------------------
: struct_id
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v);
    if(st)
    {
        cont->checkIntroduced(ident->v, st);
        unit->pushContainer(st);
    }
    else
    {
        st = cont->createStruct(IceUtil::generateUUID()); // Dummy
        assert(st);
        unit->pushContainer(st);
    }
    $$ = st;
}
'{' struct_exports '}'
{
    if($2)
    {
        unit->popContainer();
    }
    $$ = $2;

    //
    // Empty structures are not allowed
    //
    StructPtr st = StructPtr::dynamicCast($$);
    assert(st);
    if(st->dataMembers().empty())
    {
        unit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
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
: struct_data_member
;

// ----------------------------------------------------------------------
class_name
// ----------------------------------------------------------------------
: ICE_CLASS ICE_IDENTIFIER
{
    $$ = $2;
}
| ICE_CLASS keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($2);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    $$ = $2; // Dummy
}
;

// ----------------------------------------------------------------------
class_id
// ----------------------------------------------------------------------
: ICE_CLASS ICE_IDENT_OP ICE_INTEGER_LITERAL ')'
{
    IceUtil::Int64 id = IntegerTokPtr::dynamicCast($3)->v;
    if(id < 0)
    {
        unit->error("invalid compact id for class: id must be a positive integer");
    }
    else if(id > Int32Max)
    {
        unit->error("invalid compact id for class: value is out of range");
    }
    else
    {
        string typeId = unit->getTypeId(static_cast<int>(id));
        if(!typeId.empty() && !unit->ignRedefs())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast($2)->v;
    classId->t = static_cast<int>(id);
    $$ = classId;
}
| ICE_CLASS ICE_IDENT_OP scoped_name ')'
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($3);

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if(enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
            unit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
        }
        else if(enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for(EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << (*p)->scoped() << "'";
            }
            unit->error(os.str());
        }
        else
        {
            unit->error(string("`") + scoped->v + "' is not defined");
        }
    }

    if(cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int id = -1;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
    ConstPtr constant = ConstPtr::dynamicCast(cl.front());
    if(constant)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
        if(b)
        {
            switch(b->kind())
            {
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
                if(l < 0 || l > Int32Max)
                {
                    unit->error("compact id for class is out of range");
                }
                id = static_cast<int>(l);
                break;
            }
            default:
                break;
            }
        }
    }
    else if(enumerator)
    {
        id = enumerator->value();
    }

    if(id < 0)
    {
        unit->error("invalid compact id for class: id must be a positive integer");
    }
    else
    {
        string typeId = unit->getTypeId(id);
        if(!typeId.empty() && !unit->ignRedefs())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast($2)->v;
    classId->t = id;
    $$ = classId;

}
| class_name
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast($1)->v;
    classId->t = -1;
    $$ = classId;
}
;

// ----------------------------------------------------------------------
class_decl
// ----------------------------------------------------------------------
: class_name
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    $$ = cl;
}
;

// ----------------------------------------------------------------------
class_def
// ----------------------------------------------------------------------
: class_id class_extends implements
{
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast($2);
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($3);
    if(base)
    {
        bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, false, bases->v);
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
        if(!cl || cl->isInterface())
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
: optional_type_id
{
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast($1);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    unit->currentContainer()->checkIntroduced(def->v.name, dm);
    $$ = dm;
}
| optional_type_id '=' const_initializer
{
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast($1);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast($3);

    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    unit->currentContainer()->checkIntroduced(def->v.name, dm);
    $$ = dm;
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        $$ = cl->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        $$ = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        $$ = ex->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
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
        $$ = cl->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        $$ = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        $$ = ex->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    assert($$);
    unit->error("missing data member name");
}
;

// ----------------------------------------------------------------------
struct_data_member
// ----------------------------------------------------------------------
: type_id
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($1);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    $$ = dm;
}
| type_id '=' const_initializer
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($1);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast($3);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v.value,
                                            value->v.valueAsString, value->v.valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    $$ = dm;
}
| optional type_id
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($2);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    $$ = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert($$);
    unit->error("optional data members not supported in struct");
}
| optional type_id '=' const_initializer
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast($2);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    $$ = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert($$);
    unit->error("optional data members not supported in struct");
}
| type keyword
{
    TypePtr type = TypePtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    $$ = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert($$);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
| type
{
    TypePtr type = TypePtr::dynamicCast($1);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    $$ = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert($$);
    unit->error("missing data member name");
}
;

// ----------------------------------------------------------------------
return_type
// ----------------------------------------------------------------------
: optional type
{
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast($1);
    m->v.type = TypePtr::dynamicCast($2);
    $$ = m;
}
| type
{
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast($1);
    m->v.optional = false;
    m->v.tag = -1;
    $$ = m;
}
| ICE_VOID
{
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    $$ = m;
}
;

// ----------------------------------------------------------------------
operation_preamble
// ----------------------------------------------------------------------
: return_type ICE_IDENT_OP
{
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
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
| ICE_IDEMPOTENT return_type ICE_IDENT_OP
{
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
                                                Operation::Idempotent);
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
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast($1);
    string name = StringTokPtr::dynamicCast($2)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
        if(op)
        {
            unit->pushContainer(op);
            unit->error("keyword `" + name + "' cannot be used as operation name");
            $$ = op; // Dummy
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
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast($2);
    string name = StringTokPtr::dynamicCast($3)->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
                                                Operation::Idempotent);
        if(op)
        {
            unit->pushContainer(op);
            unit->error("keyword `" + name + "' cannot be used as operation name");
            $$ = op; // Dummy
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
        op->setExceptionList(el->v); // Dummy
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
    $$ = $2; // Dummy
}
;

// ----------------------------------------------------------------------
interface_decl
// ----------------------------------------------------------------------
: interface_id
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    $$ = cl;
}
;

// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: interface_id interface_extends
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast($2);
    ClassDefPtr cl = cont->createClassDef(ident->v, -1, true, bases->v);
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
        unit->error(msg); // $$ is a dummy
    }
    else
    {
        ClassDefPtr def = cl->definition();
        if(!def)
        {
        string msg = "`";
        msg += scoped->v;
        msg += "' has been declared but not defined";
        unit->error(msg); // $$ is a dummy
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
    $$ = new ClassListTok; // Dummy
}
| ICE_VALUE
{
    unit->error("illegal inheritance from type Value");
    $$ = new ClassListTok; // Dummy
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
        exception = cont->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    $$ = exception;
}
| keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    $$ = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
;

// ----------------------------------------------------------------------
sequence_def
// ----------------------------------------------------------------------
: ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($6);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($3);
    TypePtr type = TypePtr::dynamicCast($4);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, metaData->v);
}
| ICE_SEQUENCE '<' meta_data type '>' keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($6);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($3);
    TypePtr type = TypePtr::dynamicCast($4);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
;

// ----------------------------------------------------------------------
dictionary_def
// ----------------------------------------------------------------------
: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER
{
    StringTokPtr ident = StringTokPtr::dynamicCast($9);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast($3);
    TypePtr keyType = TypePtr::dynamicCast($4);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast($6);
    TypePtr valueType = TypePtr::dynamicCast($7);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
| ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($9);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast($3);
    TypePtr keyType = TypePtr::dynamicCast($4);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast($6);
    TypePtr valueType = TypePtr::dynamicCast($7);
    ContainerPtr cont = unit->currentContainer();
    $$ = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v); // Dummy
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
    $$ = $2; // Dummy
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: enum_id
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v);
    if(en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    }
    unit->pushContainer(en);
    $$ = en;
}
'{' enumerator_list '}'
{
    EnumPtr en = EnumPtr::dynamicCast($2);
    if(en)
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast($4);
        if(enumerators->v.empty())
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    $$ = $2;
}
|
ICE_ENUM
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    $$ = en;
}
'{' enumerator_list '}'
{
    unit->popContainer();
    $$ = $1;
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
| ICE_IDENTIFIER '=' enumerator_initializer
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast($3);
    if(intVal)
    {
        if(intVal->v < 0 || intVal->v > Int32Max)
        {
            unit->error("value for enumerator `" + ident->v + "' is out of range");
        }
        else
        {
            EnumeratorPtr en = cont->createEnumerator(ident->v, static_cast<int>(intVal->v));
            ens->v.push_front(en);
        }
    }
    $$ = ens;
}
| keyword
{
    StringTokPtr ident = StringTokPtr::dynamicCast($1);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    $$ = ens;
}
|
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    $$ = ens; // Dummy
}
;

// ----------------------------------------------------------------------
enumerator_initializer
// ----------------------------------------------------------------------
: ICE_INTEGER_LITERAL
{
    $$ = $1;
}
| scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v);
    IntegerTokPtr tok;
    if(!cl.empty())
    {
        ConstPtr constant = ConstPtr::dynamicCast(cl.front());
        if(constant)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, constant);
            BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
            if(b && (b->kind() == Builtin::KindByte || b->kind() == Builtin::KindShort ||
                     b->kind() == Builtin::KindInt || b->kind() == Builtin::KindLong))
            {
                IceUtil::Int64 v;
                if(IceUtilInternal::stringToInt64(constant->value(), v))
                {
                    tok = new IntegerTok;
                    tok->v = v;
                    tok->literal = constant->value();
                }
            }
        }
    }

    if(!tok)
    {
        string msg = "illegal initializer: `" + scoped->v + "' is not an integer constant";
        unit->error(msg); // $$ is dummy
    }

    $$ = tok;
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
| out_qualifier meta_data optional_type_id
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($1);
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast($3);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
        unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast($2);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
| parameters ',' out_qualifier meta_data optional_type_id
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast($3);
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast($5);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
        unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
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
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
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
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
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
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
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
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
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
| ICE_SCOPED_IDENTIFIER
{
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
| ICE_VALUE
{
    $$ = unit->builtin(Builtin::KindValue);
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
const_initializer
// ----------------------------------------------------------------------
: ICE_INTEGER_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast($1);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = intVal->literal;
    $$ = def;
}
| ICE_FLOATING_POINT_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast($1);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = floatVal->literal;
    $$ = def;
}
| scoped_name
{
    StringTokPtr scoped = StringTokPtr::dynamicCast($1);
    ConstDefTokPtr def = new ConstDefTok;
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        // Could be an enumerator
        def->v.type = TypePtr(0);
        def->v.value = SyntaxTreeBasePtr(0);
        def->v.valueAsString = scoped->v;
        def->v.valueAsLiteral = scoped->v;
    }
    else
    {
        EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
        ConstPtr constant = ConstPtr::dynamicCast(cl.front());
        if(enumerator)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, enumerator);
            def->v.type = enumerator->type();
            def->v.value = enumerator;
            def->v.valueAsString = scoped->v;
            def->v.valueAsLiteral = scoped->v;
        }
        else if(constant)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, constant);
            def->v.value = constant;
            def->v.valueAsString = constant->value();
            def->v.valueAsLiteral = constant->value();
        }
        else
        {
            string msg = "illegal initializer: `" + scoped->v + "' is a";
            static const string vowels = "aeiou";
            string kindOf = cl.front()->kindOf();
            if(vowels.find_first_of(kindOf[0]) != string::npos)
            {
                msg += "n";
            }
            msg += " " + kindOf;
            unit->error(msg); // $$ is dummy
        }
    }
    $$ = def;
}
| ICE_STRING_LITERAL
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = literal->v;
    def->v.valueAsLiteral = literal->literal;
    $$ = def;
}
| ICE_FALSE
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "false";
    def->v.valueAsLiteral = "false";
    $$ = def;
}
| ICE_TRUE
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast($1);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "true";
    def->v.valueAsLiteral = "true";
    $$ = def;
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
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast($6);
    $$ = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
}
| ICE_CONST meta_data type '=' const_initializer
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast($2);
    TypePtr const_type = TypePtr::dynamicCast($3);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast($5);
    unit->error("missing constant name");
    $$ = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
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
| ICE_CONST
{
}
| ICE_FALSE
{
}
| ICE_TRUE
{
}
| ICE_IDEMPOTENT
{
}
| ICE_OPTIONAL
{
}
| ICE_VALUE
{
}
;

%%
