// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/CsUtil.h>
#include <Slice/DotNetNames.h>
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
Slice::CsGenerator::typeToString(const TypePtr& type)
{
    if(!type)
    {
        return "void";
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
        "System.Object"
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
    if(seq && !seq->hasMetaData("clr:collection"))
    {
        return typeToString(seq->type()) + "[]";
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
            if(!isValueType((*i)->type()))
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
                                              bool streamingAPI,
                                              bool isOutParam,
                                              const string& patchParams)
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
                    if(isOutParam)
                    {
                        out << nl << "IceInternal.ParamPatcher " << param
                            << "_PP = new IceInternal.ParamPatcher(typeof(Ice.Object), \"::Ice::Object\");";
                        out << nl << stream << ".readObject(";
                        if(streamingAPI)
                        {
                            out << "(Ice.ReadObjectCallback)";
                        }
                        out << param << "_PP);";
                    }
                    else
                    {
                        out << nl << stream << ".readObject(";
                        if(streamingAPI)
                        {
                            out <<  "(Ice.ReadObjectCallback)";
                        }
                        out << "new Patcher__(" << patchParams << "));";
                    }
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
            if(isOutParam)
            {
                ContainedPtr contained = ContainedPtr::dynamicCast(type);
                out << nl << "IceInternal.ParamPatcher " << param
                    << "_PP = new IceInternal.ParamPatcher(typeof(" << typeToString(type) << "), \"" << contained->scoped() << "\");";
                out << nl << stream << ".readObject(";
                if(streamingAPI)
                {
                    out << "(Ice.ReadObjectCallback)";
                }
                out << param << "_PP);";
            }
            else
            {
                out << nl << stream << ".readObject(";
                if(streamingAPI)
                {
                    out << "(Ice.ReadObjectCallback)";
                }
                out << "new Patcher__(" << patchParams << "));";
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            if(streamingAPI)
            {
                out << nl << param << ".ice_write(" << stream << ");";
            }
            else
            {
                out << nl << param << ".write__(" << stream << ");";
            }
        }
        else
        {
            string typeS = typeToString(type);
            out << nl << param << " = new " << typeS << "();";
            if(streamingAPI)
            {
                out << nl << param << ".ice_read(" << stream << ");";
            }
            else
            {
                out << nl << param << ".read__(" << stream << ");";
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        string func;
        string cast;
        size_t sz = en->getEnumerators().size();
        if(sz <= 0x7f)
        {
            func = marshal ? "writeByte" : "readByte";
            cast = marshal ? string("(byte)") : "(" + fixId(en->scoped()) + ")";
        }
        else if(sz <= 0x7fff)
        {
            func = marshal ? "writeShort" : "readShort";
            cast = marshal ? string("(short)") : "(" + fixId(en->scoped()) + ")";
        }
        else
        {
            func = marshal ? "writeInt" : "readInt";
            cast = marshal ? string("(int)") : "(" + fixId(en->scoped()) + ")";
        }
        if(marshal)
        {
            out << nl << stream << '.' << func << '(' << cast << param << ");";
        }
        else
        {
            out << nl << param << " = " << cast << stream << '.' << func << "()" << ';';
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, param, marshal, streamingAPI);
        return;
    }

    assert(ConstructedPtr::dynamicCast(type));
    string typeS = typeToString(type);
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
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                      const SequencePtr& seq,
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
    
    TypePtr type = seq->type();
    string typeS = typeToString(type);

    bool isArray = !seq->hasMetaData("clr:collection");
    string limitID = isArray ? "Length" : "Count";

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
                    out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
                    out << sb;
                    string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
                    out << nl << stream << '.' << func << '(' << param << "[ix__]);";
                    out << eb;
                    out << eb;
                }
                else
                {
                    out << nl << "int lenx__ = " << stream << ".readSize();";
                    if(!streamingAPI)
                    {
                        if(builtin->isVariableLength())
                        {
                            out << nl << stream << ".startSeq(lenx__, "
                                << static_cast<unsigned>(builtin->minWireSize()) << ");";
                        }
                        else
                        {
                            out << nl << stream << ".checkFixedSeq(lenx__, "
                                << static_cast<unsigned>(builtin->minWireSize()) << ");";
                        }
                    }
                    out << nl << param << " = new ";
                    if(builtin->kind() == Builtin::KindObject)
                    {
                        if(isArray)
                        {
                            out << "Ice.Object[lenx__];";
                        }
                        else
                        {
                            out << typeToString(seq) << "(lenx__);";
                        }
                        out << nl << "for(int ix__ = 0; ix__ < lenx__; ++ix__)";
                        out << sb;
                        out << nl << stream << ".readObject(";
                        if(streamingAPI)
                        {
                            out << "(ReadObjectCallback)";
                        }
                        out << "new IceInternal.SequencePatcher(" << param << ", typeof(Ice.Object), ix__));";
                    }
                    else
                    {
                        if(isArray)
                        {
                            out << "Ice.ObjectPrx[lenx__];";
                        }
                        else
                        {
                            out << typeToString(seq) << "(lenx__);";
                        }
                        out << nl << "for(int ix__ = 0; ix__ < lenx__; ++ix__)";
                        out << sb;
                        if(isArray)
                        {
                            out << nl << param << "[ix__] = " << stream << ".readProxy();";
                        }
                        else
                        {
                            out << nl << "Ice.ObjectPrx val__ = new Ice.ObjectPrxHelperBase();";
                            out << nl << "val__ = " << stream << ".readProxy();";
                            out << nl << param << ".Add(val__);";
                        }
                    }
                    if(!streamingAPI && builtin->isVariableLength())
                    {
                        out << nl << stream << ".checkSeq();";
                        out << nl << stream << ".endElement();";
                    }
                    out << eb;
                    if(!streamingAPI && builtin->isVariableLength())
                    {
                        out << nl << stream << ".endSeq(lenx__);";
                    }
                }
                break;
            }
            default:
            {
                typeS[0] = toupper(typeS[0]);
                if(marshal)
                {
                    out << nl << stream << ".write" << typeS << "Seq(" << param;
                    if(!isArray)
                    {
                        out << " == null ? null : " << param << ".ToArray()";
                    }
                    out << ");";
                }
                else
                {
                    if(!isArray)
                    {
                        out << nl << param << " = new " << fixId(seq->scoped())
                            << '(' << stream << ".read" << typeS << "Seq());";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".read" << typeS << "Seq();";
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
            out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
            out << sb;
            out << nl << stream << ".writeObject(" << param << "[ix__]);";
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readSize();";
            if(!streamingAPI)
            {
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
                }
            }
            out << nl << param << " = new ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "[]", "szx__");
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << ';';
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;
            out << nl << "IceInternal.SequencePatcher spx = new IceInternal.SequencePatcher("
                << param << ", " << "typeof(" << typeS << "), ix__);";
            out << nl << stream << ".readObject(";
            if(streamingAPI)
            {
                out << "(Ice.ReadObjectCallback)";
            }
            out << "spx);";
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".checkSeq();";
                out << nl << stream << ".endElement();";
            }
            out << eb;
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".endSeq(szx__);";
            }
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
            out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
            out << sb;
            if(streamingAPI)
            {
                out << nl << param << "[ix__].ice_write(" << stream << ");";
            }
            else
            {
                out << nl << param << "[ix__].write__(" << stream << ");";
            }
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readSize();";
            if(!streamingAPI)
            {
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
                }
            }
            out << nl << param << " = new ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "[]", "szx__");
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << ';';
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;
            if(isArray)
            {
                if(!isValueType(st))
                {
                    out << nl << param << "[ix__] = new " << typeS << "();";
                }
                if(streamingAPI)
                {
                    out << nl << param << "[ix__].ice_read(" << stream << ");";
                }
                else
                {
                    out << nl << param << "[ix__].read__(" << stream << ");";
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
                out << nl << param << ".Add(val__);";
            }
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".checkSeq();";
                out << nl << stream << ".endElement();";
            }
            out << eb;
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".endSeq(szx__);";
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
            out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
            out << sb;
            out << nl << stream << ".writeByte((byte)" << param << "[ix__]);";
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx__ = " << stream << ".readSize();";
            if(!streamingAPI)
            {
                out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
            }
            out << nl << param << " = new ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "[]", "szx__");
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << ';';
            out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
            out << sb;
            if(isArray)
            {
                out << nl << param << "[ix__] = (" << typeS << ')' << stream << ".readByte();";
            }
            else
            {
                out << nl << param << ".Add((" << typeS << ')' << stream << ".readByte());";
            }
            out << eb;
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
        out << nl << "for(int ix__ = 0; ix__ < " << param << '.' << limitID << "; ++ix__)";
        out << sb;
        out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[ix__]);";
        out << eb;
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
        out << nl << "int szx__ = " << stream << ".readSize();";
        if(!streamingAPI)
        {
            if(type->isVariableLength())
            {
                out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
            }
            else
            {
                out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ");";
            }
        }
        out << nl << param << " = new ";
        if(isArray)
        {
            out << toArrayAlloc(typeS + "[]", "szx__");
        }
        else
        {
            out << fixId(seq->scoped()) << "(szx__)";
        }
        out << ';';
        out << nl << "for(int ix__ = 0; ix__ < szx__; ++ix__)";
        out << sb;
        if(isArray)
        {
            out << nl << param << "[ix__] = " << helperName << '.' << func << '(' << stream << ");";
        }
        else
        {
            out << nl << param << ".Add(" << helperName << '.' << func << '(' << stream << "));";
        }
        if(!streamingAPI && type->isVariableLength())
        {
            if(!SequencePtr::dynamicCast(type))
            {
                out << nl << stream << ".checkSeq();";
            }
            out << nl << stream << ".endElement();";
        }
        out << eb;
        if(!streamingAPI && type->isVariableLength())
        {
            out << nl << stream << ".endSeq(szx__);";
        }
        out << eb;
    }

    return;
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

Slice::CsGenerator::MetaDataVisitor::MetaDataVisitor()
    : _globalMetaDataDone(false)
{
}

bool
Slice::CsGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!_globalMetaDataDone)
    {
        //
        // Validate global metadata.
        //
        DefinitionContextPtr dc = p->definitionContext();
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        string file = dc->filename();
        static const string prefix = "cs:";
        for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
        {
            string s = *q;
            if(_history.count(s) == 0)
            {
                if(s.find(prefix) == 0)
                {
                    static const string attributePrefix = "cs:attribute:";
                    if(s.find(attributePrefix) != 0 || s.size() == attributePrefix.size())
                    {
                        cout << file << ": warning: ignoring invalid global metadata `" << s << "'" << endl;
                    }
                }
                _history.insert(s);
            }
        }
        _globalMetaDataDone = true;
    }
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
    DefinitionContextPtr dc = cont->definitionContext();
    assert(dc);
    string file = dc->filename();

    StringList localMetaData = cont->getMetaData();

    StringList::const_iterator p;

    for(p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        string s = *p;

        if(s.find("cs:") == 0) // TODO: remove this statement once "cs:" is a hard error.
        {
            if(SequencePtr::dynamicCast(cont))
            {
                if(s.substr(3) == "collection")
                {
                    cout << file << ":" << cont->line() << ": warning: `cs:' metadata prefix is deprecated; "
                         << "use `clr:' instead" << endl;
                    cont->addMetaData("clr:collection");
                }
            }
            else if(StructPtr::dynamicCast(cont))
            {
                if(s.substr(3) == "class")
                {
                    cout << file << ":" << cont->line() << ": warning: `cs:' metadata prefix is deprecated; "
                         << "use `clr:' instead" << endl;
                    cont->addMetaData("clr:class");
                }
            }
            else if(s.find("cs:attribute:") == 0)
            {
                ; // Do nothing, "cs:attribute:" is OK
            }
            else
            {
                cout << file << ":" << cont->line() << ": warning: ignoring invalid metadata `" << s << "'" << endl;
            }
        } // End TODO

        string prefix = "clr:";
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                if(SequencePtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "collection")
                    {
                        continue;
                    }
                }
                if(StructPtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "class")
                    {
                        continue;
                    }
                    if(s.substr(prefix.size()) == "property")
                    {
                        continue;
                    }
                }
                if(ClassDefPtr::dynamicCast(cont))
                {
                    if(s.substr(prefix.size()) == "property")
                    {
                        continue;
                    }
                }
                cout << file << ":" << cont->line() << ": warning: ignoring invalid metadata `" << s << "'" << endl;
            }
            _history.insert(s);
        }

        prefix = "cs:";
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                if(s.substr(prefix.size()) == "attribute:")
                {
                    continue;
                }
                cout << file << ":" << cont->line() << ": warning: ignoring invalid metadata `" << s << "'" << endl;
            }
            _history.insert(s);
        }
    }
}
