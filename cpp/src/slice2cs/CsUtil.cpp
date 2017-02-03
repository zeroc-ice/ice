// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <CsUtil.h>
#include <DotNetNames.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

string
lookupKwd(const string& name, int baseTypes, bool mangleCasts = false)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "abstract", "as", "async", "await", "base", "bool", "break", "byte", "case", "catch", "char", "checked", "class", "const",
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
StringList
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

}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C# keywords replaced by
// their "@"-prefixed version; otherwise, if the passed name is
// not scoped, but a C# keyword, return the "@"-prefixed name;
// otherwise, check if the name is one of the method names of baseTypes;
// if so, prefix it with ice_; otherwise, return the name unchanged.
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
    if(contained && contained->hasMetaData("cs:property") &&
       (contained->containedType() == Contained::ContainedTypeClass || contained->containedType() == Contained::ContainedTypeStruct))
    {
        return "_" + cont->name();
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
        case Builtin::KindValue:
        {
            return "Ice.OptionalFormat.Class";
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

    assert(isClassType(type));

    if(b)
    {
        return "Ice.Value.ice_staticId()";
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
Slice::CsGenerator::typeToString(const TypePtr& type, bool optional, bool local)
{
    if(!type)
    {
        return "void";
    }

    if(optional)
    {
        return "Ice.Optional<" + typeToString(type, false, local) + ">";
    }

    static const char* localBuiltinTable[] =
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
        "_System.Object",
        "Ice.Value"
    };

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
        "Ice.Value",
        "Ice.ObjectPrx",
        "_System.Object",
        "Ice.Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return local ? localBuiltinTable[builtin->kind()] : builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cl->isInterface() && !local)
        {
            return "Ice.Value";
        }
        else
        {
            return fixId(cl->scoped());
        }
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        ClassDefPtr def = proxy->_class()->definition();
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            return fixId(proxy->_class()->scoped() + "Prx");
        }
        else
        {
            return "Ice.ObjectPrx";
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string prefix = "cs:generic:";
        string meta;
        if(seq->findMetaData(prefix, meta))
        {
            string type = meta.substr(prefix.size());
            if(type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
            {
                return "_System.Collections.Generic." + type + "<" + typeToString(seq->type(), optional, local) + ">";
            }
            else
            {
                return "global::" + type + "<" + typeToString(seq->type(), optional, local) + ">";
            }
        }

        prefix = "cs:serializable:";
        if(seq->findMetaData(prefix, meta))
        {
            string type = meta.substr(prefix.size());
            return "global::" + type;
        }

        return typeToString(seq->type(), optional, local) + "[]";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        string prefix = "cs:generic:";
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
                + "<" + typeToString(d->keyType(), optional, local) + ", " + typeToString(d->valueType(), optional, local) + ">";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixId(contained->scoped());
    }

    return "???";
}

string
Slice::CsGenerator::resultStructName(const string& className, const string& opName, bool marshaledResult)
{
    ostringstream s;
    s << className
      << "_"
      << IceUtilInternal::toUpper(opName.substr(0, 1))
      << opName.substr(1)
      << (marshaledResult ? "MarshaledResult" : "Result");
    return s.str();
}

string
Slice::CsGenerator::resultType(const OperationPtr& op, bool dispatch)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.
    if(dispatch && op->hasMarshaledResult())
    {
        return resultStructName(cl->name(), op->name(), true);
    }

    string t;
    ParamDeclList outParams = op->outParameters();
    if(op->returnType() || !outParams.empty())
    {
        if(outParams.empty())
        {
            t = typeToString(op->returnType(), op->returnIsOptional(), cl->isLocal());
        }
        else if(op->returnType() || outParams.size() > 1)
        {
            ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
            t = fixId(cl->scope()) + resultStructName(cl->name(), op->name());
        }
        else
        {
            t = typeToString(outParams.front()->type(), outParams.front()->optional(), cl->isLocal());
        }
    }

    return t;
}

string
Slice::CsGenerator::taskResultType(const OperationPtr& op, bool dispatch)
{
    string t = resultType(op, dispatch);
    if(t.empty())
    {
        return "_System.Threading.Tasks.Task";
    }
    else
    {
        return "_System.Threading.Tasks.Task<" + resultType(op, dispatch) + '>';
    }
}

bool
Slice::CsGenerator::isClassType(const TypePtr& type)
{
    if(ClassDeclPtr::dynamicCast(type))
    {
        return true;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue);
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
            case Builtin::KindValue:
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
        if(s->hasMetaData("cs:class"))
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

void
Slice::CsGenerator::writeMarshalUnmarshalCode(Output &out,
                                              const TypePtr& type,
                                              const string& param,
                                              bool marshal,
                                              const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
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
            case Builtin::KindValue:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeValue(" << param << ");";
                }
                else
                {
                    out << nl << stream << ".readValue(" << param << ");";
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
        ClassDefPtr def = prx->_class()->definition();
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            string typeS = typeToString(type);
            if (marshal)
            {
                out << nl << typeS << "Helper.write(" << stream << ", " << param << ");";
            }
            else
            {
                out << nl << param << " = " << typeS << "Helper.read(" << stream << ");";
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << stream << ".writeProxy(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".readProxy()" << ';';
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeValue(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readValue(" << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            if(!isValueType(st))
            {
                out << nl << typeToString(st) << ".ice_write(" << stream << ", " << param << ");";
            }
            else
            {
                out << nl << param << ".ice_writeMembers(" << stream << ");";
            }
        }
        else
        {
            if(!isValueType(st))
            {
                out << nl << param << " = " << typeToString(type) << ".ice_read(" << stream << ");";
            }
            else
            {
                out << nl << param << ".ice_readMembers(" << stream << ");";
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << stream << ".writeEnum((int)" << param << ", " << en->maxValue() << ");";
        }
        else
        {
            out << nl << param << " = (" << fixId(en->scoped()) << ')' << stream << ".readEnum(" << en->maxValue()
                << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, param, marshal, true, stream);
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
                                                      const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
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
            case Builtin::KindValue:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeValue(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << stream << ".readValue(" << tag << ", " << param << ");";
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
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag
                << ", Ice.OptionalFormat.FSize))";
            out << sb;
            out << nl << "int pos = " << stream << ".startSize();";
            writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, customStream);
            out << nl << stream << ".endSize(pos);";
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOptional(" << tag << ", Ice.OptionalFormat.FSize))";
            out << sb;
            out << nl << stream << ".skip(4);";
            string tmp = "tmpVal";
            string typeS = typeToString(type);
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, tmp, marshal, customStream);
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
            out << nl << stream << ".writeValue(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".readValue(" << tag << ", " << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag << ", "
                << getOptionalFormat(st) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".startSize();";
            }
            else
            {
                out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
            }
            writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, customStream);
            if(st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(st) << "))";
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
            string tmp = "tmpVal";
            if(isValueType(st))
            {
                out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
            }
            else
            {
                out << nl << typeS << ' ' << tmp << " = null;";
            }
            writeMarshalUnmarshalCode(out, type, tmp, marshal, customStream);
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
            out << nl << "if(" << stream << ".readOptional(" << tag << ", Ice.OptionalFormat.Size))";
            out << sb;
            string typeS = typeToString(type);
            string tmp = "tmpVal";
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, tmp, marshal, customStream);
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
        writeOptionalSequenceMarshalUnmarshalCode(out, seq, param, tag, marshal, stream);
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    assert(d);
    TypePtr keyType = d->keyType();
    TypePtr valueType = d->valueType();
    if(marshal)
    {
        out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag << ", "
            << getOptionalFormat(d) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << "int pos = " << stream << ".startSize();";
        }
        else
        {
            out << nl << stream << ".writeSize(" << param << ".Value == null ? 1 : " << param << ".Value.Count * "
                << (keyType->minWireSize() + valueType->minWireSize()) << " + (" << param
                << ".Value.Count > 254 ? 5 : 1));";
        }
        writeMarshalUnmarshalCode(out, type, param + ".Value", marshal, customStream);
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".endSize(pos);";
        }
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(d) << "))";
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
        string tmp = "tmpVal";
        out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
        writeMarshalUnmarshalCode(out, type, tmp, marshal, customStream);
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
                                                      bool useHelper,
                                                      const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
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

    const string genericPrefix = "cs:generic:";
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

    const bool isArray = !isGeneric;
    const string limitID = isArray ? "Length" : "Count";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    ClassDefPtr clsDef;
    if(proxy)
    {
        clsDef = proxy->_class()->definition();
    }
    bool isObjectProxySeq = clsDef && !clsDef->isInterface() && clsDef->allOperations().size() == 0;
    Builtin::Kind kind = builtin ? builtin->kind() : Builtin::KindObjectProxy;

    if(builtin || isObjectProxySeq)
    {
        switch(kind)
        {
            case Builtin::KindValue:
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
                            out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                            out << sb;
                            out << nl << stream << ".writeProxy(" << param << "_tmp[ix]);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                                << "> e = " << param << ".GetEnumerator();";
                            out << nl << "while(e.MoveNext())";
                            out << sb;
                            string func = (kind == Builtin::KindObject ||
                                           kind == Builtin::KindValue) ? "writeValue" : "writeProxy";
                            out << nl << stream << '.' << func << "(e.Current);";
                            out << eb;
                        }
                    }
                    else
                    {
                        out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                        out << sb;
                        string func = (kind == Builtin::KindObject ||
                                       kind == Builtin::KindValue) ? "writeValue" : "writeProxy";
                        out << nl << stream << '.' << func << '(' << param << "[ix]);";
                        out << eb;
                    }
                    out << eb;
                }
                else
                {
                    out << nl << "int " << param << "_lenx = " << stream << ".readAndCheckSeqSize("
                        << static_cast<unsigned>(type->minWireSize()) << ");";
                    if(!isStack)
                    {
                        out << nl << param << " = new ";
                    }
                    if((kind == Builtin::KindObject || kind == Builtin::KindValue))
                    {
                        if(isArray)
                        {
                            out << "Ice.Value[" << param << "_lenx];";
                        }
                        else if(isCustom)
                        {
                            out << "global::" << genericType << "<Ice.Value>();";
                        }
                        else if(isGeneric)
                        {
                            out << "_System.Collections.Generic." << genericType << "<Ice.Value>(";
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
                        out << nl << "for(int ix = 0; ix < " << param << "_lenx; ++ix)";
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
                        out << nl << "IceInternal." << patcherName << "Patcher<Ice.Value> p = new IceInternal."
                            << patcherName << "Patcher<Ice.Value>(\"::Ice::Object\", " << param << ", ix);";
                        out << nl << stream << ".readValue(p.patch);";
                    }
                    else
                    {
                        if(isStack)
                        {
                            out << nl << "Ice.ObjectPrx[] " << param << "_tmp = new Ice.ObjectPrx[" << param << "_lenx];";
                        }
                        else if(isArray)
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

                        out << nl << "for(int ix = 0; ix < " << param << "_lenx; ++ix)";
                        out << sb;
                        if(isArray || isStack)
                        {
                            string v = isArray ? param : param + "_tmp";
                            out << nl << v << "[ix] = " << stream << ".readProxy();";
                        }
                        else
                        {
                            out << nl << "Ice.ObjectPrx val = new Ice.ObjectPrxHelperBase();";
                            out << nl << "val = " << stream << ".readProxy();";
                            out << nl << param << "." << addMethod << "(val);";
                        }
                    }
                    out << eb;

                    if(isStack)
                    {
                        out << nl << "_System.Array.Reverse(" << param << "_tmp);";
                        out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                            << param << "_tmp);";
                    }
                }
                break;
            }
            default:
            {
                string prefix = "cs:serializable:";
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
                    else if(isCustom)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : "
                            << param << ".Count, " << param << ");";
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : "
                            << param << ".Count, " << param << ");";
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
                        out << nl << "int szx = " << stream << ".readSize();";
                        out << nl << "for(int ix = 0; ix < szx; ++ix)";
                        out << sb;
                        out << nl << param << ".Add(" << stream << ".read" << func << "());";
                        out << eb;
                        out << eb;
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".read" << func << "Seq(out " << param << ");";
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
                    << "> e = " << param << ".GetEnumerator();";
                out << nl << "while(e.MoveNext())";
                out << sb;
                out << nl << stream << ".writeValue(e.Current);";
                out << eb;
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".writeValue(" << param << "[ix]);";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            out << nl << param << " = new ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "[]", "szx");
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
                    out << "szx";
                }
                out << ")";
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx)";
            }
            out << ';';
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
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
                << patcherName << "Patcher<" << typeS << ">(\"" << scoped << "\", " << param << ", ix);";
            out << nl << stream << ".readValue(spx.patch);";
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
                    out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                }
                else
                {
                    out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                        << "> e = " << param << ".GetEnumerator();";
                    out << nl << "while(e.MoveNext())";
                }
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            }
            out << sb;
            string call;
            if(isGeneric && !isList && !isStack)
            {
                if(isValueType(type))
                {
                    call = "e.Current";
                }
                else
                {
                    call = "(e.Current == null ? new ";
                    call += typeS + "() : e.Current)";
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
                    call += "[ix] == null ? new " + typeS + "() : " + param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                }
                call += "[ix]";
                if(!isValueType(type))
                {
                    call += ")";
                }
            }
            call += ".";
            call += "ice_writeMembers";
            call += "(" + stream + ");";
            out << nl << call;
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isGeneric)
            {
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            else
            {
                out << nl << param << " = new " << fixId(seq->scoped()) << "(szx);";
            }
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                if(!isValueType(st))
                {
                    out << nl << v << "[ix] = new " << typeS << "();";
                }
                out << nl << v << "[ix].ice_readMembers(" << stream << ");";
            }
            else
            {
                out << nl << typeS << " val = new " << typeS << "();";
                out << nl << "val.ice_readMembers(" << stream << ");";
                out << nl << param << "." << addMethod << "(val);";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "_System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "_tmp);";
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
                    out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)" << param << "_tmp[ix], " << en->maxValue() << ");";
                    out << eb;
                }
                else
                {
                    out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                        << "> e = " << param << ".GetEnumerator();";
                    out << nl << "while(e.MoveNext())";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)e.Current, " << en->maxValue() << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".writeEnum((int)" << param << "[ix], " << en->maxValue() << ");";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize(" <<
                static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isGeneric)
            {
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            else
            {
                out << nl << param << " = new " << fixId(seq->scoped()) << "(szx);";
            }
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                out << nl << v << "[ix] = (" << typeS << ')' << stream << ".readEnum(" << en->maxValue() << ");";
            }
            else
            {
                out << nl << param << "." << addMethod << "((" << typeS << ')' << stream << ".readEnum("
                    << en->maxValue() << "));";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "_System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "_tmp);";
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
                out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", " << param << "_tmp[ix]);";
                out << eb;
            }
            else
            {
                out << nl << "_System.Collections.Generic.IEnumerator<" << typeS
                    << "> e = " << param << ".GetEnumerator();";
                out << nl << "while(e.MoveNext())";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", e.Current);";
                out << eb;
            }
        }
        else
        {
            out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            out << sb;
            out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[ix]);";
            out << eb;
        }
        out << eb;
    }
    else
    {
        func = "read";
        out << sb;
        out << nl << "int szx = " << stream << ".readAndCheckSeqSize("
            << static_cast<unsigned>(type->minWireSize()) << ");";
        if(isArray)
        {
            out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else if(isCustom)
        {
            out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
        }
        else if(isStack)
        {
            out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else if(isGeneric)
        {
            out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">();";
        }
        else
        {
            out << nl << param << " = new " << fixId(seq->scoped()) << "(szx);";
        }
        out << nl << "for(int ix = 0; ix < szx; ++ix)";
        out << sb;
        if(isArray || isStack)
        {
            string v = isArray ? param : param + "_tmp";
            out << nl << v << "[ix] = " << helperName << '.' << func << '(' << stream << ");";
        }
        else
        {
            out << nl << param << "." << addMethod << "(" << helperName << '.' << func << '(' << stream << "));";
        }
        out << eb;
        if(isStack)
        {
            out << nl << "_System.Array.Reverse(" << param << "_tmp);";
            out << nl << param << " = new _System.Collections.Generic." << genericType << "<" << typeS << ">("
                << param << "_tmp);";
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
                                                              const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    const TypePtr type = seq->type();
    const string typeS = typeToString(type);
    const string seqS = typeToString(seq);

    string meta;
    const bool isArray = !seq->findMetaData("cs:generic:", meta);
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
            const bool isSerializable = seq->findMetaData("cs:serializable:", meta);

            if(marshal)
            {
                if(isSerializable)
                {
                    out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag
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
                out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
                out << sb;
                if(builtin->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindBool)
                {
                    out << nl << stream << ".skipSize();";
                }
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, true, stream);
                out << nl << param << " = new Ice.Optional<" << seqS << ">(" << tmp << ");";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << param << " = new Ice.Optional<" << seqS << ">();";
                out << eb;
            }
            break;
        }

        case Builtin::KindValue:
        case Builtin::KindObject:
        case Builtin::KindObjectProxy:
        {
            if(marshal)
            {
                out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag << ", "
                    << getOptionalFormat(seq) << "))";
                out << sb;
                out << nl << "int pos = " << stream << ".startSize();";
                writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, true, stream);
                out << nl << stream << ".endSize(pos);";
                out << eb;
            }
            else
            {
                out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
                out << sb;
                out << nl << stream << ".skip(4);";
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, true, stream);
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
            out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag << ", "
                << getOptionalFormat(seq) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".startSize();";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".writeSize(" << param << ".Value == null ? 1 : " << length << " * "
                    << st->minWireSize() << " + (" << length << " > 254 ? 5 : 1));";
            }
            writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, true, stream);
            if(st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".skipSize();";
            }
            string tmp = "tmpVal";
            out << nl << seqS << ' ' << tmp << ';';
            writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, true, stream);
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
        out << nl << "if(" << param << ".HasValue && " << stream << ".writeOptional(" << tag << ", "
            << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << "int pos = " << stream << ".startSize();";
        writeSequenceMarshalUnmarshalCode(out, seq, param + ".Value", marshal, true, stream);
        out << nl << stream << ".endSize(pos);";
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << stream << ".skip(4);";
        string tmp = "tmpVal";
        out << nl << seqS << ' ' << tmp << ';';
        writeSequenceMarshalUnmarshalCode(out, seq, tmp, marshal, true, stream);
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
    //
    // Could do it only when param == "info", but not as good for testing
    //
    string dataMember = "this." + param;
    if(optional)
    {
        const string typeName = typeToString(type, true);
        if(serialize)
        {
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
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
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetByte(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetBoolean(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetInt16(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetInt32(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetInt64(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetSingle(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetDouble(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << " == null ? \"\" : " << dataMember
                        << ");";
                }
                else
                {
                    out << nl << dataMember << " = " << "info.GetString(\"" << param << "\");";
                }
                break;
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindLocalObject:
            {
                const string typeName = typeToString(type, false);
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
                }
                else
                {
                    out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof("
                        << typeName << "));";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(serialize)
                {
                    out << nl << "info.AddValue(\"" << param << "\", " << dataMember
                        << ", typeof(Ice.ObjectPrxHelperBase));";
                }
                else
                {
                    out << nl << dataMember << " = (Ice.ObjectPrx)info.GetValue(\"" << param
                        << "\", typeof(Ice.ObjectPrxHelperBase));";
                }
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "Helper));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
                << "Helper));";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        const string typeName = typeToString(type, false);
        if(serialize)
        {
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
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
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
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
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
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
            out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
        }
        else
        {
            out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
                << "));";
        }
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    assert(d);
    const string typeName = typeToString(type, false);
    if(serialize)
    {
        out << nl << "info.AddValue(\"" << param << "\", " << dataMember << ", typeof(" << typeName << "));";
    }
    else
    {
        out << nl << dataMember << " = (" << typeName << ")info.GetValue(\"" << param << "\", typeof(" << typeName
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
        StringList newGlobalMetaData;
        bool emitWarnings = !dc->suppressWarning("invalid-metadata");
        static const string csPrefix = "cs:";
        static const string clrPrefix = "clr:";

        for(StringList::iterator r = globalMetaData.begin(); r != globalMetaData.end(); ++r)
        {
            string& s = *r;
            string oldS = s;

            if(s.find(clrPrefix) == 0)
            {
                s.replace(0, clrPrefix.size(), csPrefix);
            }

            if(s.find(csPrefix) == 0)
            {
                static const string csAttributePrefix = csPrefix + "attribute:";
                if(s.find(csAttributePrefix) != 0 || s.size() == csAttributePrefix.size())
                {
                    if(emitWarnings)
                    {
                        emitWarning(file, -1, "ignoring invalid global metadata `" + oldS + "'");
                    }
                    continue;
                }
            }
            newGlobalMetaData.push_back(oldS);
        }

        dc->setMetaData(newGlobalMetaData);
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
    const string msg = "ignoring invalid metadata";

    StringList localMetaData = cont->getMetaData();
    StringList newLocalMetaData;

    const UnitPtr unit = cont->unit();
    const DefinitionContextPtr dc = unit->findDefinitionContext(cont->file());
    assert(dc);
    bool emitWarnings = !dc->suppressWarning("invalid-metadata");

    for(StringList::iterator p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        string& s = *p;
        string oldS = s;

        const string csPrefix = "cs:";
        const string clrPrefix = "clr:";

        if(s.find(clrPrefix) == 0)
        {
            s.replace(0, clrPrefix.size(), csPrefix);
        }


        if(s.find(csPrefix) == 0)
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                static const string csGenericPrefix = csPrefix + "generic:";
                if(s.find(csGenericPrefix) == 0)
                {
                    string type = s.substr(csGenericPrefix.size());
                    if(type == "LinkedList" || type == "Queue" || type == "Stack")
                    {
                        if(!isClassType(seq->type()))
                        {
                            newLocalMetaData.push_back(s);
                            continue;
                        }
                    }
                    else if(!type.empty())
                    {
                        newLocalMetaData.push_back(s);
                        continue; // Custom type or List<T>
                    }
                }
                static const string csSerializablePrefix = csPrefix + "serializable:";
                if(s.find(csSerializablePrefix) == 0)
                {
                    string meta;
                    if(cont->findMetaData(csPrefix + "generic:", meta))
                    {
                        if(emitWarnings)
                        {
                            emitWarning(cont->file(), cont->line(), msg + " `" + meta + "':\n" +
                                        "serialization can only be used with the array mapping for byte sequences");
                        }
                        continue;
                    }
                    string type = s.substr(csSerializablePrefix.size());
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                    if(!type.empty() && builtin && builtin->kind() == Builtin::KindByte)
                    {
                        newLocalMetaData.push_back(s);
                        continue;
                    }
                }
            }
            else if(StructPtr::dynamicCast(cont))
            {
                if(s.substr(csPrefix.size()) == "class")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                if(s.substr(csPrefix.size()) == "property")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                static const string csImplementsPrefix = csPrefix + "implements:";
                if(s.find(csImplementsPrefix) == 0)
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
            }
            else if(ClassDefPtr::dynamicCast(cont))
            {
                if(s.substr(csPrefix.size()) == "property")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                static const string csImplementsPrefix = csPrefix + "implements:";
                if(s.find(csImplementsPrefix) == 0)
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
            }
            else if(DictionaryPtr::dynamicCast(cont))
            {
                static const string csGenericPrefix = csPrefix + "generic:";
                if(s.find(csGenericPrefix) == 0)
                {
                    string type = s.substr(csGenericPrefix.size());
                    if(type == "SortedDictionary" ||  type == "SortedList")
                    {
                        newLocalMetaData.push_back(s);
                        continue;
                    }
                }
            }

            static const string csAttributePrefix = csPrefix + "attribute:";
            static const string csTie = csPrefix + "tie";
            if(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size())
            {
                newLocalMetaData.push_back(s);
                continue;
            }
            else if(s.find(csTie) == 0 && s.size() == csTie.size())
            {
                newLocalMetaData.push_back(s);
                continue;
            }

            if(emitWarnings)
            {
                emitWarning(cont->file(), cont->line(), msg + " `" + oldS + "'");
            }
            continue;
        }
        else if(s == "delegate")
        {
            ClassDefPtr cl = ClassDefPtr::dynamicCast(cont);
            if(cl && cl->isDelegate())
            {
                newLocalMetaData.push_back(s);
                continue;
            }

            if(emitWarnings)
            {
                emitWarning(cont->file(), cont->line(), msg + " `" + s + "'");
            }
            continue;
        }
        newLocalMetaData.push_back(s);
    }

    cont->setMetaData(newLocalMetaData);
}
