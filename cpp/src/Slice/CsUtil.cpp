// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/CsUtil.h>
#include <Slice/DotNetNames.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

static string
lookupKwd(const string& name, int baseTypes, bool mangleCasts = false)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked", "class", "const",
        "continue", "decimal", "default", "delegate", "do", "double", "else", "enum", "event", "explicit", "extern",
        "false", "finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in", "int", "interface",
        "internal", "is", "lock", "long", "namespace", "new", "null", "object", "operator", "out", "override",
        "params", "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short",
        "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true", "try", "typeof",
        "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual", "void", "volatile", "while"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "@" + name;
    }
    if(mangleCasts && (name == "checkedCast" || name == "uncheckedCast"))
    {
        return string(DotNet::manglePrefix) + name;
    }
    return Slice::DotNet::mangleName(name, baseTypes);
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C# keywords replaced by
// their "@"-prefixed version; otherwise, if the passed name is
// not scoped, but a C# keyword, return the "@"-prefixed name;
// otherwise, check if the name is one of the method names of baseTypes;
// if so, prefix it with _Ice_; otherwise, return the name unchanged.
//
string
Slice::CsGenerator::fixId(const string& name, int baseTypes, bool mangleCasts)
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name, baseTypes, mangleCasts);
    }
    StringList ids = splitScopedName(name);
    StringList newIds;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        newIds.push_back(lookupKwd(*i, baseTypes));
    }
    stringstream result;
    for(StringList::const_iterator j = newIds.begin(); j != newIds.end(); ++j)
    {
        if(j != newIds.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::CsGenerator::fixId(const ContainedPtr& cont, int baseTypes, bool mangleCasts)
{
    ContainerPtr container = cont->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    if(contained && contained->hasMetaData("clr:property") &&
       (contained->containedType() == Contained::ContainedTypeClass || contained->containedType() == Contained::ContainedTypeStruct))
    {
        return cont->name() + "__prop";
    }
    else
    {
        return fixId(cont->name(), baseTypes, mangleCasts);
    }
}

string
Slice::CsGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return "Ice.OptionalFormat.F1";
        }
        case Builtin::KindShort:
        {
            return "Ice.OptionalFormat.F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "Ice.OptionalFormat.F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "Ice.OptionalFormat.F8";
        }
        case Builtin::KindString:
        {
            return "Ice.OptionalFormat.VSize";
        }
        case Builtin::KindObject:
        {
            return "Ice.OptionalFormat.Class";
        }
        case Builtin::KindObjectProxy:
        {
            return "Ice.OptionalFormat.FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "Ice.OptionalFormat.Class";
}

string
Slice::CsGenerator::getStaticId(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    assert((b && b->kind() == Builtin::KindObject) || cl);

    if(b)
    {
        return "Ice.ObjectImpl.ice_staticId()";
    }
    else if(cl->isInterface())
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(cl->container());
        assert(cont);
        return fixId(cont->scoped(), DotNet::ICloneable) + "." + cl->name() + "Disp_.ice_staticId()";
    }
    else
    {
        return fixId(cl->scoped(), DotNet::ICloneable) + ".ice_staticId()";
    }
}

string
Slice::CsGenerator::typeToString(const TypePtr& type, bool optional)
{
    if(!type)
    {
        return "void";
    }

    if(optional)
    {
        return "Ice.Optional<" + typeToString(type, false) + ">";
    }

    static const char* builtinTable[] =
    {
        "byte",
        "bool",
        "short",
        "int",
        "long",
        "float",
        "double",
        "string",
        "Ice.Object",
        "Ice.ObjectPrx",
        "_System.Object"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixId(proxy->_class()->scoped() + "Prx");
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(seq->hasMetaData("clr:collection"))
        {
            return fixId(seq->scoped());
        }

        string prefix = "clr:generic:";
        string meta;
        if(seq->findMetaData(prefix, meta))
        {
            string type = meta.substr(prefix.size());
            if(type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
            {
                return "_System.Collections.Generic." + type + "<" + typeToString(seq->type()) + ">";
            }
            else
            {
                return "global::" + type + "<" + typeToString(seq->type()) + ">";
            }
        }

        prefix = "clr:serializable:";
        if(seq->findMetaData(prefix, meta))
        {
            string type = meta.substr(prefix.size());
            return "global::" + type;
        }

        return typeToString(seq->type()) + "[]";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        if(d->hasMetaData("clr:collection"))
        {
            return fixId(d->scoped());
        }

        string prefix = "clr:generic:";
        string meta;
        string typeName;
        if(d->findMetaData(prefix, meta))
        {
            typeName = meta.substr(prefix.size());
        }
        else
        {
            typeName = "Dictionary";
        }
        return "_System.Collections.Generic." + typeName
                + "<" + typeToString(d->keyType()) + ", " + typeToString(d->valueType()) + ">";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixId(contained->scoped());
    }

    return "???";
}

bool
Slice::CsGenerator::isValueType(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                return false;
                break;
            }
            default:
            {
                return true;
                break;
            }
        }
    }
    StructPtr s = StructPtr::dynamicCast(type);
    if(s)
    {
        if(s->hasMetaData("clr:class"))
        {
            return false;
        }
        DataMemberList dm = s->dataMembers();
        for(DataMemberList::const_iterator i = dm.begin(); i != dm.end(); ++i)
        {
            if(!isValueType((*i)->type()) || (*i)->defaultValueType())
            {
                return false;
            }
        }
        return true;
    }
    if(EnumPtr::dynamicCast(type))
    {
        return true;
    }
    return false;
}

bool
Slice::CsGenerator::isSerializable(const TypePtr& type)
{
    //
    // A proxy cannot be serialized because a communicator is required during deserialization.
    //
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if((builtin && builtin->kind() == Builtin::KindObjectProxy) || proxy)
    {
        return false;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return isSerializable(seq->type());
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return isSerializable(d->keyType()) && isSerializable(d->valueType());
    }

    return true;
}

void
Slice::CsGenerator::writeMarshalUnmarshalCode(Output &out,
                                              const TypePtr& type,
                                              const string& param,
                                              bool marshal,
                                              bool streamingAPI)
{
    string stream;

    if(marshal)
    {
        stream = streamingAPI ? "outS__" : "os__";
    }
    else
    {
        stream = streamingAPI ? "inS__" : "is__";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()" << ';';
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()" << ';';
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()" << ';';
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()" << ';';
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()" << ';';
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()" << ';';
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()" << ';';
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()" << ';';
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << param << ");";
                }
                else
                {
                    out << nl << stream << ".readObject(" << param << ");";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                string typeS = typeToString(type);
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy()" << ';';
                }
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        string typeS = typeToString(type);
        if(marshal)
        {
            out << nl << typeS << "Helper.write";
            if(!streamingAPI)
            {
                out << "__";
            }
            out << "(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeS << "Helper.read";
            if(!streamingAPI)
            {
                out << "__";
            }
            out << "(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readObject(" << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            const string write = streamingAPI ? "ice_write" : "write__";
            if(!isValueType(st))
            {
                out << nl << typeToString(st) << "." << write << "(" << stream << ", " << param << ");";
            }
            else
            {
                out << nl << param << "." << write << "(" << stream << ");";
            }
        }
        else
        {
            if(!isValueType(st))
            {
                const string r = streamingAPI ? "ice_read" : "read__";
                out << nl << param << " = " << typeToString(type) << "." << r << "(" << stream << ", " << param << ");";
            }
            else
            {
                const string read = streamingAPI ? "ice_read" : "read__";
                out << nl << param << "." << read << "(" << stream << ");";
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            if(streamingAPI)
            {
                out << nl << "if((int)" << param << " < " << en->minValue()
                    << " || (int)" << param << " > " << en->maxValue() << ")";
                out << sb;
                out << nl << "throw new Ice.MarshalException(\"enumerator out of range\");";
                out << eb;
            }
            out << nl << stream << ".writeEnum((int)" << param << ", " << en->maxValue() << ");";
        }
        else
        {
            out << nl << param << " = (" << fixId(en->scoped()) << ')' << stream << ".readEnum(" << en->maxValue()
                << ");";
            if(streamingAPI)
            {
                out << nl << "if((int)" << param << " < " << en->minValue() << " || (int)" << param << " > "
                    << en->maxValue() << ")";
                out << sb;
                out << nl << "throw new Ice.MarshalException(\"enumerator out of range\");";
                out << eb;
            }
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, param, marshal, streamingAPI, true);
        return;
    }

    assert(ConstructedPtr::dynamicCast(type));
    string typeS;
    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        typeS = fixId(d->scope()) + d->name();
    }
    else
    {
        typeS = typeToString(type);
    }
    if(marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << typeS << "Helper.read(" << stream << ')' << ';';
    }
}

void
Slice::CsGenerator::writeOptionalMarshalUnmarshalCode(Output &out,
                                                      const TypePtr& type,
                                                      const string& param,
                                                      int tag,
                                                      bool marshal,
                                                      bool streamingAPI)
{
    string stream;

    if(marshal)
    {
        stream = streamingAPI ? "outS__" : "os__";
    }
    else
    {
        stream = streamingAPI ? "inS__" : "is__";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte(" << tag << ");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool(" << tag << ");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort(" << tag << ");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt(" << tag << ");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong(" << tag << ");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat(" << tag << ");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble(" << tag << ");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString(" << tag << ");";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << stream << ".readObject(" << tag << ", " << param << ");";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                string typeS = typeToString(type);
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = new Ice.Optional<Ice.ObjectPrx>(" << stream << ".readProxy(" << tag
                        << "));";
                }
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        if(marshal)
        {
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag
                << ", Ice.OptionalFormat.FSize))";
            out << sb;
            out << nl << "int pos__ = " << stream << ".startSize();";
            writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, streamingAPI);
            out << nl << stream << ".endSize(pos__);";
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOpt(" << tag << ", Ice.OptionalFormat.FSize))";
            out << sb;
            out << nl << stream << ".skip(4);";
            string tmp = "tmpVal__";
            string typeS = typeToString(type);
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, tmp, marshal, streamingAPI);
            out << nl << param << " = new Ice.Optional<" << typeS << ">(" << tmp << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = new Ice.Optional<" << typeS << ">();";
            out << eb;
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".readObject(" << tag << ", " << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag << ", "
                << getOptionalFormat(st) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos__ = " << stream << ".startSize();";
            }
            else
            {
                out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
            }
            writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, streamingAPI);
            if(st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos__);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(st) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else
            {
                out << nl << stream << ".skipSize();";
            }
            string typeS = typeToString(type);
            string tmp = "tmpVal__";
            if(isValueType(st))
            {
                out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
            }
            else
            {
                out << nl << typeS << ' ' << tmp << " = null;";
            }
            writeMarshalUnmarshalCode(out, type, tmp, marshal, streamingAPI);
            out << nl << param << " = new Ice.Optional<" << typeS << ">(" << tmp << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = new Ice.Optional<" << typeS << ">();";
            out << eb;
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        size_t sz = en->getEnumerators().size();
        if(marshal)
        {
            out << nl << "if(" << param << ".HasValue)";
            out << sb;
            out << nl << stream << ".writeEnum(" << tag << ", (int)" << param << ".Value, " << sz << ");";
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOpt(" << tag << ", Ice.OptionalFormat.Size))";
            out << sb;
            string typeS = typeToString(type);
            string tmp = "tmpVal__";
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, tmp, marshal, streamingAPI);
            out << nl << param << " = new Ice.Optional<" << typeS << ">(" << tmp << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = new Ice.Optional<" << typeS << ">();";
            out << eb;
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeOptionalSequenceMarshalUnmarshalCode(out, seq, param, tag, marshal, streamingAPI);
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    assert(d);
    TypePtr keyType = d->keyType();
    TypePtr valueType = d->valueType();
    if(marshal)
    {
        out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag << ", "
            << getOptionalFormat(d) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << "int pos__ = " << stream << ".startSize();";
        }
        else
        {
            out << nl << stream << ".writeSize(" << param << ".Value == null ? 1 : " << param << ".Value.Count * "
                << (keyType->minWireSize() + valueType->minWireSize()) << " + (" << param
                << ".Value.Count > 254 ? 5 : 1));";
        }
        writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, streamingAPI);
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".endSize(pos__);";
        }
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(d) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".skip(4);";
        }
        else
        {
            out << nl << stream << ".skipSize();";
        }
        string typeS = typeToString(type);
        string tmp = "tmpVal__";
        out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
        writeMarshalUnmarshalCode(out, type, tmp, marshal, streamingAPI);
        out << nl << param << " = new Ice.Optional<" << typeS << ">(" << tmp << ");";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << param << " = new Ice.Optional<" << typeS << ">();";
        out << eb;
    }
}

void
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                      const SequencePtr& seq,
                                                      const string& param,
                                                      bool marshal,
                                                      bool streamingAPI,
                                                      bool useHelper)
{
    string stream;
    if(marshal)
    {
        stream = streamingAPI ? "outS__" : "os__";
    }
    else
    {
        stream = streamingAPI ? "inS__" : "is__";
    }

    if(useHelper)
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(seq->container());
        assert(cont);
        string helperName = fixId(cont->scoped(), DotNet::ICloneable) + "." + seq->name() + "Helper";
        if(marshal)
        {
            out << nl << helperName << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << helperName << ".read(" << stream << ");";
        }
        return;
    }

    TypePtr type = seq->type();
    string typeS = typeToString(type);

    const string genericPrefix = "clr:generic:";
    string genericType;
    string addMethod = "Add";
    const bool isGeneric = seq->findMetaData(genericPrefix, genericType);
    bool isStack = false;
    bool isList = false;
    bool isLinkedList = false;
    bool isCustom = false;
    if(isGeneric)
    {
        genericType = genericType.substr(genericPrefix.size());
        if(genericType == "LinkedList")
        {
            addMethod = "AddLast";
            isLinkedList = true;
        }
        else if(genericType == "Queue")
        {
            addMethod = "Enqueue";
        }
        else if(genericType == "Stack")
        {
            addMethod = "Push";
            isStack = true;
        }
        else if(genericType == "List")
        {
            isList = true;
        }
        else
        {
            isCustom = true;
        }
    }
    const bool isCollection = seq->hasMetaData("clr:collection");
    const bool isArray = !isGeneric && !isCollection;
    const string limitID = isArray ? "Length" : "Count";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << "if(" << param << " == null)";
                    out << sb;
                    out << nl << stream << ".writeSize(0);";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
                    if(isGeneric && !isList)
                    {
                        if(isStack)
                        {
                            //
                            // If the collection is a stack, write in top-to-bottom order. Stacks
                            // cannot contain Ice.Object.
                            //
                            out << nl << "Ice.ObjectPrx[] " << param << "_tmp = " << param << ".ToArray();";
                            out << nl << "for(int ix__ = 0; ix__ < " << param << "_tmp.Length; ++ix__)";
                            out << sb;
                            out << nl << stream << ".writeProxy(" << param << "_tmp[ix__]);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                                << "> e__ = " << param << ".GetEnumerator();";
                            out << nl << "while(e__.MoveNext())";
                            out << sb;
                            string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
                            out << nl << stream << '.' << func << "(e__.Current);";
                            out << eb;
                        }
                    }
                    else
                    {
                        out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
                        out << sb;
                        string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
                        out << nl << stream << '.' << func << '(' << param << "[ix__]);";
                        out << eb;
                    }
                    out << eb;
                }
                else
                {
                    out << nl << "int " << param << "_lenx = " << stream << ".readAndCheckSeqSize("
                        << static_cast<unsigned>(builtin->minWireSize()) << ");";
                    out << nl << param << " = new ";
                    if(builtin->kind() == Builtin::KindObject)
                    {
                        if(isArray)
                        {
                            out << "Ice.Object[" << param << "_lenx];";
                        }
                        else if(isCustom)
                        {
                            out << "global::" << genericType << "<Ice.Object>();";
                        }
                        else if(isGeneric)
                        {
                            out << "_System.Collections.Generic." << genericType << "<Ice.Object>(";
                            if(!isLinkedList)
                            {
                                out << param << "_lenx";
                            }
                            out << ");";
                        }
                        else
                        {
                            out << typeToString(seq) << "(" << param << "_lenx);";
                        }
                        out << nl << "for(int ix__ = 0; ix__ < " << param << "_lenx; ++ix__)";
                        out << sb;
                        out << nl << stream << ".readObject(";
                        string patcherName;
                        if(isCustom)
                        {
                            patcherName = "CustomSeq";
                        }
                        else if(isList)
                        {
                            patcherName = "List";
                        }
                        else if(isArray)
                        {
                            patcherName = "Array";
                        }
                        else
                        {
                            patcherName = "Sequence";
                        }
                        out << "new IceInternal." << patcherName << "Patcher<Ice.Object>(\"::Ice::Object\", "
                            << param << ", ix__));";
                    }
                    else
                    {
                        if(isArray)
                        {
                            out << "Ice.ObjectPrx[" << param << "_lenx];";
                        }
                        else if(isGeneric)
                        {
                            out << "_System.Collections.Generic." << genericType << "<Ice.ObjectPrx>(";
                            if(!isLinkedList)
                            {
                                out << param << "_lenx";
                            }
                            out << ");";
                        }
                        else
                        {
                            out << typeToString(seq) << "(" << param << "_lenx);";
                        }
                        out << nl << "for(int ix__ = 0; ix__ < " << param << "_lenx; ++ix__)";
                        out << sb;
                        if(isArray)
                        {
                            out << nl << param << "[ix__] = " << stream << ".readProxy();";
                        }
                        else
                        {
                            out << nl << "Ice.ObjectPrx val__ = new Ice.ObjectPrxHelperBase();";
                            out << nl << "val__ = " << stream << ".readProxy();";
                            out << nl << param << "." << addMethod << "(val__);";
                        }
                    }
                    out << eb;
                }
                break;
            }
            default:
            {
                string prefix = "clr:serializable:";
                string meta;
                if(seq->findMetaData(prefix, meta))
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeSerializable(" << param << ");";
                    }
                    else
                    {
                        out << nl << param << " = (" << typeToString(seq) << ")" << stream << ".readSerializable();";
                    }
                    break;
                }

                string func = typeS;
                func[0] = toupper(static_cast<unsigned char>(typeS[0]));
                if(marshal)
                {
                    if(isArray)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << param << ");";
                    }
                    else if(isCollection)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << param << " == null ? null : "
                            << param << ".ToArray());";
                    }
                    else if(isCustom)
                    {
                        if(streamingAPI)
                        {
                            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
                            out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                                << "> e__ = " << param << ".GetEnumerator();";
                            out << nl << "while(e__.MoveNext())";
                            out << sb;
                            out << nl << stream << ".write" << func << "(e__.Current);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : "
                                << param << ".Count, " << param << ");";
                        }
                    }
                    else
                    {
                        assert(isGeneric);
                        if(!streamingAPI)
                        {
                            out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : "
                                << param << ".Count, " << param << ");";
                        }
                        else if(isLinkedList)
                        {
                            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
                            out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                                << "> e__ = " << param << ".GetEnumerator();";
                            out << nl << "while(e__.MoveNext())";
                            out << sb;
                            out << nl << stream << ".write" << func << "(e__.Current);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << stream << ".write" << func << "Seq(" << param << " == null ? null : "
                                << param << ".ToArray());";
                        }
                    }
                }
                else
                {
                    if(isArray)
                    {
                        out << nl << param << " = " << stream << ".read" << func << "Seq();";
                    }
                    else if(isCustom)
                    {
                        out << sb;
                        out << nl << param << " = new " << "global::" << genericType << "<"
                            << typeToString(type) << ">();";
                        out << nl << "int szx__ = " << stream << ".readSize();";
                        out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
                        out << sb;
                        out << nl << param << ".Add(" << stream << ".read" << func << "());";
                        out << eb;
                        out << eb;
                    }
                    else if(isCollection)
                    {
                        out << nl << param << " = new " << fixId(seq->scoped())
                            << '(' << stream << ".read" << func << "Seq());";
                    }
                    else
                    {
                        assert(isGeneric);
                        if(streamingAPI)
                        {
                            if(isStack)
                            {
                                //
                                // Stacks are marshaled in top-to-bottom order. The "Stack(type[])"
                                // constructor assumes the array is in bottom-to-top order, so we
                                // read the array first, then reverse it.
                                //
                                out << nl << typeS << "[] arr__ = " << stream << ".read" << func << "Seq();";
                                out << nl << "_System.Array.Reverse(arr__);";
                                out << nl << param << " = new " << typeToString(seq) << "(arr__);";
                            }
                            else
                            {
                                out << nl << param << " = new " << typeToString(seq) << '(' << stream
                                    << ".read" << func << "Seq());";
                            }
                        }
                        else
                        {
                            out << nl << stream << ".read" << func << "Seq(out " << param << ");";
                        }
                    }
                }
                break;
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks cannot contain class instances, so there is no need to marshal a
                // stack bottom-up here.
                //
                out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                    << "> e__ = " << param << ".GetEnumerator();";
                out << nl << "while(e__.MoveNext())";
                out << sb;
                out << nl << stream << ".writeObject(e__.Current);";
                out << eb;
            }
            else
            {
                out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
                out << sb;
                out << nl << stream << ".writeObject(" << param << "[ix__]);";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            out << nl << param << " = new ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "[]", "szx__");
            }
            else if(isCustom)
            {
                out << "global::" << genericType << "<" << typeS << ">()";
            }
            else if(isGeneric)
            {
                out << "_System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx__";
                }
                out << ")";
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << ';';
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;

            string patcherName;
            if(isCustom)
            {
                patcherName = "CustomSeq";
            }
            else if(isList)
            {
                patcherName = "List";
            }
            else if(isArray)
            {
                patcherName = "Array";
            }
            else
            {
                patcherName = "Sequence";
            }
            string scoped = ContainedPtr::dynamicCast(type)->scoped();
            out << nl << "IceInternal." << patcherName << "Patcher<" << typeS << "> spx = new IceInternal."
                << patcherName << "Patcher<" << typeS << ">(\"" << scoped << "\", " << param << ", ix__);";
            out << nl << stream << ".readObject(";
            out << "spx);";
            out << eb;
            out << eb;
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if(isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for(int ix__ = 0; ix__ < " << param << "_tmp.Length; ++ix__)";
                }
                else
                {
                    out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                        << "> e__ = " << param << ".GetEnumerator();";
                    out << nl << "while(e__.MoveNext())";
                }
            }
            else
            {
                out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
            }
            out << sb;
            string call;
            if(isGeneric && !isList && !isStack)
            {
                if(isValueType(type))
                {
                    call = "e__.Current";
                }
                else
                {
                    call = "(e__.Current == null ? new ";
                    call += typeS + "() : e__.Current)";
                }
            }
            else
            {
                if(isValueType(type))
                {
                    call = param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                }
                else
                {
                    call = "(";
                    call += param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                    call += "[ix__] == null ? new " + typeS + "() : " + param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                }
                call += "[ix__]";
                if(!isValueType(type))
                {
                    call += ")";
                }
            }
            call += ".";
            call += streamingAPI ? "ice_write" : "write__";
            call += "(" + stream + ");";
            out << nl << call;
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "__tmp = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
            }
            else if(isGeneric)
            {
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx__";
                }
                out << ");";
            }
            else
            {
                out << nl << param << " = new " << fixId(seq->scoped()) << "(szx__);";
            }
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "__tmp";
                if(!isValueType(st))
                {
                    out << nl << v << "[ix__] = new " << typeS << "();";
                }
                if(streamingAPI)
                {
                    out << nl << v << "[ix__].ice_read(" << stream << ");";
                }
                else
                {
                    out << nl << v << "[ix__].read__(" << stream << ");";
                }
            }
            else
            {
                out << nl << typeS << " val__ = new " << typeS << "();";
                if(streamingAPI)
                {
                    out << nl << "val__.ice_read(" << stream << ");";
                }
                else
                {
                    out << nl << "val__.read__(" << stream << ");";
                }
                out << nl << param << "." << addMethod << "(val__);";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "_System.Array.Reverse(" << param << "__tmp);";
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "__tmp);";
            }
            out << eb;
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.'<< limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if(isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for(int ix__ = 0; ix__ < " << param << "_tmp.Length; ++ix__)";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)" << param << "_tmp[ix__], " << en->maxValue() << ");";
                    out << eb;
                }
                else
                {
                    out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                        << "> e__ = " << param << ".GetEnumerator();";
                    out << nl << "while(e__.MoveNext())";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)e__.Current, " << en->maxValue() << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
                out << sb;
                out << nl << stream << ".writeEnum((int)" << param << "[ix__], " << en->maxValue() << ");";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readAndCheckSeqSize(" <<
                static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "__tmp = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
            }
            else if(isGeneric)
            {
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx__";
                }
                out << ");";
            }
            else
            {
                out << nl << param << " = new " << fixId(seq->scoped()) << "(szx__);";
            }
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "__tmp";
                out << nl << v << "[ix__] = (" << typeS << ')' << stream << ".readEnum(" << en->maxValue() << ");";
            }
            else
            {
                out << nl << param << "." << addMethod << "((" << typeS << ')' << stream << ".readEnum("
                    << en->maxValue() << "));";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "_System.Array.Reverse(" << param << "__tmp);";
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "__tmp);";
            }
            out << eb;
        }
        return;
    }

    string helperName;
    if(ProxyPtr::dynamicCast(type))
    {
        helperName = fixId(ProxyPtr::dynamicCast(type)->_class()->scoped() + "PrxHelper");
    }
    else
    {
        helperName = fixId(ContainedPtr::dynamicCast(type)->scoped() + "Helper");
    }

    string func;
    if(marshal)
    {
        func = "write";
        if(!streamingAPI && ProxyPtr::dynamicCast(type))
        {
           func += "__";
        }
        out << nl << "if(" << param << " == null)";
        out << sb;
        out << nl << stream << ".writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
        if(isGeneric && !isList)
        {
            //
            // Stacks are marshaled top-down.
            //
            if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                out << nl << "for(int ix__ = 0; ix__ < " << param << "_tmp.Length; ++ix__)";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", " << param << "_tmp[ix__]);";
                out << eb;
            }
            else
            {
                out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                    << "> e__ = " << param << ".GetEnumerator();";
                out << nl << "while(e__.MoveNext())";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", e__.Current);";
                out << eb;
            }
        }
        else
        {
            out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
            out << sb;
            out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[ix__]);";
            out << eb;
        }
        out << eb;
    }
    else
    {
        func = "read";
        if(!streamingAPI && ProxyPtr::dynamicCast(type))
        {
           func += "__";
        }
        out << sb;
        out << nl << "int szx__ = " << stream << ".readAndCheckSeqSize("
            << static_cast<unsigned>(type->minWireSize()) << ");";
        if(isArray)
        {
            out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
        }
        else if(isCustom)
        {
            out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
        }
        else if(isStack)
        {
            out << nl << typeS << "[] " << param << "__tmp = new " << toArrayAlloc(typeS + "[]", "szx__") << ";";
        }
        else if(isGeneric)
        {
            out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">();";
        }
        else
        {
            out << nl << param << " = new " << fixId(seq->scoped()) << "(szx__);";
        }
        out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
        out << sb;
        if(isArray || isStack)
        {
            string v = isArray ? param : param + "__tmp";
            out << nl << v << "[ix__] = " << helperName << '.' << func << '(' << stream << ");";
        }
        else
        {
            out << nl << param << "." << addMethod << "(" << helperName << '.' << func << '(' << stream << "));";
        }
        out << eb;
        if(isStack)
        {
            out << nl << "_System.Array.Reverse(" << param << "__tmp);";
            out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                << param << "__tmp);";
        }
        out << eb;
    }

    return;
}

void
Slice::CsGenerator::writeOptionalSequenceMarshalUnmarshalCode(Output& out,
                                                              const SequencePtr& seq,
                                                              const string& param,
                                                              int tag,
                                                              bool marshal,
                                                              bool streamingAPI)
{
    string stream;
    if(marshal)
    {
        stream = streamingAPI ? "outS__" : "os__";
    }
    else
    {
        stream = streamingAPI ? "inS__" : "is__";
    }

    const TypePtr type = seq->type();
    const string typeS = typeToString(type);
    const string seqS = typeToString(seq);

    string meta;
    const bool isArray = !seq->findMetaData("clr:generic:", meta) && !seq->hasMetaData("clr:collection");
    const string length = isArray ? param + ".Value.Length" : param + ".Value.Count";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        case Builtin::KindShort:
        case Builtin::KindInt:
        case Builtin::KindFloat:
        case Builtin::KindLong:
        case Builtin::KindDouble:
        case Builtin::KindString:
        {
            string func = typeS;
            func[0] = toupper(static_cast<unsigned char>(typeS[0]));
            const bool isSerializable = seq->findMetaData("clr:serializable:", meta);

            if(marshal)
            {
                if(isSerializable)
                {
                    out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag
                        << ", Ice.OptionalFormat.VSize))";
                    out << sb;
                    out << nl << stream << ".writeSerializable(" << param << ".Value);";
                    out << eb;
                }
                else if(isArray)
                {
                    out << nl << stream << ".write" << func << "Seq(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << "if(" << param << ".HasValue)";
                    out << sb;
                    out << nl << stream << ".write" << func << "Seq(" << tag << ", " << param
                        << ".Value == null ? 0 : " << param << ".Value.Count, " << param << ".Value);";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(seq) << "))";
                out << sb;
                if(builtin->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindBool)
                {
                    out << nl << stream << ".skipSize();";
                }
                string tmp = "tmpVal__";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, streamingAPI, true);
                out << nl << param << " = new Ice.Optional<" << seqS << ">(" << tmp << ");";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << param << " = new Ice.Optional<" << seqS << ">();";
                out << eb;
            }
            break;
        }

        case Builtin::KindObject:
        case Builtin::KindObjectProxy:
        {
            if(marshal)
            {
                out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag << ", "
                    << getOptionalFormat(seq) << "))";
                out << sb;
                out << nl << "int pos__ = " << stream << ".startSize();";
                writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, streamingAPI, true);
                out << nl << stream << ".endSize(pos__);";
                out << eb;
            }
            else
            {
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(seq) << "))";
                out << sb;
                out << nl << stream << ".skip(4);";
                string tmp = "tmpVal__";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, streamingAPI, true);
                out << nl << param << " = new Ice.Optional<" << seqS << ">(" << tmp << ");";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << param << " = new Ice.Optional<" << seqS << ">();";
                out << eb;
            }
            break;
        }

        case Builtin::KindLocalObject:
            assert(false);
        }

        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag << ", "
                << getOptionalFormat(seq) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos__ = " << stream << ".startSize();";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".writeSize(" << param << ".Value == null ? 1 : " << length << " * "
                    << st->minWireSize() << " + (" << length << " > 254 ? 5 : 1));";
            }
            writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, streamingAPI, true);
            if(st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos__);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(seq) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".skipSize();";
            }
            string tmp = "tmpVal__";
            out << nl << seqS << ' ' << tmp << ';';
            writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, streamingAPI, true);
            out << nl << param << " = new Ice.Optional<" << seqS << ">(" << tmp << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = new Ice.Optional<" << seqS << ">();";
            out << eb;
        }
        return;
    }

    //
    // At this point, all remaining element types have variable size.
    //
    if(marshal)
    {
        out << nl << "if(" << param << ".HasValue && " << stream << ".writeOpt(" << tag << ", "
            << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << "int pos__ = " << stream << ".startSize();";
        writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, streamingAPI, true);
        out << nl << stream << ".endSize(pos__);";
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << stream << ".skip(4);";
        string tmp = "tmpVal__";
        out << nl << seqS << ' ' << tmp << ';';
        writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, streamingAPI, true);
        out << nl << param << " = new Ice.Optional<" << seqS << ">(" << tmp << ");";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << param << " = new Ice.Optional<" << seqS << ">();";
        out << eb;
    }
}

void
Slice::CsGenerator::writeSerializeDeserializeCode(Output &out,
                                                  const TypePtr& type,
                                                  const string& param,
                                                  bool optional,
                                                  int tag,
                                                  bool serialize)
{
    if(!isSerializable(type))
    {
        return;
    }

    if(optional)
    {
        const string typeName = typeToString(type, true);
        if(serialize)
        {
            out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetByte(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetBoolean(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetInt16(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetInt32(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetInt64(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetSingle(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetDouble(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << " == null ? \"\" : " << param
                        << ");";
                }
                else
                {
                    out << nl << param << " = " << "info__.GetString(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindLocalObject:
            {
                const string typeName = typeToString(type, false);
                if(serialize)
                {
                    out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
                }
                else
                {
                    out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof("
                        << typeName << "));";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                //
                // Proxies cannot be serialized.
                //
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        //
        // Proxies cannot be serialized.
        //
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    assert(d);
    const string typeName = typeToString(type, false);
    if(serialize)
    {
        out << nl << "info__.AddValue(\"" << param << "\", " << param << ", typeof(" << typeName << "));";
    }
    else
    {
        out << nl << param << " = (" << typeName << ")info__.GetValue(\"" << param << "\", typeof(" << typeName
            << "));";
    }
}

string
Slice::CsGenerator::toArrayAlloc(const string& decl, const string& sz)
{
    int count = 0;
    string::size_type pos = decl.size();
    while(pos > 1 && decl.substr(pos - 2, 2) == "[]")
    {
        ++count;
        pos -= 2;
    }
    assert(count > 0);

    ostringstream o;
    o << decl.substr(0, pos) << '[' << sz << ']' << decl.substr(pos + 2);
    return o.str();
}

void
Slice::CsGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, true);
}

bool
Slice::CsGenerator::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();

        static const string csPrefix = "cs:";
        static const string clrPrefix = "clr:";
        for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end(); ++r)
        {
            string s = *r;
            if(_history.count(s) == 0)
            {
                if(s.find(csPrefix) == 0)
                {
                    static const string csAttributePrefix = csPrefix + "attribute:";
                    if(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size())
                    {
                        continue;
                    }
                    emitWarning(file, -1, "ignoring invalid global metadata `" + s + "'");
                    _history.insert(s);
                }
                else if(s.find(clrPrefix) == 0)
                {
                    emitWarning(file, -1, "ignoring invalid global metadata `" + s + "'");
                    _history.insert(s);
                }
            }
        }
    }
    return true;
}

bool
Slice::CsGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::CsGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    validate(p);
}

bool
Slice::CsGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::CsGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::CsGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::CsGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    if(p->hasMetaData("UserException"))
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
        if(!cl->isLocal())
        {
            ostringstream os;
            os << "ignoring invalid metadata `UserException': directive applies only to local operations "
               << "but enclosing " << (cl->isInterface() ? "interface" : "class") << "`" << cl->name()
               << "' is not local";
            emitWarning(p->file(), p->line(), os.str());
        }
    }
    validate(p);

    ParamDeclList params = p->parameters();
    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        visitParamDecl(*i);
    }
}

void
Slice::CsGenerator::MetaDataVisitor::visitParamDecl(const ParamDeclPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::validate(const ContainedPtr& cont)
{
    static set<string> collectionWarnings;
    const string msg = "ignoring invalid metadata";

    StringList localMetaData = cont->getMetaData();

    for(StringList::const_iterator p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        string s = *p;

        string prefix = "clr:";
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                SequencePtr seq = SequencePtr::dynamicCast(cont);
                if(seq)
                {
                    if(s.substr(prefix.size()) == "collection")
                    {
                        if(collectionWarnings.find(cont->file()) == collectionWarnings.end()) {
                            emitWarning(cont->file(), cont->line(), "the \"" + s + "\" metadata has been deprecated");
                            collectionWarnings.insert(cont->file());
                        }
                        continue;
                    }
                    static const string clrGenericPrefix = prefix + "generic:";
                    if(s.find(clrGenericPrefix) == 0)
                    {
                        string type = s.substr(clrGenericPrefix.size());
                        if(type == "LinkedList" || type == "Queue" || type == "Stack")
                        {
                            ClassDeclPtr cd = ClassDeclPtr::dynamicCast(seq->type());
                            BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                            if(!cd && !(builtin && builtin->kind() == Builtin::KindObject))
                            {
                                continue;
                            }
                        }
                        else if(!type.empty())
                        {
                            continue; // Custom type or List<T>
                        }
                    }
                    static const string clrSerializablePrefix = prefix + "serializable:";
                    if(s.find(clrSerializablePrefix) == 0)
                    {
                        string meta;
                        if(cont->findMetaData(prefix + "collection", meta)
                           || cont->findMetaData(prefix + "generic:", meta))
                        {
                            emitWarning(cont->file(), cont->line(), msg + " `" + meta + "':\n" +
                                        "serialization can only be used with the array mapping for byte sequences");
                        }
                        string type = s.substr(clrSerializablePrefix.size());
                        BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                        if(!type.empty() && builtin && builtin->kind() == Builtin::KindByte)
                        {
                            continue;
                        }
                    }
                }
                else if(StructPtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "class")
                    {
                        continue;
                    }
                    if(s.substr(prefix.size()) == "property")
                    {
                        continue;
                    }
                    static const string clrImplementsPrefix = prefix + "implements:";
                    if(s.find(clrImplementsPrefix) == 0)
                    {
                        continue;
                    }
                }
                else if(ClassDefPtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "property")
                    {
                        continue;
                    }
                    static const string clrImplementsPrefix = prefix + "implements:";
                    if(s.find(clrImplementsPrefix) == 0)
                    {
                        continue;
                    }
                }
                else if(DictionaryPtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "collection")
                    {
                        if(collectionWarnings.find(cont->file()) == collectionWarnings.end()) {
                            emitWarning(cont->file(), cont->line(), "the \"" + s + "\" metadata has been deprecated");
                            collectionWarnings.insert(cont->file());
                        }
                        continue;
                    }
                    static const string clrGenericPrefix = prefix + "generic:";
                    if(s.find(clrGenericPrefix) == 0)
                    {
                        string type = s.substr(clrGenericPrefix.size());
                        if(type == "SortedDictionary" ||  type == "SortedList")
                        {
                            continue;
                        }
                    }
                }
                emitWarning(cont->file(), cont->line(), msg + " `" + s + "'");
                _history.insert(s);
            }
        }

        prefix = "cs:";
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                static const string csAttributePrefix = prefix + "attribute:";
                if(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size())
                {
                    continue;
                }
                emitWarning(cont->file(), cont->line(), msg + " `" + s + "'");
                _history.insert(s);
            }
        }
    }
}
