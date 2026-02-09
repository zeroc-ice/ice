// Copyright (c) ZeroC, Inc.

#include "JsUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <regex>

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
Slice::JavaScript::getJavaScriptModule(const DefinitionContextPtr& dc)
{
    // Check if the file contains the 'js:module' file metadata.
    assert(dc);
    return dc->getMetadataArgs("js:module").value_or("");
}

string
Slice::JavaScript::importPathToIdentifier(const string& path)
{
    string identifier = path;

    // Strip file extensions for relative paths (normalized by toRelativePath to start with "./").
    // For module names like "foo" or "@zeroc/ice", there is no extension to strip.
    if (identifier.size() > 2 && identifier[0] == '.' && identifier[1] == '/')
    {
        static constexpr string_view extensions[] = {".ice", ".js"};
        for (string_view ext : extensions)
        {
            if (identifier.size() > ext.size() &&
                identifier.compare(identifier.size() - ext.size(), ext.size(), ext.data(), ext.size()) == 0)
            {
                identifier.erase(identifier.size() - ext.size());
                break;
            }
        }
    }

    // Replace any character that is not valid in a JavaScript identifier with '_'.
    static const regex disallowedChars("[^a-zA-Z0-9_$]");
    return regex_replace(identifier, disallowedChars, "_");
}

namespace
{
    // Applies the import alias prefix to a scoped name when it originates from an external js:module.
    // For example, "Foo.SF" from js:module "foo" becomes "foo_Foo.SF".
    string applyImportAlias(const string& scopedName, const DefinitionContextPtr& dc, const string& currentJsModule)
    {
        string typeJsModule = Slice::JavaScript::getJavaScriptModule(dc);
        if (typeJsModule.empty() || typeJsModule == currentJsModule || typeJsModule == "@zeroc/ice")
        {
            // No alias needed: same module, no module, or the Ice runtime (always imported without alias).
            return scopedName;
        }

        // Type is from an external js:module - replace the top-level module name with the aliased import name.
        size_t dot = scopedName.find('.');
        string topLevel = (dot != string::npos) ? scopedName.substr(0, dot) : scopedName;
        string rest = (dot != string::npos) ? scopedName.substr(dot) : "";
        return Slice::JavaScript::importPathToIdentifier(typeJsModule) + "_" + topLevel + rest;
    }
}

string
Slice::JavaScript::resolveJsType(const TypePtr& type, const string& currentJsModule)
{
    string base = typeToJsString(type);

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (!contained)
    {
        return base;
    }

    return applyImportAlias(base, contained->definitionContext(), currentJsModule);
}

string
Slice::JavaScript::resolveJsScope(const ContainedPtr& contained, const string& currentJsModule)
{
    return applyImportAlias(contained->mappedScoped("."), contained->definitionContext(), currentJsModule);
}

string
Slice::JavaScript::typeToJsString(const TypePtr& type, bool definition)
{
    if (!type)
    {
        return "void";
    }

    static const char* javaScriptBuiltinTable[] = {
        "Number",  // byte
        "Boolean", // bool
        "Number",  // short
        "Number",  // int
        "BigInt",  // long
        "Number",  // float
        "Number",  // double
        "String",
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
        return cl->mappedScoped(".");
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return proxy->mappedScoped(".") + "Prx";
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
        return contained->mappedScoped(".");
    }

    return "???";
}

void
Slice::JavaScript::writeMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& param,
    bool marshal,
    const string& currentJsModule)
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
            out << nl << resolveJsType(type, currentJsModule) << "._write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << resolveJsType(type, currentJsModule) << "._read(" << stream << ");";
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
            out << nl << resolveJsType(type, currentJsModule) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << resolveJsType(type, currentJsModule) << ".read(" << stream << ", " << param
                << ");";
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
            // TypeRegistry key is a string literal - no alias resolution needed.
            out << nl << stream << ".readValue(obj => " << param << " = obj, Ice.TypeRegistry.getValueType(\""
                << typeToJsString(type) << "\"));";
        }
        return;
    }

    if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        if (marshal)
        {
            out << nl << getHelper(type, currentJsModule) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << getHelper(type, currentJsModule) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
Slice::JavaScript::writeOptionalMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& param,
    int32_t tag,
    bool marshal,
    const string& currentJsModule)
{
    assert(!type->isClassType()); // Optional classes are disallowed by the parser.

    string stream = marshal ? "ostr" : "istr";

    if (dynamic_pointer_cast<Enum>(type))
    {
        if (marshal)
        {
            out << nl << resolveJsType(type, currentJsModule) << "._writeOpt(" << stream << ", " << tag << ", "
                << param << ");";
        }
        else
        {
            out << nl << param << " = " << resolveJsType(type, currentJsModule) << "._readOpt(" << stream << ", "
                << tag << ");";
        }
        return;
    }

    if (marshal)
    {
        out << nl << getHelper(type, currentJsModule) << ".writeOptional(" << stream << ", " << tag << ", " << param
            << ");";
    }
    else
    {
        out << nl << param << " = " << getHelper(type, currentJsModule) << ".readOptional(" << stream << ", " << tag
            << ");";
    }
}

std::string
Slice::JavaScript::getHelper(const TypePtr& type, const string& currentJsModule)
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
        return resolveJsType(type, currentJsModule) + "._helper";
    }

    if (dynamic_pointer_cast<Struct>(type))
    {
        return resolveJsType(type, currentJsModule);
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        return resolveJsType(type, currentJsModule);
    }

    if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
        return resolveJsScope(contained, currentJsModule) + "Helper";
    }

    if (dynamic_pointer_cast<ClassDecl>(type))
    {
        return "Ice.ObjectHelper";
    }

    assert(false);
    return "???";
}

string
Slice::JavaScript::jsLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr& target)
{
    ostringstream result;
    if (target)
    {
        if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
        {
            result << typeToJsString(builtinTarget, true);
        }
        else
        {
            if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
            {
                string targetScoped = operationTarget->interface()->mappedScoped(".");

                // link to the method on the proxy interface
                result << targetScoped << "Prx." << operationTarget->mappedName();
            }
            else
            {
                string targetScoped = dynamic_pointer_cast<Contained>(target)->mappedScoped(".");
                if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
                {
                    // link to the proxy interface
                    result << targetScoped << "Prx";
                }
                else
                {
                    result << targetScoped;
                }
            }
        }
    }
    else
    {
        auto hashPos = rawLink.find('#');
        if (hashPos != string::npos)
        {
            // JavaScript TypeDoc doc processor doesn't accept # at the beginning of a link.
            if (hashPos != 0)
            {
                result << rawLink.substr(0, hashPos) << "#";
            }
            result << rawLink.substr(hashPos + 1);
        }
        else
        {
            result << rawLink;
        }
    }
    return result.str();
}

void
Slice::JavaScript::validateJsMetadata(const UnitPtr& u)
{
    map<string, MetadataInfo> knownMetadata;

    // "js:module"
    MetadataInfo moduleInfo = {
        .validOn = {typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:module", std::move(moduleInfo));

    // "js:defined-in"
    MetadataInfo definedInInfo = {
        .validOn = {typeid(InterfaceDecl), typeid(ClassDecl)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:defined-in", std::move(definedInInfo));

    // "js:identifier"
    MetadataInfo identifierInfo = {
        .validOn =
            {typeid(Module),
             typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(Parameter),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:identifier", std::move(identifierInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "js", std::move(knownMetadata));
}
