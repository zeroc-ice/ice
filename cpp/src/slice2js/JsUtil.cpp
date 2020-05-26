//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <JsUtil.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

// TODO: fix this warning!
#if defined(_MSC_VER)
#   pragma warning(disable:4456) // shadow
#   pragma warning(disable:4457) // shadow
#   pragma warning(disable:4459) // shadow
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

const std::array<std::string, 17> builtinSuffixTable =
{
    "Bool",
    "Byte",
    "Short",
    "UShort",
    "Int",
    "UInt",
    "VarInt",
    "VarUInt",
    "Long",
    "ULong",
    "VarLong",
    "VarULong",
    "Float",
    "Double",
    "String",
    "Proxy",
    "Object"
};

}

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

    vector<string>::const_iterator i1 = tokens1.begin();
    vector<string>::const_iterator i2 = tokens2.begin();

    while(i1 != tokens1.end() && i2 != tokens2.end() && *i1 == *i2)
    {
        i1++;
        i2++;
    }

    //
    // Different volumes, relative path not possible.
    //
    if(i1 == tokens1.begin() && i2 == tokens2.begin())
    {
        return p1;
    }

    string newPath;
    if(i2 == tokens2.end())
    {
        newPath += "./";
        for (; i1 != tokens1.end(); ++i1)
        {
            newPath += *i1 + "/";
        }
    }
    else
    {
        for(vector<string>::difference_type i = tokens2.end() - i2; i > 0; i--)
        {
            newPath += "../";
        }

        for(; i1 != tokens1.end(); ++i1)
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
    static const string keywordList[] =
    {
        "await", "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do",
        "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", "implements", "import",
        "in", "instanceof", "interface", "let", "new", "null", "package", "private", "protected", "public", "return",
        "static", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with",
        "yield"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "_" + name;
    }

    return name;
}

string
Slice::JsGenerator::getModuleMetadata(const TypePtr& constType)
{
    TypePtr type = unwrapIfOptional(constType);

    static const std::array<std::string, 17> builtinModuleTable =
    {
        "",           // bool
        "",           // byte
        "",           // short
        "",           // short
        "",           // int
        "",           // int
        "",           // int
        "",           // int
        "ice",        // long
        "ice",        // long
        "ice",        // long
        "ice",        // long
        "",           // float
        "",           // double
        "",           // string
        "ice",        // Ice.ObjectPrx
        "ice"         // Ice.Value
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinModuleTable[builtin->kind()];
    }

    return getModuleMetadata(ContainedPtr::dynamicCast(type));
}

string
Slice::JsGenerator::getModuleMetadata(const ContainedPtr& p)
{
    //
    // Check if the file contains the js:module global metadata.
    //
    DefinitionContextPtr dc = p->definitionContext();
    assert(dc);
    const string prefix = "js:module:";
    const string value = dc->findMetaData(prefix);
    return value.empty() ? value : value.substr(prefix.size());
}

bool
Slice::JsGenerator::isClassType(const TypePtr& constType)
{
    TypePtr type = unwrapIfOptional(constType);
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return (builtin && builtin->usesClasses()) || ClassDeclPtr::dynamicCast(type);
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
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }

    auto ids = splitScopedName(name);
    transform(begin(ids), end(ids), begin(ids), lookupKwd);

    stringstream result;
    for(vector<string>::const_iterator j = ids.begin(); j != ids.end(); ++j)
    {
        if(j != ids.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::JsGenerator::fixId(const ContainedPtr& cont)
{
    return fixId(cont->name());
}

string
Slice::JsGenerator::importPrefix(const TypePtr& constType,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports)
{
    TypePtr type = unwrapIfOptional(constType);
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return typeToString(type, toplevel, imports, true);
    }
    else if(ContainedPtr::dynamicCast(type))
    {
        InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
        if(proxy)
        {
            return "iceNS0.";
        }
        else
        {
            return importPrefix(ContainedPtr::dynamicCast(type), toplevel, imports);
        }
    }
    return "";
}

string
Slice::JsGenerator::importPrefix(const ContainedPtr& contained,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports)
{
    string m1 = getModuleMetadata(contained);
    string m2 = getModuleMetadata(toplevel);

    string p;

    if(m1.empty())
    {
        string p1 = contained->definitionContext()->filename();
        string p2 = toplevel->definitionContext()->filename();

        p = relativePath(p1, p2);

        string::size_type pos = p.rfind('.');
        if (pos != string::npos)
        {
            p.erase(pos);
        }
    }
    else if(m1 == "ice" && m1 != m2)
    {
        return "iceNS0.";
    }
    else if(m1 != m2)
    {
        p = m1;
    }

    if(!p.empty())
    {
        for(vector<pair<string, string> >::const_iterator i = imports.begin(); i != imports.end(); ++i)
        {
            if(i->first == p)
            {
                return i->second + ".";
            }
        }
    }

    return "";
}

bool
Slice::JsGenerator::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); i++)
    {
        string s = *i;
        if(s.find(prefix) == 0)
        {
            value = s.substr(prefix.size());
            return true;
        }
    }
    return false;
}

string
Slice::JsGenerator::importPrefix(const string& type, const ContainedPtr& toplevel)
{
    const string module = getModuleMetadata(toplevel);
    return (type.find("Ice.") == 0 && module != "ice") ? "iceNS0." : "";
}

string
Slice::JsGenerator::getUnqualified(const string& type, const string& scope, const string& importPrefix)
{
    if(importPrefix.empty())
    {
        const string localScope = getLocalScope(scope) + ".";
        if(type.find(localScope) == 0)
        {
            string t = type.substr(localScope.size());
            if(t.find(".") == string::npos)
            {
                return t;
            }
        }
    }
    return type;
}

string
Slice::JsGenerator::typeToString(const TypePtr& constType,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports,
                                 bool typescript,
                                 bool definition)
{
    TypePtr type = unwrapIfOptional(constType);

    if(!type)
    {
        return "void";
    }

    static const std::array<std::string, 17> typeScriptBuiltinTable =
    {
        "boolean",          // bool
        "number",           // byte
        "number",           // short
        "number",           // ushort
        "number",           // int
        "number",           // uint
        "number",           // varint
        "number",           // varuint
        "Ice.Long",         // long
        "Ice.Long",         // ulong
        "Ice.Long",         // varlong
        "Ice.Long",         // varulong
        "number",           // float
        "number",           // double
        "string",
        "Ice.ObjectPrx",
        "Ice.Value"
    };

    static const std::array<std::string, 17> javaScriptBuiltinTable =
    {
        "Boolean",          // bool
        "Number",           // byte
        "Number",           // short
        "Number",           // ushort
        "Number",           // int
        "Number",           // uint
        "Number",           // varint
        "Number",           // varuint
        "Ice.Long",         // long
        "Ice.Long",         // ulong
        "Ice.Long",         // varlong
        "Ice.Long",         // varulong
        "Number",           // float
        "Number",           // double
        "String",
        "Ice.ObjectPrx",
        "Ice.Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(typescript)
        {
            ostringstream os;
            if(getModuleMetadata(type) == "ice" && getModuleMetadata(toplevel) != "ice")
            {
                os << "iceNS0.";
            }
            os << getUnqualified(typeScriptBuiltinTable[builtin->kind()], toplevel->scope(), "iceNS0.");
            return os.str();
        }
        else
        {
            return javaScriptBuiltinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        string prefix;
        ostringstream os;
        if(typescript)
        {
            prefix = importPrefix(ContainedPtr::dynamicCast(cl), toplevel, imports);
        }
        os << prefix;
        if(!prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(cl->scoped()), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(cl->scoped());
        }
        return os.str();
    }

    InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
    if(proxy)
    {
        ostringstream os;
        InterfaceDefPtr def = proxy->definition();

        string prefix;
        if (typescript)
        {
            prefix = importPrefix(ContainedPtr::dynamicCast(proxy), toplevel, imports);
            os << prefix;
        }

        if (prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(proxy->scoped() + "Prx"), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(proxy->scoped() + "Prx");
        }

        return os.str();
    }

    if(!typescript || definition)
    {
        SequencePtr seq = SequencePtr::dynamicCast(type);
        if (seq)
        {
            BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
            if (b && b->kind() == Builtin::KindByte)
            {
                return "Uint8Array";
            }
            else
            {
                return typeToString(seq->type(), toplevel, imports, typescript) + "[]";
            }
        }

        DictionaryPtr d = DictionaryPtr::dynamicCast(type);
        if(d)
        {
            const TypePtr keyType = d->keyType();
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(keyType);
            ostringstream os;
            if ((builtin && (builtin->kind() == Builtin::KindLong || builtin->kind() == Builtin::KindULong ||
                 builtin->kind() == Builtin::KindVarLong || builtin->kind() == Builtin::KindVarULong)) ||
                 StructPtr::dynamicCast(keyType))
            {
                const string prefix = importPrefix("Ice.HashMap", toplevel);
                os << prefix << getUnqualified("Ice.HashMap", toplevel->scope(), prefix);
            }
            else
            {
                os << "Map";
            }

            if (typescript)
            {
                os << "<"
                    << typeToString(keyType, toplevel, imports, true) << ", "
                    << typeToString(d->valueType(), toplevel, imports, true) << ">";
            }
            return os.str();
        }
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        ostringstream os;
        string prefix;
        if(typescript)
        {
            prefix = importPrefix(contained, toplevel, imports);
            os << prefix;
        }

        if(prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(contained->scoped()), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(contained->scoped());
        }
        return os.str();
    }

    assert(0);
    return "???";
}

string
Slice::JsGenerator::typeToString(const TypePtr& constType,
                                 const ContainedPtr& toplevel,
                                 const std::vector<std::pair<std::string, std::string> >& imports,
                                 bool typeScript,
                                 bool definition,
                                 bool usealias)
{
    TypePtr type = unwrapIfOptional(constType);
    string t = typeToString(type, toplevel, imports, typeScript, definition);
    if(usealias)
    {
        string m1 = getModuleMetadata(type);
        string m2 = getModuleMetadata(toplevel);
        if (!m1.empty() && m1 == m2)
        {
            // we are using the same module
            return t;
        }
        string p = importPrefix(type, toplevel, imports);

        //
        // When using an import prefix we don't need an alias, prefixes use iceNSXX that is reserved
        // name prefix
        //
        string::size_type i = t.find(".");
        if(p.empty() && i != string::npos)
        {
            const string scoped = fixId(toplevel->scoped()) + ".";
            if(scoped.find("." + t.substr(0, i + 1)) != string::npos)
            {
                replace(t.begin(), t.end(), '.', '_');
                t = "iceA_" + t;
            }
        }
    }
    return t;
}

string
Slice::JsGenerator::getLocalScope(const string& scope, const string& separator)
{
    assert(!scope.empty());

    //
    // Remove trailing "::" if present.
    //
    string fixedScope;
    if(scope[scope.size() - 1] == ':')
    {
        assert(scope[scope.size() - 2] == ':');
        fixedScope = scope.substr(0, scope.size() - 2);
    }
    else
    {
        fixedScope = scope;
    }

    if(fixedScope.empty())
    {
        return "";
    }
    auto ids = splitScopedName(fixedScope);
    transform(begin(ids), end(ids), begin(ids), lookupKwd);

    //
    // Return local scope for "::A::B::C" as A.B.C
    //
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        if(i != ids.begin())
        {
            result << separator;
        }
        result << *i;
    }
    return result.str();
}

void
Slice::JsGenerator::writeMarshalUnmarshalCode(Output& out,
                                              const TypePtr& constType,
                                              const string& param,
                                              bool marshal)
{
    TypePtr type = unwrapIfOptional(constType);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    // Builtin types that use classes are handled by isClassType below.
    if(builtin && !builtin->usesClasses())
    {
        if(marshal)
        {
            out << nl << "ostr.write" << builtinSuffixTable[builtin->kind()] << "(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << "istr.read" << builtinSuffixTable[builtin->kind()] << "();";
        }
        return;
    }

    if(EnumPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << "._write(ostr, " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << "._read(istr);";
        }
        return;
    }

    if(InterfaceDeclPtr::dynamicCast(type) || StructPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << ".write(ostr, " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << ".read(istr, " << param << ");";
        }
        return;
    }

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << "ostr.writeValue(" << param << ");";
        }
        else
        {
            out << nl << "istr.readValue(obj => " << param << " = obj, " << typeToString(type) << ");";
        }
        return;
    }

    if(SequencePtr::dynamicCast(type) || DictionaryPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << getHelper(type) <<".write(ostr, " << param << ");";
        }
        else
        {
            out << nl << param << " = " << getHelper(type) << ".read(istr);";
        }
        return;
    }

    assert(false);
}

void
Slice::JsGenerator::writeTaggedMarshalUnmarshalCode(Output& out,
                                                      const TypePtr& constType,
                                                      const string& param,
                                                      int tag,
                                                      bool marshal)
{
    TypePtr type = unwrapIfOptional(constType);
    string stream = marshal ? "ostr" : "istr";

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << stream << ".writeOptionalValue(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".readOptionalValue(" << tag << ", obj => " << param << " = obj, "
                << typeToString(type) << ");";
        }
        return;
    }

    if(EnumPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) <<"._writeOpt(" << stream << ", " << tag << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << "._readOpt(" << stream << ", " << tag << ");";
        }
        return;
    }

    if(marshal)
    {
        out << nl << getHelper(type) <<".writeOptional(" << stream << ", " << tag << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << getHelper(type) << ".readOptional(" << stream << ", " << tag << ");";
    }
}

std::string
Slice::JsGenerator::getHelper(const TypePtr& constType)
{
    TypePtr type = unwrapIfOptional(constType);
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObject)
        {
            return "Ice.ObjectPrx";
        }
        else
        {
            return "Ice." + builtinSuffixTable[builtin->kind()] + "Helper";
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return typeToString(type) + "._helper";
    }

    if(StructPtr::dynamicCast(type))
    {
        return typeToString(type);
    }

    InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
    if(proxy)
    {
        return typeToString(type);
    }

    if(SequencePtr::dynamicCast(type) || DictionaryPtr::dynamicCast(type))
    {
        stringstream s;
        s << getLocalScope(ContainedPtr::dynamicCast(type)->scoped()) << "Helper";
        return s.str();
    }

    if(ClassDeclPtr::dynamicCast(type))
    {
        return "Ice.ObjectHelper";
    }

    throw logic_error("");
}
