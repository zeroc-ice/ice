// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
    if(seq && !seq->hasMetaData("cs:collection"))
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
       return !s->hasMetaData("cs:class");
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
        stream = streamingAPI ? "__out" : "__os";
    }
    else
    {
        stream = streamingAPI ? "__in" : "__is";
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
			    << "_PP = new IceInternal.ParamPatcher(typeof(Ice.Object));";
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
			out << "new __Patcher(" << patchParams << "));";
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
            out << nl << typeS << "Helper.";
	    if(!streamingAPI)
	    {
	        out << "__";
	    }
	    out << "write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeS << "Helper.";
	    if(!streamingAPI)
	    {
		out << "__";
	    }
	    out << "read(" << stream << ')' << ';';
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
		out << "new __Patcher(" << patchParams << "));";
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
	size_t sz = en->getEnumerators().size();
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
        stream = streamingAPI ? "__out" : "__os";
    }
    else
    {
        stream = streamingAPI ? "__in" : "__is";
    }
    
    TypePtr type = seq->type();
    string typeS = typeToString(type);

    bool isArray = !seq->hasMetaData("cs:collection");
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
		    out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
		    out << sb;
		    string func = builtin->kind() == Builtin::KindObject ? "writeObject" : "writeProxy";
		    out << nl << stream << '.' << func << '(' << param << "[__ix]);";
		    out << eb;
		    out << eb;
		}
		else
		{
		    out << nl << "int __lenx = " << stream << ".readSize();";
		    if(!streamingAPI)
		    {
			out << nl << stream << ".startSeq(__lenx, "
			    << static_cast<unsigned>(builtin->minWireSize()) << ");";
		    }
		    out << nl << param << " = new ";
		    if(builtin->kind() == Builtin::KindObject)
		    {
			if(isArray)
			{
			    out << "Ice.Object[__lenx];";
			}
			else
			{
			    out << typeToString(seq) << "(__lenx);";
			}
			out << nl << "for(int __ix = 0; __ix < __lenx; ++__ix)";
			out << sb;
			out << nl << stream << ".readObject(";
			if(streamingAPI)
			{
			    out << "(ReadObjectCallback)";
			}
			out << "new IceInternal.SequencePatcher(" << param << ", typeof(Ice.Object), __ix));";
		    }
		    else
		    {
		        if(isArray)
			{
			    out << "Ice.ObjectPrx[__lenx];";
			}
			else
			{
			    out << typeToString(seq) << "(__lenx);";
			}
			out << nl << "for(int __ix = 0; __ix < __lenx; ++__ix)";
			out << sb;
			out << nl << param << "[__ix] = " << stream << ".readProxy();";
		    }
		    if(!streamingAPI)
		    {
			out << nl << stream << ".checkSeq();";
			out << nl << stream << ".endElement();";
		    }
		    out << eb;
		    if(!streamingAPI)
		    {
			out << nl << stream << ".endSeq(__lenx);";
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
		        out << ".ToArray()";
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
	    out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
	    out << sb;
            out << nl << stream << ".writeObject(" << param << "[__ix]);";
	    out << eb;
	    out << eb;
        }
        else
        {
	    out << sb;
	    out << nl << "int szx = " << stream << ".readSize();";
	    if(!streamingAPI)
	    {
		out << nl << stream << ".startSeq(szx, " << static_cast<unsigned>(type->minWireSize()) << ");";
	    }
	    out << nl << param << " = new ";
	    if(isArray)
	    {
		out << toArrayAlloc(typeS + "[]", "szx");
	    }
	    else
	    {
	        out << fixId(seq->scoped()) << "(szx)";
	    }
	    out << ';';
	    out << nl << "for(int i = 0; i < szx; ++i)";
	    out << sb;
	    out << nl << "IceInternal.SequencePatcher spx = new IceInternal.SequencePatcher("
		<< param << ", " << "typeof(" << typeS << "), i);";
	    out << nl << stream << ".readObject(";
	    if(streamingAPI)
	    {
	        out << "(Ice.ReadObjectCallback)";
	    }
	    out << "spx);";
	    if(!streamingAPI)
	    {
		out << nl << stream << ".checkSeq();";
		out << nl << stream << ".endElement();";
	    }
	    out << eb;
	    if(!streamingAPI)
	    {
		out << nl << stream << ".endSeq(szx);";
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
	    out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
	    out << sb;
	    out << nl << param << "[__ix].__write(" << stream << ");";
	    out << eb;
	    out << eb;
	}
	else
	{
	    out << sb;
	    out << nl << "int szx = " << stream << ".readSize();";
	    if(!streamingAPI)
	    {
		out << nl << stream << ".startSeq(szx, " << static_cast<unsigned>(type->minWireSize()) << ");";
	    }
	    out << nl << param << " = new ";
	    if(isArray)
	    {
	        out << toArrayAlloc(typeS + "[]", "szx");
	    }
	    else
	    {
		out << fixId(seq->scoped()) << "(szx)";
	    }
	    out << ';';
	    out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
	    out << sb;
	    out << nl << param << "[__ix].__read(" << stream << ");";
	    if(st->isVariableLength() && !streamingAPI)
	    {
		out << nl << stream << ".checkSeq();";
		out << nl << stream << ".endElement();";
	    }
	    out << eb;
	    if(!streamingAPI)
	    {
		out << nl << stream << ".endSeq(szx);";
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
	    out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
	    out << sb;
	    out << nl << stream << ".writeByte((byte)" << param << "[__ix]);";
	    out << eb;
	    out << eb;
	}
	else
	{
	    out << sb;
	    out << nl << "int szx = " << stream << ".readSize();";
	    if(!streamingAPI)
	    {
		out << nl << stream << ".startSeq(szx, " << static_cast<unsigned>(type->minWireSize()) << ");";
	    }
	    out << nl << param << " = new ";
	    if(isArray)
	    {
	        out << toArrayAlloc(typeS + "[]", "szx");
	    }
	    else
	    {
	        out << fixId(seq->scoped()) << "(szx)";
	    }
	    out << ';';
	    out << nl << "for(int __ix = 0; __ix < szx; ++__ix)";
	    out << sb;
	    if(isArray)
	    {
		out << nl << param << "[__ix] = (" << typeS << ')' << stream << ".readByte();";
	    }
	    else
	    {
	        out << nl << param << ".Add((" << typeS << ')' << stream << ".readByte());";
	    }
	    out << eb;
	    if(!streamingAPI)
	    {
		out << nl << stream << ".endSeq(szx);";
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
    if(!streamingAPI && ProxyPtr::dynamicCast(type))
    {
       func = "__";
    }
    if(marshal)
    {
        func += "write";
	out << nl << "if(" << param << " == null)";
	out << sb;
	out << nl << stream << ".writeSize(0);";
	out << eb;
	out << nl << "else";
	out << sb;
	out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
	out << nl << "for(int __ix = 0; __ix < " << param << '.' << limitID << "; ++__ix)";
	out << sb;
	out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[__ix]);";
	out << eb;
	out << eb;
    }
    else
    {
        func += "read";
	out << sb;
	out << nl << "int szx = " << stream << ".readSize();";
	if(!streamingAPI)
	{
	    out << nl << stream << ".startSeq(szx, " << static_cast<unsigned>(type->minWireSize()) << ");";
	}
	out << nl << param << " = new ";
	if(isArray)
	{
	    out << toArrayAlloc(typeS + "[]", "szx");
	}
	else
	{
	    out << fixId(seq->scoped()) << "(szx)";
	}
	out << ';';
	out << nl << "for(int __ix = 0; __ix < szx; ++__ix)";
	out << sb;
	if(isArray)
	{
	    out << nl << param << "[__ix] = " << helperName << '.' << func << '(' << stream << ");";
	}
	else
	{
	    out << nl << param << ".Add(" << helperName << '.' << func << '(' << stream << "));";
	}
	if(type->isVariableLength())
	{
	    if(!SequencePtr::dynamicCast(type) && !streamingAPI)
	    {
		out << nl << stream << ".checkSeq();";
	    }
	    if(!streamingAPI)
	    {
		out << nl << stream << ".endElement();";
	    }
	}
	out << eb;
	if(!streamingAPI)
	{
	    out << nl << stream << ".endSeq(szx);";
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
Slice::CsGenerator::validateMetaData(const UnitPtr& unit)
{
    MetaDataVisitor visitor;
    unit->visit(&visitor, false);
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
    StringList globalMetaData = dc->getMetaData();
    string file = dc->filename();

    StringList localMetaData = cont->getMetaData();

    StringList::const_iterator p;
    static const string prefix = "cs:";

    for(p = globalMetaData.begin(); p != globalMetaData.end(); ++p)
    {
        string s = *p;
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
		cout << file << ": warning: ignoring invalid global metadata `" << s << "'" << endl;
            }
            _history.insert(s);
        }
    }

    for(p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
	string s = *p;
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
		}
		cout << file << ":" << cont->line() << ": warning: ignoring invalid metadata `" << s << "'" << endl;
            }
            _history.insert(s);
        }
    }
}
