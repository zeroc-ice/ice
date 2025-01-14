//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "JsUtil.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

string
Slice::relativePath(const string& p1, const string& p2)
{
    vector<string> tokens1;
    vector<string> tokens2;

    splitString(p1, "/\\", tokens1);
    splitString(p2, "/\\", tokens2);

    string f1 = tokens1.back();
    string f2 = tokens2.back();

    tokens1.pop_back();
    tokens2.pop_back();

    auto i1 = tokens1.begin();
    auto i2 = tokens2.begin();

    while (i1 != tokens1.end() && i2 != tokens2.end() && *i1 == *i2)
    {
        i1++;
        i2++;
    }

    //
    // Different volumes, relative path not possible.
    //
    if (i1 == tokens1.begin() && i2 == tokens2.begin())
    {
        return p1;
    }

    string newPath;
    if (i2 == tokens2.end())
    {
        newPath += "./";
        for (; i1 != tokens1.end(); ++i1)
        {
            newPath += *i1 + "/";
        }
    }
    else
    {
        for (vector<string>::difference_type i = tokens2.end() - i2; i > 0; i--)
        {
            newPath += "../";
        }

        for (; i1 != tokens1.end(); ++i1)
        {
            newPath += *i1 + "/";
        }
    }
    newPath += f1;

    return newPath;
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] = {
        "await",     "break",  "case",     "catch",  "class",      "const",   "continue",  "debugger",
        "default",   "delete", "do",       "else",   "enum",       "export",  "extends",   "false",
        "finally",   "for",    "function", "if",     "implements", "import",  "in",        "instanceof",
        "interface", "let",    "new",      "null",   "package",    "private", "protected", "public",
        "return",    "static", "super",    "switch", "this",       "throw",   "true",      "try",
        "typeof",    "var",    "void",     "while",  "with",       "yield"};
    bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
    if (found)
    {
        return "_" + name;
    }

    return name;
}

static vector<string>
fixIds(const vector<string>& ids)
{
    vector<string> newIds;
    for (const auto& id : ids)
    {
        newIds.push_back(lookupKwd(id));
    }
    return newIds;
}

string
Slice::getJavaScriptModuleForType(const TypePtr& type)
{
    static const char* builtinModuleTable[] = {
        "",    // byte
        "",    // bool
        "",    // short
        "",    // int
        "ice", // long
        "",    // float
        "",    // double
        "",    // string
        "ice", // Ice.Value
        "ice", // Ice.ObjectPrx
        "ice"  // Ice.Object
    };

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        return builtinModuleTable[builtin->kind()];
    }

    return getJavaScriptModule(dynamic_pointer_cast<Contained>(type)->definitionContext());
}

string
Slice::getJavaScriptModule(const DefinitionContextPtr& dc)
{
    // Check if the file contains the 'js:module' file metadata.
    assert(dc);
    return dc->getMetadataArgs("js:module").value_or("");
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are JS keywords replaced by
// their "_"-prefixed version; otherwise, if the passed name is
// not scoped, but a JS keyword, return the "_"-prefixed name.
//
string
Slice::JsGenerator::fixId(const string& name)
{
    if (name.empty())
    {
        return name;
    }
    if (name[0] != ':')
    {
        return lookupKwd(name);
    }

    const vector<string> ids = splitScopedName(name);
    const vector<string> newIds = fixIds(ids);

    stringstream result;
    for (auto j = newIds.begin(); j != newIds.end(); ++j)
    {
        if (j != newIds.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::JsGenerator::fixDataMemberName(const std::string& name, bool isStruct, bool isLegalKeyType)
{
    if (name == "constructor")
    {
        return "_constructor";
    }
    else if (isStruct && (name == "equals" || name == "clone" || (isLegalKeyType && name == "hashCode")))
    {
        return "_" + name;
    }
    return Slice::JsGenerator::fixId(name);
}

string
Slice::JsGenerator::getUnqualified(const string& type, const string& scope, const string& importPrefix)
{
    if (importPrefix.empty())
    {
        const string localScope = getLocalScope(scope) + ".";
        if (type.find(localScope) == 0)
        {
            string t = type.substr(localScope.size());
            if (t.find(".") == string::npos)
            {
                return t;
            }
        }
    }
    return type;
}

string
Slice::JsGenerator::typeToJsString(const TypePtr& type, bool definition)
{
    if (!type)
    {
        return "void";
    }

    static const char* javaScriptBuiltinTable[] = {
        "Number",   // byte
        "Boolean",  // bool
        "Number",   // short
        "Number",   // int
        "Ice.Long", // long
        "Number",   // float
        "Number",   // double
        "String",
        "Ice.Value",
        "Ice.ObjectPrx",
        "Ice.Value"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        return javaScriptBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return fixId(cl->scoped());
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return fixId(proxy->scoped() + "Prx");
    }

    if (definition)
    {
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            builtin = dynamic_pointer_cast<Builtin>(seq->type());
            if (builtin && builtin->kind() == Builtin::KindByte)
            {
                return "Uint8Array";
            }
            else
            {
                return typeToJsString(seq->type()) + "[]";
            }
        }

        DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
        if (d)
        {
            const TypePtr keyType = d->keyType();
            builtin = dynamic_pointer_cast<Builtin>(keyType);
            ostringstream os;
            if ((builtin && builtin->kind() == Builtin::KindLong) || dynamic_pointer_cast<Struct>(keyType))
            {
                os << "Ice.HashMap";
            }
            else
            {
                os << "Map";
            }
            return os.str();
        }
    }

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (contained)
    {
        return fixId(contained->scoped());
    }

    return "???";
}

string
Slice::JsGenerator::getLocalScope(const string& scope, const string& separator)
{
    assert(!scope.empty());

    //
    // Remove trailing "::" if present.
    //
    string fixedScope;
    if (scope[scope.size() - 1] == ':')
    {
        assert(scope[scope.size() - 2] == ':');
        fixedScope = scope.substr(0, scope.size() - 2);
    }
    else
    {
        fixedScope = scope;
    }

    if (fixedScope.empty())
    {
        return "";
    }
    const vector<string> ids = fixIds(splitScopedName(fixedScope));

    //
    // Return local scope for "::A::B::C" as A.B.C
    //
    stringstream result;
    for (auto i = ids.begin(); i != ids.end(); ++i)
    {
        if (i != ids.begin())
        {
            result << separator;
        }
        result << *i;
    }
    return result.str();
}

void
Slice::JsGenerator::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal)
{
    string stream = marshal ? "ostr" : "istr";

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeByte(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()" << ';';
                }
                return;
            }
            case Builtin::KindBool:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()" << ';';
                }
                return;
            }
            case Builtin::KindShort:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()" << ';';
                }
                return;
            }
            case Builtin::KindInt:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()" << ';';
                }
                return;
            }
            case Builtin::KindLong:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()" << ';';
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()" << ';';
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()" << ';';
                }
                return;
            }
            case Builtin::KindString:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeString(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()" << ';';
                }
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                // Handled by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy();";
                }
                return;
            }
        }
    }

    if (dynamic_pointer_cast<Enum>(type))
    {
        if (marshal)
        {
            out << nl << typeToJsString(type) << "._write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToJsString(type) << "._read(" << stream << ");";
        }
        return;
    }

    if (dynamic_pointer_cast<InterfaceDecl>(type))
    {
        if (marshal)
        {
            out << nl << stream << ".writeProxy(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << stream << ".readProxy();";
        }
        return;
    }

    if (dynamic_pointer_cast<Struct>(type))
    {
        if (marshal)
        {
            out << nl << typeToJsString(type) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToJsString(type) << ".read(" << stream << ", " << param << ");";
        }
        return;
    }

    if (type->isClassType())
    {
        if (marshal)
        {
            out << nl << stream << ".writeValue(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readValue(obj => " << param << " = obj, Ice.TypeRegistry.getValueType(\""
                << typeToJsString(type) << "\"));";
        }
        return;
    }

    if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        if (marshal)
        {
            out << nl << getHelper(type) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << getHelper(type) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
Slice::JsGenerator::writeOptionalMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& param,
    int tag,
    bool marshal)
{
    assert(!type->isClassType()); // Optional classes are disallowed by the parser.

    string stream = marshal ? "ostr" : "istr";

    if (dynamic_pointer_cast<Enum>(type))
    {
        if (marshal)
        {
            out << nl << typeToJsString(type) << "._writeOpt(" << stream << ", " << tag << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToJsString(type) << "._readOpt(" << stream << ", " << tag << ");";
        }
        return;
    }

    if (marshal)
    {
        out << nl << getHelper(type) << ".writeOptional(" << stream << ", " << tag << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << getHelper(type) << ".readOptional(" << stream << ", " << tag << ");";
    }
}

std::string
Slice::JsGenerator::getHelper(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                return "Ice.ByteHelper";
            }
            case Builtin::KindBool:
            {
                return "Ice.BoolHelper";
            }
            case Builtin::KindShort:
            {
                return "Ice.ShortHelper";
            }
            case Builtin::KindInt:
            {
                return "Ice.IntHelper";
            }
            case Builtin::KindLong:
            {
                return "Ice.LongHelper";
            }
            case Builtin::KindFloat:
            {
                return "Ice.FloatHelper";
            }
            case Builtin::KindDouble:
            {
                return "Ice.DoubleHelper";
            }
            case Builtin::KindString:
            {
                return "Ice.StringHelper";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return "Ice.ObjectHelper";
            }
            case Builtin::KindObjectProxy:
            {
                return "Ice.ObjectPrx";
            }
        }
    }

    if (dynamic_pointer_cast<Enum>(type))
    {
        return typeToJsString(type) + "._helper";
    }

    if (dynamic_pointer_cast<Struct>(type))
    {
        return typeToJsString(type);
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        return typeToJsString(type);
    }

    if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        stringstream s;
        s << getLocalScope(dynamic_pointer_cast<Contained>(type)->scoped()) << "Helper";
        return s.str();
    }

    if (dynamic_pointer_cast<ClassDecl>(type))
    {
        return "Ice.ObjectHelper";
    }

    assert(false);
    return "???";
}
