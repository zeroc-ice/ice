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

#include <Slice/CsUtil.h>
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
lookupKwd(const string& name)
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
			       name);
    if(found)
    {
        return "@" + name;
    }

    static const string memberList[] =
    {
	"Add", "Clear", "Clone", "CompareTo", "Contains", "CopyTo", "Count", "Dictionary", "Equals", "Finalize",
	"GetBaseException", "GetEnumerator", "GetHashCode", "GetObjectData", "GetType", "HResult", "HelpLink",
	"IndexOf", "InnerException", "InnerHashtable", "InnerList", "Insert", "IsFixedSize", "IsReadOnly",
	"IsSynchronized", "Item", "Keys", "List", "MemberWiseClone", "Message", "OnClear", "OnClearComplete",
	"OnGet", "OnInsert", "OnInsertComplete", "OnRemove", "OnRemoveComplete", "OnSet", "OnSetComplete",
	"OnValidate", "ReferenceEquals", "Remove", "RemoveAt", "Source", "StackTrace", "SyncRoot", "TargetSite",
	"ToString", "Values", "checked_cast", "unchecked_cast"
    };
    found = binary_search(&memberList[0],
                           &memberList[sizeof(memberList) / sizeof(*memberList)],
			   name);
    return found ? "_cs_" + name : name;
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
// otherwise, return the name unchanged.
//
string
Slice::CsGenerator::fixId(const string& name)
{
    if(name.empty())
    {
	return name;
    }
    if(name[0] != ':')
    {
	return lookupKwd(name);
    }
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
	if(i != ids.begin())
	{
	    result << '.';
	}
	result << *i;
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
					      bool isSeq,
					      bool isOutParam,
					      const string& patchParams)
{
    string stream = marshal ? "__os" : "__is";

    string startAssign;
    string endAssign;
    if(isSeq)
    {
        startAssign = ".Add(";
        endAssign = ")";
    }
    else
    {
	startAssign = " = ";
	endAssign = "";
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
                    out << nl << param << startAssign << stream << ".readByte()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readBool()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readShort()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readInt()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readLong()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readFloat()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readDouble()" << endAssign << ";";
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
                    out << nl << param << startAssign << stream << ".readString()" << endAssign << ";";
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
			    << "_PP = new IceInternal.ParamPatcher(typeof(Ice.Object));";
			out << nl << stream << ".readObject(" << param << "_PP);";
		    }
		    else
		    {
			out << nl << stream << ".readObject(new __Patcher(" << patchParams << "));";
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
                    out << nl << param << startAssign << stream << ".readProxy()" << endAssign << ";";
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
            out << nl << typeS << "Helper.__write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << startAssign << typeS << "Helper.__read(" << stream << ")" << endAssign << ";";
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
		out << nl << "IceInternal.ParamPatcher " << param
		    << "_PP = new IceInternal.ParamPatcher(typeof(" << typeToString(type) << "));";
		out << nl << stream << ".readObject(" << param << "_PP);";
	    }
	    else
	    {
		out << nl << stream << ".readObject(new __Patcher(" << patchParams << "));";
	    }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << param << ".__write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type);
            out << nl << param << " = new " << typeS << "();";
            out << nl << param << ".__read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	string func;
	string cast;
	int sz = en->getEnumerators().size();
	if(sz <= 0x7f)
	{
	    func = marshal ? "writeByte" : "readByte";
	    cast = marshal ? "(byte)" : "(" + fixId(en->scoped()) + ")";
	}
	else if(sz <= 0x7fff)
	{
	    func = marshal ? "writeShort" : "readShort";
	    cast = marshal ? "(short)" : "(" + fixId(en->scoped()) + ")";
	}
	else
	{
	    func = marshal ? "writeInt" : "readInt";
	    cast = marshal ? "(int)" : "(" + fixId(en->scoped()) + ")";
	}
	if(marshal)
	{
	    out << nl << stream << "." << func << "(" << cast << param << ");";
	}
	else
	{
	    out << nl << param << startAssign << cast << stream << "." << func << "()" << endAssign << ";";
	}
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, param, marshal, isSeq);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = typeToString(type);
    if(marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << param << ");";
    }
    else
    {
        out << nl << param << startAssign << typeS << "Helper.read(" << stream << ")" << endAssign << ";";
    }
}

void
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                        const SequencePtr& seq,
                                                        const string& param,
                                                        bool marshal,
							bool isSeq)
{
    string stream = marshal ? "__os" : "__is";

    string startAssign;
    string endAssign;
    if(isSeq)
    {
        startAssign = ".Add(";
	endAssign = ")";
    }
    else
    {
        startAssign = " = ";
	endAssign = "";
    }

    TypePtr type = seq->type();
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
		    out << nl << stream << ".writeSize(" << param << ".Count);";
		    out << nl << "for(int __i = 0; __i < " << param << ".Count; ++__i)";
		    out << sb;
		    string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
		    out << nl << stream << "." << func << "(" << param << "[__i]);";
		    out << eb;
		    out << eb;
		}
		else
		{
		   if(builtin->kind() == Builtin::KindObject)
		   {
		       out << nl << param << " = new Ice.ObjectSeq();";
		       out << nl << "int __len = " << stream << ".readSize();";
		       out << nl << "for(int __i = 0; __i < __len; ++__i)";
		       out << sb;
		       out << nl << stream << ".readObject(new IceInternal.SequencePatcher("
		           << param << ", typeof(Ice.Object), __i));";
		       out << eb;
		   }
		   else
		   {
		       out << nl << param << " = new Ice.ObjectProxySeq();";
		       out << nl << "int __len = " << stream << ".readSize();";
		       out << nl << "for(int __i = 0; __i < __len; ++__i)";
		       out << sb;
		       out << nl << param << ".Add(" << stream << ".readProxy());";
		       out << eb;
		   }
		}
	        break;
	    }
	    default:
	    {
		string typeS = typeToString(type);
		typeS[0] = toupper(typeS[0]);
		if(marshal)
		{
		    out << nl << stream << ".write" << typeS << "Seq(" << param << ".ToArray());";
		}
		else
		{
		    out << nl << param << startAssign << "new " << fixId(seq->scoped()) << "(" << stream
		        << ".read" << typeS << "Seq())" << endAssign << ";";
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
	    out << nl << stream << ".writeSize(" << param << ".Count);";
	    out << nl << "for(int __i = 0; __i < " << param << ".Count; ++__i)";
	    out << sb;
            out << nl << stream << ".writeObject(" << param << "[__i]);";
	    out << eb;
        }
        else
        {
	    out << nl << "int sz = " << stream << ".readSize();";
	    out << nl << param << " = new " << fixId(seq->scoped()) << "(sz);";
	    out << nl << "for(int i = 0; i < sz; ++i)";
	    out << sb;
	    out << nl << "IceInternal.SequencePatcher sp = new IceInternal.SequencePatcher("
		<< param << ", " << "typeof(" << typeToString(type) << "), i);";
	    out << nl << stream << ".readObject(sp);";
	    out << eb;
        }
        return;
    }


    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	if(marshal)
	{
	    out << nl << stream << ".writeSize(" << param << ".Count);";
	    out << nl << "for(int __i = 0; __i < " << param << ".Count; ++__i)";
	    out << sb;
	    writeMarshalUnmarshalCode(out, type, param + "[__i]", marshal, true);
	    out << eb;
	}
	else
	{
	    out << nl << param << startAssign << "new " << fixId(seq->scoped()) << "()" << endAssign << ";";
	    out << nl << "int sz = " << stream << ".readSize();";
	    out << nl << "for(int __i = 0; __i < sz; ++__i)";
	    out << sb;
	    writeMarshalUnmarshalCode(out, type, param, marshal, true);
	    out << eb;
	}
        return;
    }

    string typeS = typeToString(type);
    if(marshal)
    {
        string func = ProxyPtr::dynamicCast(type) ? "__write" : "write";
	out << nl << stream << ".writeSize(" << param << ".Count);";
	out << nl << "for(int __i = 0; __i < " << param << ".Count; ++__i)";
	out << sb;
	out << nl << typeS << "Helper." << func << "(" << stream << ", " << param << "[__i]);";
	out << eb;
    }
    else
    {
        string func = ProxyPtr::dynamicCast(type) ? "__read" : "read";
	out << nl << param << startAssign << "new " << fixId(seq->scoped()) << "()" << endAssign << ";";
	out << sb;
	out << nl << "int sz = " << stream << ".readSize();";
	out << nl << "for(int __i = 0; __i < sz; ++__i)";
	out << sb;
	out << nl << param << ".Add(" << typeS << "Helper." << func << "(" << stream << "));";
	out << eb;
	out << eb;
    }

    return;
}
