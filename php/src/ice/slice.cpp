// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ice_slice.h"
#include "ice_marshal.h"
#include "ice_util.h"

#include <Slice/Preprocessor.h>

using namespace std;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// In order for a script to have access to Slice types, the classes for those types must be
// defined prior to script execution. We initially tried creating internal classes for
// the Slice types, because we would only have to create them once, during module startup.
// However, we eventually realized that these classes should not be internal classes - the
// extension is not implementing these classes, it is simply defining them. In other words,
// the extension needs to create classes just like PHP's parser does.
//
// To do this, we descend the Slice parse tree immediately after parsing and "generate" PHP
// code into a string. Eventually, the script will invoke the Ice_compileSlice global function,
// at which point we "compile" all of the code we generated previously by having the PHP
// interpreter evaluate it.
//

//
// CodeVisitor descends the Slice parse tree and generates PHP code for certain Slice types.
//
class CodeVisitor : public Slice::ParserVisitor
{
public:
    CodeVisitor(ostream&);

    virtual void visitClassDecl(const Slice::ClassDeclPtr&);
    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual void visitClassDefEnd(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual void visitExceptionEnd(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitStructEnd(const Slice::StructPtr&);
    virtual void visitOperation(const Slice::OperationPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:
    string getTypeHint(const Slice::TypePtr&);

    ostream& _out;
};

//
// The Slice parse tree. This is static because the Slice files are parsed only once,
// when the module is loaded.
//
static Slice::UnitPtr _unit;

//
// The code we've generated from the Slice definitions.
//
static string _code;

//
// This map associates a flattened type id to a ClassDef. It is populated by CodeVisitor.
//
static map<string, Slice::ClassDefPtr> _classDefs;

//
// This PHP code defines the core types we need. We supply a few of the common
// local exception subclasses; all other local exceptions are mapped to
// UnknownLocalException. We don't define Ice::Identity here because it's
// possible the user will have included its definition (see Slice_init).
//
static const char* _coreTypes =
    "define(\"ICE_STRING_VERSION\", \"1.1.0\");\n"
    "define(\"ICE_INT_VERSION\", 10100);\n"
    "\n"
    "abstract class Ice_LocalException\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_UnknownException extends Ice_LocalException\n"
    "{\n"
    "    var $unknown;\n"
    "}\n"
    "\n"
    "class Ice_UnknownLocalException extends Ice_UnknownException\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_UnknownUserException extends Ice_UnknownException\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_RequestFailedException extends Ice_LocalException\n"
    "{\n"
    "    var $id;\n"
    "    var $facet;\n"
    "    var $operation;\n"
    "}\n"
    "\n"
    "class Ice_ObjectNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_FacetNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_OperationNotExistException extends Ice_RequestFailedException\n"
    "{\n"
    "}\n"
    "\n"
    "abstract class Ice_UserException\n"
    "{\n"
    "}\n"
    "\n"
    "interface Ice_LocalObject\n"
    "{\n"
    "}\n"
    "\n"
    "interface Ice_Object\n"
    "{\n"
    "}\n"
    "\n"
    "class Ice_ObjectImpl implements Ice_Object\n"
    "{\n"
    "    var $ice_facets = array();\n"
    "}\n"
;

//
// Parse the Slice files that define the types and operations available to a PHP script.
//
static bool
parseSlice(const string& argStr)
{
    vector<string> args;
    if(!ice_splitString(argStr, args))
    {
        return false;
    }

    string cppArgs;
    vector<string> files;
    bool debug = false;
    bool ice = true; // This must be true so that we can create Ice::Identity when necessary
    bool caseSensitive = false;

    vector<string>::const_iterator p;
    for(p = args.begin(); p != args.end(); ++p)
    {
        string arg = *p;
        if(arg.substr(0, 2) == "-I" || arg.substr(0, 2) == "-D" || arg.substr(0, 2) == "-U")
        {
            cppArgs += ' ';
            if(arg.find(' ') != string::npos)
            {
                cppArgs += "'";
                cppArgs += arg;
                cppArgs += "'";
            }
            else
            {
                cppArgs += arg;
            }
        }
        else if(arg == "--case-sensitive")
        {
            caseSensitive = true;
        }
        else if(arg[0] == '-')
        {
            zend_error(E_ERROR, "unknown option `%s' in ice.parse", arg.c_str());
            return false;
        }
        else
        {
            files.push_back(arg);
        }
    }

    if(files.empty())
    {
        zend_error(E_ERROR, "no Slice files specified in ice.parse");
        return false;
    }

    bool ignoreRedefs = false;
    bool all = true;
    _unit = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
    bool status = true;

    for(p = files.begin(); p != files.end(); ++p)
    {
        Slice::Preprocessor icecpp("icecpp", *p, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            status = false;
            break;
        }

        int parseStatus = _unit->parse(cppHandle, debug);

        if(!icecpp.close())
        {
            status = false;
            break;
        }

        if(parseStatus == EXIT_FAILURE)
        {
            status = false;
            break;
        }
    }

    return status;
}

bool
Slice_init(TSRMLS_D)
{
    //
    // Parse the Slice files.
    //
    char* parse = INI_STR("ice.parse");
    if(parse && strlen(parse) > 0)
    {
        if(!parseSlice(parse))
        {
            return false;
        }
    }
    else
    {
        //
        // We must be allowed to obtain builtin types, as well as create Ice::Identity if necessary.
        //
        _unit = Slice::Unit::createUnit(false, false, true, false);
    }

    //
    // Create the Slice definition for Ice::Identity if it doesn't exist. The PHP class will
    // be created automatically by CodeVisitor.
    //
    string scoped = "::Ice::Identity";
    Slice::TypeList l = _unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::ContainedList c = _unit->lookupContained("Ice", false);
        Slice::ModulePtr module;
        if(c.empty())
        {
            module = _unit->createModule("Ice");
        }
        else
        {
            module = Slice::ModulePtr::dynamicCast(c.front());
            if(!module)
            {
                zend_error(E_ERROR, "the symbol `::Ice' is defined in Slice but is not a module");
                return false;
            }
        }
        Slice::StructPtr identity = module->createStruct("Identity", false);
        Slice::TypePtr str = _unit->builtin(Slice::Builtin::KindString);
        identity->createDataMember("category", str);
        identity->createDataMember("name", str);
    }

    //
    // Descend the parse tree, create PHP code, and store it in the _code map.
    //
    ostringstream out;
    CodeVisitor visitor(out);
    _unit->visit(&visitor);
    _code = out.str();

    return true;
}

bool
Slice_shutdown(TSRMLS_D)
{
    if(_unit)
    {
        try
        {
            _unit->destroy();
            _unit = 0;
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            zend_error(E_ERROR, "unable to destroy Slice parse tree:\n%s", ostr.str().c_str());
            return false;
        }
    }

    return true;
}

Slice::UnitPtr
Slice_getUnit()
{
    return _unit;
}

bool
Slice_isNativeKey(const Slice::TypePtr& type)
{
    //
    // PHP's native associative array supports only integer and string types for the key.
    // For Slice dictionaries that meet this criteria, we use the native array type.
    //
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindBool: // We allow bool even though PHP doesn't support it directly.
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        case Slice::Builtin::KindString:
            return true;

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            break;
        }
    }

    return false;
}

Slice::ClassDefPtr
Slice_getClassDef(const string& flat)
{
    return _classDefs[ice_lowerCase(flat)];
}

ZEND_FUNCTION(Ice_compileSlice)
{
    if(!ICE_G(coreTypesLoaded))
    {
        if(zend_eval_string(const_cast<char*>(_coreTypes), NULL, "__core" TSRMLS_CC) == FAILURE)
        {
            zend_error(E_ERROR, "unable to create core types");
            return;
        }
        ICE_G(coreTypesLoaded) = 1;
    }

    if(zend_eval_string(const_cast<char*>(_code.c_str()), NULL, "__slice" TSRMLS_CC) == FAILURE)
    {
        zend_error(E_ERROR, "unable to create Slice types:\n%s", _code.c_str());
        return;
    }
}

CodeVisitor::CodeVisitor(ostream& out) :
    _out(out)
{
}

void
CodeVisitor::visitClassDecl(const Slice::ClassDeclPtr& p)
{
    Slice::ClassDefPtr def = p->definition();
    if(!def)
    {
        string scoped = p->scoped();
        zend_error(E_WARNING, "%s %s declared but not defined", p->isInterface() ? "interface" : "class",
                   scoped.c_str());
    }
}

bool
CodeVisitor::visitClassDefStart(const Slice::ClassDefPtr& p)
{
    string flat = ice_flatten(p->scoped());

    _classDefs[ice_lowerCase(flat)] = p;

    Slice::ClassList bases = p->bases();

    if(p->isInterface())
    {
        _out << "interface " << flat << " implements ";
        if(!bases.empty())
        {
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ",";
                }
                _out << ice_flatten((*q)->scoped());
            }
        }
        else
        {
            _out << "Ice_Object";
        }
    }
    else
    {
        if(p->isAbstract())
        {
            _out << "abstract ";
        }
        _out << "class " << flat << " extends ";
        if(!bases.empty() && !bases.front()->isInterface())
        {
            _out << ice_flatten(bases.front()->scoped());
            bases.pop_front();
        }
        else
        {
            _out << "Ice_ObjectImpl";
        }
        if(!bases.empty())
        {
            _out << " implements ";
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ",";
                }
                _out << ice_flatten((*q)->scoped());
            }
        }
    }

    _out << endl << '{' << endl;

    return true;
}

void
CodeVisitor::visitClassDefEnd(const Slice::ClassDefPtr& p)
{
    _out << '}' << endl;
}

bool
CodeVisitor::visitExceptionStart(const Slice::ExceptionPtr& p)
{
    string flat = ice_flatten(p->scoped());
    Slice::ExceptionPtr base = p->base();

    _out << "class " << flat << " extends ";
    if(!base)
    {
        if(p->isLocal())
        {
            _out << "Ice_LocalException";
        }
        else
        {
            _out << "Ice_UserException";
        }
    }
    else
    {
        _out << ice_flatten(base->scoped());
    }

    _out << endl << '{' << endl;

    return true;
}

void
CodeVisitor::visitExceptionEnd(const Slice::ExceptionPtr& p)
{
    _out << '}' << endl;
}

bool
CodeVisitor::visitStructStart(const Slice::StructPtr& p)
{
    string flat = ice_flatten(p->scoped());

    _out << "class " << ice_flatten(p->scoped()) << endl;
    _out << '{' << endl;

    return true;
}

void
CodeVisitor::visitStructEnd(const Slice::StructPtr& p)
{
    _out << '}' << endl;
}

void
CodeVisitor::visitOperation(const Slice::OperationPtr& p)
{
    string name = ice_fixIdent(p->name());

    Slice::ParamDeclList params = p->parameters();

    Slice::ClassDefPtr cl = Slice::ClassDefPtr::dynamicCast(p->container());
    assert(cl);

    if(!cl->isInterface())
    {
        _out << "abstract public ";
    }
    _out << "function " << name << '(';
    for(Slice::ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
    {
        Slice::ParamDeclPtr param = *q;
        if(q != params.begin())
        {
            _out << ", ";
        }
        if(param->isOutParam())
        {
            _out << '&';
        }
        else
        {
            string hint = getTypeHint(param->type());
            if(!hint.empty())
            {
                _out << hint << ' ';
            }
        }
        _out << '$' << ice_fixIdent(param->name());
    }
    _out << ");" << endl;
}

void
CodeVisitor::visitDataMember(const Slice::DataMemberPtr& p)
{
    _out << "var $" << ice_fixIdent(p->name()) << ';' << endl;
}

void
CodeVisitor::visitDictionary(const Slice::DictionaryPtr& p)
{
    Slice::TypePtr keyType = p->keyType();
    if(!Slice_isNativeKey(keyType))
    {
        //
        // TODO: Generate class.
        //
        string scoped = p->scoped();
        zend_error(E_WARNING, "skipping dictionary %s - unsupported key type", scoped.c_str());
    }
}

void
CodeVisitor::visitEnum(const Slice::EnumPtr& p)
{
    string flat = ice_flatten(p->scoped());

    _out << "class " << flat << endl;
    _out << '{' << endl;

    //
    // Create a class constant for each enumerator.
    //
    Slice::EnumeratorList l = p->getEnumerators();
    Slice::EnumeratorList::const_iterator q;
    long i;
    for(q = l.begin(), i = 0; q != l.end(); ++q, ++i)
    {
        string name = ice_fixIdent((*q)->name());
        _out << "const " << ice_fixIdent((*q)->name()) << " = " << i << ';' << endl;
    }

    _out << '}' << endl;
}

void
CodeVisitor::visitConst(const Slice::ConstPtr& p)
{
    string flat = ice_flatten(p->scoped());
    Slice::TypePtr type = p->type();
    string value = p->value();

    _out << "define(\"" << flat << "\", ";

    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindBool:
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
            _out << value;
            break;

        case Slice::Builtin::KindLong:
        {
            IceUtil::Int64 l;
            string::size_type pos;
            IceUtil::stringToInt64(value, l, pos);
            //
            // The platform's 'long' type may not be 64 bits, so we store 64-bit
            // values as a string.
            //
            if(sizeof(IceUtil::Int64) > sizeof(long) && (l < LONG_MIN || l > LONG_MAX))
            {
                _out << "\"" << value << "\";";
            }
            else
            {
                _out << value;
            }
            break;
        }

        case Slice::Builtin::KindString:
        {
            //
            // Expand strings into the basic source character set. We can't use isalpha() and the like
            // here because they are sensitive to the current locale.
            //
            static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "0123456789"
                                                   "_{}[]#()<>%:;,?*+=/^&|~!=,\\' \t";
            static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

            _out << "\"";                                        // Opening "

            ios_base::fmtflags originalFlags = _out.flags();        // Save stream state
            streamsize originalWidth = _out.width();
            ostream::char_type originalFill = _out.fill();

            for(string::const_iterator c = value.begin(); c != value.end(); ++c)
            {
                if(*c == '$')
                {
                    _out << "\\$";
                }
                else if(*c == '"')
                {
                    _out << "\\\"";
                }
                else if(charSet.find(*c) == charSet.end())
                {
                    unsigned char uc = *c;                    // char may be signed, so make it positive
                    _out << "\\";                                // Print as octal if not in basic source character set
                    _out.flags(ios_base::oct);
                    _out.width(3);
                    _out.fill('0');
                    _out << static_cast<unsigned>(uc);
                }
                else
                {
                    _out << *c;                                  // Print normally if in basic source character set
                }
            }

            _out.fill(originalFill);                             // Restore stream state
            _out.width(originalWidth);
            _out.flags(originalFlags);

            _out << "\"";                                        // Closing "

            break;
        }

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            assert(false);
        }

        _out << ");" << endl;
        return;
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        string::size_type colon = value.rfind(':');
        if(colon != string::npos)
        {
            value = value.substr(colon + 1);
        }
        Slice::EnumeratorList l = en->getEnumerators();
        Slice::EnumeratorList::iterator q;
        for(q = l.begin(); q != l.end(); ++q)
        {
            if((*q)->name() == value)
            {
                _out << ice_flatten(en->scoped()) << "::" << ice_fixIdent(value) << ");" << endl;
                return;
            }
        }
        assert(false); // No match found.
    }
}

string
CodeVisitor::getTypeHint(const Slice::TypePtr& type)
{
    //
    // Currently, the Zend engine does not allow an argument with a type hint to have
    // a value of null, therefore we can only use type hints for structs.
    //
    Slice::StructPtr st = Slice::StructPtr::dynamicCast(type);
    if(st)
    {
        return ice_flatten(st->scoped());
    }

    return string();
}
