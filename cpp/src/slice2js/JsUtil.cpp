// Copyright (c) ZeroC, Inc.

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

string
Slice::getJavaScriptModule(const DefinitionContextPtr& dc)
{
    // Check if the file contains the 'js:module' file metadata.
    assert(dc);
    return dc->getMetadataArgs("js:module").value_or("");
}

string
Slice::JsGenerator::typeToJsString(const TypePtr& type, bool definition)
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
        return dynamic_pointer_cast<Contained>(type)->mappedScoped(".") + "Helper";
    }

    if (dynamic_pointer_cast<ClassDecl>(type))
    {
        return "Ice.ObjectHelper";
    }

    assert(false);
    return "???";
}
