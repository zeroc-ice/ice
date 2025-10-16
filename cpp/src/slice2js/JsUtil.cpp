// Copyright (c) ZeroC, Inc.

#include "JsUtil.h"
#include "../Slice/MetadataValidation.h"
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
Slice::JavaScript::getJavaScriptModule(const DefinitionContextPtr& dc)
{
    // Check if the file contains the 'js:module' file metadata.
    assert(dc);
    return dc->getMetadataArgs("js:module").value_or("");
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
Slice::JavaScript::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal)
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
Slice::JavaScript::writeOptionalMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& param,
    int32_t tag,
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
Slice::JavaScript::getHelper(const TypePtr& type)
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
        return dynamic_pointer_cast<Contained>(type)->mappedScoped(".") + "Helper";
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
