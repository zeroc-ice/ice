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
	"ToString", "Values",
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
					      int& iter)
{
    string stream = marshal ? "__os" : "__is";

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
                    out << nl << param << " = " << stream << ".readByte();";
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
                    out << nl << param << " = " << stream << ".readBool();";
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
                    out << nl << param << " = " << stream << ".readShort();";
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
                    out << nl << param << " = " << stream << ".readInt();";
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
                    out << nl << param << " = " << stream << ".readLong();";
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
                    out << nl << param << " = " << stream << ".readFloat();";
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
                    out << nl << param << " = " << stream << ".readDouble();";
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
                    out << nl << param << " = " << stream << ".readString();";
                }
                break;
            }
#if 0
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << param << ");";
                }
                else
                {
		    if(holder)
		    {
			out << nl << stream << ".readObject(" << param << ".getPatcher());";
		    }
		    else
		    {
                        if(patchParams.empty())
                        {
                            out << nl << stream << ".readObject(new Patcher());";
                        }
                        else
                        {
                            out << nl << stream << ".readObject(" << patchParams << ");";
                        }
		    }
                }
                break;
            }
#endif
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy();";
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
            out << nl << param << " = " << typeS << "Helper.__read(" << stream << ");";
        }
        return;
    }

#if 0
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << param << ");";
        }
        else
        {
            string typeS = typeToString(type);
	    if(holder)
	    {
		out << nl << stream << ".readObject(" << param << ".getPatcher());";
	    }
	    else
	    {
                if(patchParams.empty())
                {
                    out << nl << stream << ".readObject(new Patcher());";
                }
                else
                {
                    out << nl << stream << ".readObject(" << patchParams << ");";
                }
	    }
        }
        return;
    }
#endif

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
	    out << nl << param << " = " << cast << stream << "." << func << "();";
	}
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        // writeSequenceMarshalUnmarshalCode(out, scope, seq, v, marshal, iter, true, metaData); TODO
	if(!marshal)
	{
	    out << nl << param << " = null;";
	}
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
        out << nl << param << " = " << typeS << "Helper.read(" << stream << ");";
    }
}
#if 0

void
Slice::CsGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& scope,
                                                const TypePtr& type,
                                                const string& param,
                                                bool marshal,
                                                int& iter,
                                                bool holder,
                                                const list<string>& metaData,
						const string& patchParams)
{
    string stream = marshal ? "__os" : "__is";
    string v;
    if(holder)
    {
        v = param + ".value";
    }
    else
    {
        v = param;
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
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << v << ");";
                }
                else
                {
		    if(holder)
		    {
			out << nl << stream << ".readObject(" << param << ".getPatcher());";
		    }
		    else
		    {
                        if(patchParams.empty())
                        {
                            out << nl << stream << ".readObject(new Patcher());";
                        }
                        else
                        {
                            out << nl << stream << ".readObject(" << patchParams << ");";
                        }
		    }
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy();";
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
        string typeS = typeToString(type, TypeModeIn, scope);
        if(marshal)
        {
            out << nl << typeS << "Helper.__write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.__read(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << v << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
	    if(holder)
	    {
		out << nl << stream << ".readObject(" << param << ".getPatcher());";
	    }
	    else
	    {
                if(patchParams.empty())
                {
                    out << nl << stream << ".readObject(new Patcher());";
                }
                else
                {
                    out << nl << stream << ".readObject(" << patchParams << ");";
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
            out << nl << v << ".__write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            out << nl << v << " = new " << typeS << "();";
            out << nl << v << ".__read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << v << ".__write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            out << nl << v << " = " << typeS << ".__read(" << stream << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, scope, seq, v, marshal, iter, true, metaData);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
    if(marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
    }
}

void
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                        const string& scope,
                                                        const SequencePtr& seq,
                                                        const string& param,
                                                        bool marshal,
                                                        int& iter,
                                                        bool useHelper,
                                                        const list<string>& metaData)
{
    string stream = marshal ? "__os" : "__is";
    string v = param;

    //
    // Check for metadata that overrides the default sequence
    // mapping. If no metadata is found, we use a regular Java
    // array. If metadata is found, the value of the metadata
    // must be the name of a class which implements the
    // java.util.List interface.
    //
    // There are two sources of metadata - that passed into
    // this function (which most likely comes from a data
    // member definition), and that associated with the type
    // itself. If data member metadata is found, and does
    // not match the type's metadata, then we cannot use
    // the type's Helper class for marshalling - we must
    // generate marshalling code inline.
    //
    string listType = findMetaData(metaData);
    list<string> typeMetaData = seq->getMetaData();
    if(listType.empty())
    {
        listType = findMetaData(typeMetaData);
    }
    else
    {
        string s = findMetaData(typeMetaData);
        if(listType != s)
        {
            useHelper = false;
        }
    }

    //
    // If we can use the sequence's helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(seq->scoped(), scope);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    //
    // Determine sequence depth
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has metadata.
        //
        string m = findMetaData(s->getMetaData());
        if(!m.empty())
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, scope);

    if(!listType.empty())
    {
        //
        // Marshal/unmarshal a custom sequence type
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                string it = o.str();
                iter++;
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;

                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte __elem = (java.lang.Byte)" << it << ".next();";
                        out << nl << stream << ".writeByte(__elem.byteValue());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean __elem = (java.lang.Boolean)" << it << ".next();";
                        out << nl << stream << ".writeBool(__elem.booleanValue());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short __elem = (java.lang.Short)" << it << ".next();";
                        out << nl << stream << ".writeShort(__elem.shortValue());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer __elem = (java.lang.Integer)" << it << ".next();";
                        out << nl << stream << ".writeInt(__elem.intValue());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long __elem = (java.lang.Long)" << it << ".next();";
                        out << nl << stream << ".writeLong(__elem.longValue());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float __elem = (java.lang.Float)" << it << ".next();";
                        out << nl << stream << ".writeFloat(__elem.floatValue());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double __elem = (java.lang.Double)" << it << ".next();";
                        out << nl << stream << ".writeDouble(__elem.doubleValue());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String __elem = (java.lang.String)" << it << ".next();";
                        out << nl << stream << ".writeString(__elem);";
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
                out << eb; // while
                out << eb;
            }
            else
            {
                out << nl << v << " = new " << listType << "();";
                ostringstream o;
                o << origContentS << "[]";
                int d = depth;
                while(d--)
                {
                    o << "[]";
                }
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Byte(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readBoolSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter
                            << "] ? java.lang.Boolean.TRUE : java.lang.Boolean.FALSE);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readShortSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Short(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readIntSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Integer(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readLongSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Long(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readFloatSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Float(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readDoubleSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Double(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readStringSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter << "]);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
        }
        else
        {
            string typeS = getAbsolute(seq->scoped(), scope);
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                iter++;
                string it = o.str();
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;
                out << nl << origContentS << " __elem = (" << origContentS << ")" << it << ".next();";
                writeMarshalUnmarshalCode(out, scope, seq->type(), "__elem", true, iter, false);
                out << eb; // while
                out << eb; // else
            }
            else
            {
                bool isObject = false;
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(origContent);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(origContent))
                {
                    isObject = true;
                }
                out << nl << v << " = new " << listType << "();";
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(isObject)
                {
                    if(builtin)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                    }
                }
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
		if(isObject)
		{
                    //
                    // Add a null value to the list as a placeholder for the element.
                    //
		    out << nl << v << ".add(null);";
		    ostringstream patchParams;
		    patchParams << "new IceInternal.ListPatcher(" << v << ", " << origContentS << ".class, __type"
                                << iter << ", __i" << iter << ')';
		    writeMarshalUnmarshalCode(out, scope, seq->type(), "__elem", false, iter, false,
			                      list<string>(), patchParams.str());
		}
		else
		{
		    out << nl << origContentS << " __elem;";
		    writeMarshalUnmarshalCode(out, scope, seq->type(), "__elem", false, iter, false);
		}
                iter++;
		if(!isObject)
		{
		    out << nl << v << ".add(__elem);";
		}
                out << eb;
            }
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteSeq();";
                    }
                    break;
                }
                case Builtin::KindBool:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeBoolSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBoolSeq();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortSeq();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntSeq();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongSeq();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatSeq();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleSeq();";
                    }
                    break;
                }
                case Builtin::KindString:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeStringSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readStringSeq();";
                    }
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".length);";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, scope, seq->type(), o.str(), true, iter, false);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
		if((b && b->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(origContent))
                {
                    isObject = true;
                }
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(isObject)
                {
                    if(b)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                    }
                }
                out << nl << v << " = new " << origContentS << "[__len" << iter << "]";
                int d = depth;
                while(d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
		ostringstream patchParams;
                if(isObject)
                {
                    patchParams << "new IceInternal.SequencePatcher(" << v << ", " << origContentS
                                << ".class, __type" << iter << ", __i" << iter << ')';
                    writeMarshalUnmarshalCode(out, scope, seq->type(), o.str(), false, iter, false,
                                              list<string>(), patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, scope, seq->type(), o.str(), false, iter, false);
                }
                iter++;
                out << eb;
            }
        }
    }
}

string
Slice::CsGenerator::findMetaData(const list<string>& metaData)
{
    static const string prefix = "cs:";
    for(list<string>::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if((*q).find(prefix) == 0)
        {
            return (*q).substr(prefix.size());
        }
    }

    return "";
}
#endif
