// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/VbUtil.h>
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
        "AddHandler", "AddressOf", "Alias", "And", "AndAlso", "Ansi", "As", "Assembly", "Auto",
        "Boolean", "ByRef", "Byte", "ByVal", "Call", "Case", "Catch", "CBool", "CByte", "CChar",
        "CDate", "CDbl", "CDec", "Char", "CInt", "Class", "CLng", "CObj", "Const", "CShort", "CSng",
        "CStr", "CType", "Date", "Decimal", "Declare", "Default", "Delegate", "Dim", "DirectCast",
        "Do", "Double", "Each", "Else", "ElseIf", "End", "EndIf", "Enum", "Erase", "Error", "Event",
        "Exit", "False", "Finally", "For", "Friend", "Function", "Get", "GetType", "GoSub", "GoTo",
        "Handles", "If", "Implements", "Imports", "In", "Inherits", "Integer", "Interface", "Is",
        "Let", "Lib", "Like", "Long", "Loop", "Me", "Mod", "Module", "MustInherit", "MustOverride",
        "MyBase", "MyClass", "Namespace", "New", "Next", "Not", "Nothing", "NotInheritable", 
        "NotOverridable", "Object", "On", "Option", "Optional", "Or", "OrElse", "Overloads", "Overridable",
        "Overrides", "ParamArray", "Preserve", "Private", "Property", "Protected", "Public", "RaiseEvent",
        "ReadOnly", "ReDim", "REM", "RemoveHandler", "Resume", "Return", "Select", "Set",
        "Shadows", "Shared", "Short", "Single", "Static", "Step", "Stop", "String", "Structure",
        "Sub", "SyncLock", "Then", "Throw", "To", "True", "Try", "TypeOf", "Unicode", "Until",
        "Variant", "Wend", "When", "While", "With", "WithEvents", "WriteOnly", "Xor"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "[" + name + "]";
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
// but with all components that are VB keywords replaced by
// their "[]"-surrounded version; otherwise, if the passed name is
// not scoped, but a VB keyword, return the "[]"-surrounded name;
// otherwise, check if the name is one of the method names of baseTypes;
// if so, prefix it with _Ice_; otherwise, reutrn the name unchanged.
//
string
Slice::VbGenerator::fixId(const string& name, int baseTypes, bool mangleCasts)
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
    for(StringList::const_iterator j = ids.begin(); j != ids.end(); ++j)
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
Slice::VbGenerator::typeToString(const TypePtr& type)
{
    if(!type)
    {
        return "";
    }

    static const char* builtinTable[] =
    {
        "Byte",
        "Boolean",
        "Short",
        "Integer",
        "Long",
        "Single",
        "Double",
        "String",
        "Ice.Object",
        "Ice.ObjectPrx",
        "Ice.LocalObject"
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
        return typeToString(seq->type()) + "()";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixId(contained->scoped());
    }

    return "???";
}

bool
Slice::VbGenerator::isValueType(const TypePtr& type)
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
Slice::VbGenerator::writeMarshalUnmarshalCode(Output &out,
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
                    out << nl << stream << ".writeByte(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << param << ')';
                }
                else
                {
                    if(isOutParam)
                    {
                        out << nl << "Dim " << param
                            << "_PP As IceInternal.ParamPatcher = New IceInternal.ParamPatcher(GetType(Ice.Object), "
                            << "\"::Ice::Object\")";
                        if(streamingAPI)
                        {
                            out << nl << stream << ".readObject(CType(" << param << "_PP, Ice.ReadObjectCallback))";
                        }
                        else
                        {
                            out << nl << stream << ".readObject(" << param << "_PP)";
                        }
                    }
                    else
                    {
                        if(streamingAPI)
                        {
                            out << nl << stream << ".readObject(CType(New Patcher__(" << patchParams
                                << ", Ice.ReadObjectCallback)))";
                        }
                        else
                        {
                            out << nl << stream << ".readObject(New Patcher__(" << patchParams << "))";
                        }
                    }
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ')';
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy()";
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
        ContainedPtr contained = ContainedPtr::dynamicCast(type);
        string helperName = fixId((contained ? contained->scoped() : typeToString(type)) + "Helper");
        if(marshal)
        {
            out << nl << helperName << ".write";
            if(!streamingAPI)
            {
                out << "__";
            }
            out << '(' << stream << ", " << param << ')';
        }
        else
        {
            out << nl << param << " = " << helperName << ".read";
            if(!streamingAPI)
            {
                out << "__";
            }
            out << "(" << stream << ')';
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << param << ")";
        }
        else
        {
            if(isOutParam)
            {
                ContainedPtr contained = ContainedPtr::dynamicCast(type);
                out << nl << "Dim " << param
                    << "_PP As IceInternal.ParamPatcher = New IceInternal.ParamPatcher(GetType("
                    << typeToString(type) << "), \"" << contained->scoped() << "\")";
                if(streamingAPI)
                {
                    out << nl << stream << ".readObject(CType(" << param << "_PP, Ice.ReadObjectCallback))";
                }
                else
                {
                    out << nl << stream << ".readObject(" << param << "_PP)";
                }
            }
            else
            {
                if(streamingAPI)
                {
                    out << nl << stream << ".readObject(New Patcher__(CType(" << patchParams
                        << ", Ice.ReadObjectCallback)))";
                }
                else
                {
                    out << nl << stream << ".readObject(New Patcher__(" << patchParams << "))";
                }
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
                out << nl << param << ".ice_write(" << stream << ')';
            }
            else
            {
                out << nl << param << ".write__(" << stream << ')';
            }
        }
        else
        {
            string typeS = typeToString(type);
            out << nl << param << " = New " << typeS;
            if(streamingAPI)
            {
                out << nl << param << ".ice_read(" << stream << ")";
            }
            else
            {
                out << nl << param << ".read__(" << stream << ")";
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
            cast = marshal ? string("Byte") : fixId(en->scoped());
        }
        else if(sz <= 0x7fff)
        {
            func = marshal ? "writeShort" : "readShort";
            cast = marshal ? string("Short") : fixId(en->scoped());
        }
        else
        {
            func = marshal ? "writeInt" : "readInt";
            cast = marshal ? string("Integer") : fixId(en->scoped());
        }
        if(marshal)
        {
            out << nl << stream << '.' << func << "(CType(" << param << ", " << cast << "))";
        }
        else
        {
            out << nl << param << " = CType(" << stream << '.' << func << "(), " << cast << ')';
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
    string helperName = fixId(ContainedPtr::dynamicCast(type)->scoped() + "Helper");
    if(marshal)
    {
        out << nl << helperName << ".write(" << stream << ", " << param << ')';
    }
    else
    {
        out << nl << param << " = " << helperName << ".read(" << stream << ')';
    }
}

void
Slice::VbGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
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
                    out << nl << "If " << param << " Is Nothing Then";
                    out.inc();
                    out << nl << stream << ".writeSize(0)";
                    out.dec();
                    out << nl << "Else";
                    out.inc();
                    out << nl << stream << ".writeSize(" << param << '.' << limitID << ")";
                    out << nl << "For ix__ As Integer = 0 To " << param << '.' << limitID << " - 1";
                    out.inc();
                    string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
                    out << nl << stream << '.' << func << '(' << param << "(ix__))";
                    out.dec();
                    out << nl << "Next";
                    out.dec();
                    out << nl << "End If";
                }
                else
                {
                    out << nl << "For block__ As Integer = 0 To 0";
                    out.inc();
                    out << nl << "Dim lenx__ As Integer = " << stream << ".readSize()";
                    if(!streamingAPI)
                    {
                        if(builtin->isVariableLength())
                        {
                            out << nl << stream << ".startSeq(lenx__, " << static_cast<unsigned>(builtin->minWireSize())
                                << ")";
                        }
                        else
                        {
                            out << nl << stream << ".checkFixedSeq(lenx__, "
                                << static_cast<unsigned>(builtin->minWireSize()) << ")";
                        }
                    }
                    out << nl << param << " = New ";
                    if(builtin->kind() == Builtin::KindObject)
                    {
                        if(isArray)
                        {
                            out << "Ice.Object(lenx__ - 1) {}";
                        }
                        else
                        {
                            out << typeToString(seq);
                        }
                        out << nl << "For ix__ As Integer = 0 To lenx__ - 1";
                        out.inc();
                        if(streamingAPI)
                        {
                            out << nl << stream << ".readObject(CType(New IceInternal.SequencePatcher("
                                << param << ", GetType(Ice.Object), ix__), Ice.ReadObjectCallback))";
                        }
                        else
                        {
                            out << nl << stream << ".readObject(New IceInternal.SequencePatcher("
                                << param << ", GetType(Ice.Object), ix__))";
                        }
                        out.dec();
                        out << nl << "Next";
                    }
                    else
                    {
                        if(isArray)
                        {
                            out << "Ice.ObjectPrx(lenx__ - 1) {}";
                        }
                        else
                        {
                            out << typeToString(seq);
                        }
                        out << nl << "For ix__ As Integer = 0 To lenx__ - 1";
                        out.inc();
                        if(isArray)
                        {
                            out << nl << param << "(ix__) = " << stream << ".readProxy()";
                        }
                        else
                        {
                            out << nl << "Dim val__ As Ice.ObjectPrx = New Ice.ObjectPrxHelperBase";
                            out << nl << "val__ = " << stream << ".readProxy()";
                            out << nl << param << ".Add(val__)";
                        }
                        out.dec();
                        out << nl << "Next";
                    }
                    if(!streamingAPI && builtin->isVariableLength())
                    {
                        out << nl << stream << ".checkSeq()";
                        out << nl << stream << ".endElement()";
                        out << nl << stream << ".endSeq(lenx__)";
                    }
                    out.dec();
                    out << nl << "Next";
                }
                break;
            }
            default:
            {
                string marshalName = builtin->kindAsString();
                marshalName[0] = toupper(marshalName[0]);
                if(marshal)
                {
                    out << nl << stream << ".write" << marshalName << "Seq(" << param;
                    if(!isArray)
                    {
                        out << ".ToArray()";
                    }
                    out << ')';
                }
                else
                {
                    if(!isArray)
                    {
                        out << nl << param << " = New " << fixId(seq->scoped())
                            << '(' << stream << ".read" << marshalName << "Seq())";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".read" << marshalName << "Seq()";
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
        out << nl << "For block__ As Integer = 0 To 0";
        out.inc();
        if(marshal)
        {
            out << nl << "If " << param << " Is Nothing";
            out.inc();
            out << nl << stream << ".writeSize(0)";
            out.dec();
            out << nl << "Else";
            out.inc();
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ")";
            out << nl << "For ix__ As Integer = 0 To " << param << '.' << limitID << " - 1";
            out.inc();
            out << nl << stream << ".writeObject(" << param << "(ix__))";
            out.dec();
            out << nl << "Next";
            out.dec();
            out << nl << "End If";
        }
        else
        {
            out << nl << "Dim szx__ As Integer = " << stream << ".readSize()";
            if(!streamingAPI)
            {
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ')';
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ')';
                }
            }
            out << nl << param << " = New ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "()", "szx__ - 1") << " {}";
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << nl << "For ix__ As Integer = 0 To szx__ - 1";
            out.inc();
            out << nl << "Dim spx__ As IceInternal.SequencePatcher = New IceInternal.SequencePatcher("
                << param << ", " << "GetType(" << typeS << "), ix__)";
            if(streamingAPI)
            {
                out << nl << stream << ".readObject(CType(spx__, Ice.ReadObjectCallback))";
            }
            else
            {
                out << nl << stream << ".readObject(spx__)";
            }
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".checkSeq()";
                out << nl << stream << ".endElement()";
            }
            out.dec();
            out << nl << "Next";
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".endSeq(szx__)";
            }
        }
        out.dec();
        out << nl << "Next";
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        out << nl << "For block__ As Integer = 0 To 0";
        out.inc();
        if(marshal)
        {
            out << nl << "If " << param << " Is Nothing";
            out.inc();
            out << nl << stream << ".writeSize(0)";
            out.dec();
            out << nl << "Else";
            out.inc();
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ")";
            out << nl << "For ix__ As Integer = 0 To " << param << '.' << limitID << " - 1";
            out.inc();
            if(streamingAPI)
            {
                out << nl << param << "(ix__).ice_write(" << stream << ")";
            }
            else
            {
                out << nl << param << "(ix__).write__(" << stream << ")";
            }
            out.dec();
            out << nl << "Next";
            out.dec();
            out << nl << "End If";
        }
        else
        {
            out << nl << "Dim szx__ As Integer = " << stream << ".readSize()";
            if(!streamingAPI)
            {
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ')';
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ')';
                }
            }
            out << nl << param << " = New ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "()", "szx__ - 1") << " {}";
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << nl << "For ix__ As Integer = 0 To szx__ - 1";
            out.inc();
            if(isArray)
            {
                if(!isValueType(st))
                {
                    out << nl << param << "(ix__) = New " << typeS;
                }
                if(streamingAPI)
                {
                    out << nl << param << "(ix__).ice_read(" << stream << ")";
                }
                else
                {
                    out << nl << param << "(ix__).read__(" << stream << ")";
                }
            }
            else
            {
                out << nl << "Dim val__ As " << typeS << " = New " << typeS;
                if(streamingAPI)
                {
                    out << nl << "val__.ice_read(" << stream << ')';
                }
                else
                {
                    out << nl << "val__.read__(" << stream << ')';
                }
                out << nl << param << ".Add(val__)";
            }
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".checkSeq()";
                out << nl << stream << ".endElement()";
            }
            out.dec();
            out << nl << "Next";
            if(!streamingAPI && type->isVariableLength())
            {
                out << nl << stream << ".endSeq(szx__)";
            }
        }
        out.dec();
        out << nl << "Next";
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        out << nl << "For block__ As Integer = 0 To 0";
        out.inc();
        if(marshal)
        {
            out << nl << "If " << param << " Is Nothing";
            out.inc();
            out << nl << stream << ".writeSize(0)";
            out.dec();
            out << nl << "Else";
            out.inc();
            out << nl << stream << ".writeSize(" << param << '.'<< limitID << ')';
            out << nl << "For ix__ As Integer = 0 To " << param << '.' << limitID << " - 1";
            out.inc();
            out << nl << stream << ".writeByte(CType(" << param << "(ix__), Byte))";
            out.dec();
            out << nl << "Next";
            out.dec();
            out << nl << "End If";
        }
        else
        {
            out << nl << "Dim szx__ As Integer = " << stream << ".readSize()";
            if(!streamingAPI)
            {
                out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ')';
            }
            out << nl << param << " = New ";
            if(isArray)
            {
                out << toArrayAlloc(typeS + "()", "szx__ - 1") << " {}";
            }
            else
            {
                out << fixId(seq->scoped()) << "(szx__)";
            }
            out << nl << "For ix__ As Integer = 0 To szx__ - 1";
            out.inc();
            if(isArray)
            {
                out << nl << param << "(ix__) = CType(" << stream << ".readByte(), " << typeS << ')';
            }
            else
            {
                out << nl << param << ".Add(CType(" << stream << ".readByte(), " << typeS << "))";
            }
            out.dec();
            out << nl << "Next";
        }
        out.dec();
        out << nl << "Next";
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
        out << nl << "If " << param << " Is Nothing";
        out.inc();
        out << nl << stream << ".writeSize(0)";
        out.dec();
        out << nl << "Else";
        out.inc();
        out << nl << stream << ".writeSize(" << param << '.' << limitID << ")";
        out << nl << "For ix__ As Integer = 0 To " << param << '.' << limitID << " - 1";
        out.inc();
        out << nl << helperName << '.' << func << '(' << stream << ", " << param << "(ix__))";
        out.dec();
        out << nl << "Next";
        out.dec();
        out << nl << "End If";
    }
    else
    {
        func = "read";
        if(!streamingAPI && ProxyPtr::dynamicCast(type))
        {
            func += "__";
        }
        out << nl << "For block__ As Integer = 0 To 0";
        out.inc();
        out << nl << "Dim szx__ As Integer = " << stream << ".readSize()";
        if(!streamingAPI)
        {
            if(type->isVariableLength())
            {
                out << nl << stream << ".startSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ")";
            }
            else
            {
                out << nl << stream << ".checkFixedSeq(szx__, " << static_cast<unsigned>(type->minWireSize()) << ")";
            }
        }
        out << nl << param << " = New ";
        if(isArray)
        {
            out << toArrayAlloc(typeS + "()", "szx__ - 1") << " {}";
        }
        else
        {
            out << fixId(seq->scoped()) << "(szx__)";
        }
        out << nl << "For ix__ As Integer = 0 To szx__ - 1";
        out.inc();
        if(isArray)
        {
            out << nl << param << "(ix__) = " << helperName << '.' << func << '(' << stream << ")";
        }
        else
        {
            out << nl << param << ".Add(" << helperName << '.' << func << '(' << stream << "))";
        }
        if(!streamingAPI && type->isVariableLength())
        {
            if(!SequencePtr::dynamicCast(type))
            {
                out << nl << stream << ".checkSeq()";
            }
            out << nl << stream << ".endElement()";
        }
        out.dec();
        out << nl << "Next";
        if(!streamingAPI && type->isVariableLength())
        {
            out << nl << stream << ".endSeq(szx__)";
        }
        out.dec();
        out << nl << "Next";
    }

    return;
}

string
Slice::VbGenerator::toArrayAlloc(const string& decl, const string& sz)
{
    int count = 0;
    string::size_type pos = decl.size();
    while(pos > 1 && decl.substr(pos - 2, 2) == "()")
    {
        ++count;
        pos -= 2;
    }
    assert(count > 0);

    ostringstream o;
    o << decl.substr(0, pos) << '(' << sz << ')' << decl.substr(pos + 2);
    return o.str();
}

void
Slice::VbGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, false);
}

Slice::VbGenerator::MetaDataVisitor::MetaDataVisitor()
    : _globalMetaDataDone(false)
{
}

bool
Slice::VbGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
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
        static const string prefix = "vb:";
        for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
        {
            string s = *q;
            if(_history.count(s) == 0)
            {
                if(s.find(prefix) == 0)
                {
                    static const string attributePrefix = "vb:attribute:";
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
Slice::VbGenerator::MetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::VbGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    validate(p);
}

bool
Slice::VbGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    validate(p);
    return true;
}

void
Slice::VbGenerator::MetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::VbGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    validate(p);
    return true;
}

void
Slice::VbGenerator::MetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::VbGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    validate(p);
    return true;
}

void
Slice::VbGenerator::MetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::VbGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitParamDecl(const ParamDeclPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    validate(p);
}

void
Slice::VbGenerator::MetaDataVisitor::validate(const ContainedPtr& cont)
{
    DefinitionContextPtr dc = cont->definitionContext();
    assert(dc);
    string file = dc->filename();

    StringList localMetaData = cont->getMetaData();

    StringList::const_iterator p;

    for(p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        string s = *p;

        if(s.find("vb:") == 0) // TODO: remove this statement once "vb:" is a hard error.
        {
            if(SequencePtr::dynamicCast(cont))
            {
                if(s.substr(3) == "collection")
                {
                    cout << file << ":" << cont->line() << ": warning: `vb:' metadata prefix is deprecated; "
                         << "use `clr:' instead" << endl;
                    cont->addMetaData("clr:collection");
                }
            }
            else if(StructPtr::dynamicCast(cont))
            {
                if(s.substr(3) == "class")
                {
                    cout << file << ":" << cont->line() << ": warning: `vb:' metadata prefix is deprecated; "
                         << "use `clr:' instead" << endl;
                    cont->addMetaData("clr:class");
                }
            }
            else if(s.find("vb:attribute:") == 0)
            {
                ; // Do nothing, "vb:attribute:" is OK
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

        prefix = "vb:";
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
