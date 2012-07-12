// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/CPlusPlusUtil.h>
#include <Slice/Util.h>
#include <cstring>
#include <functional>

#ifndef _WIN32
#  include <fcntl.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

Slice::FeatureProfile Slice::featureProfile = Slice::Ice;

char
Slice::ToIfdef::operator()(char c)
{
    if(!isalnum(static_cast<unsigned char>(c)))
    {
        return '_';
    }
    else
    {
        return c;
    }
}


void
Slice::printHeader(Output& out)
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    out << header;
    out << "//\n";
    out << "// Ice version " << ICE_STRING_VERSION << "\n";
    out << "//\n";
}

void
Slice::printVersionCheck(Output& out)
{
    out << "\n";
    out << "\n#ifndef ICE_IGNORE_VERSION";
    int iceVersion = ICE_INT_VERSION; // Use this to prevent warning with C++Builder
    if(iceVersion % 100 > 50)
    {
        //
        // Beta version: exact match required
        //
        out << "\n#   if ICE_INT_VERSION  != " << ICE_INT_VERSION;
        out << "\n#       error Ice version mismatch: an exact match is required for beta generated code";
        out << "\n#   endif";
    }
    else
    {
        out << "\n#   if ICE_INT_VERSION / 100 != " << ICE_INT_VERSION / 100;
        out << "\n#       error Ice version mismatch!";
        out << "\n#   endif";
        
        //
        // Generated code is release; reject beta header
        //
        out << "\n#   if ICE_INT_VERSION % 100 > 50";
        out << "\n#       error Beta header file detected";
        out << "\n#   endif";
        
        out << "\n#   if ICE_INT_VERSION % 100 < " << ICE_INT_VERSION % 100;
        out << "\n#       error Ice patch level mismatch!";
        out << "\n#   endif";
    }
    out << "\n#endif";
}

void
Slice::printDllExportStuff(Output& out, const string& dllExport)
{
    if(dllExport.size())
    {
        out << sp;
        out << "\n#ifndef " << dllExport;
        out << "\n#   ifdef " << dllExport << "_EXPORTS";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_EXPORT";
        out << "\n#   else";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_IMPORT";
        out << "\n#   endif";
        out << "\n#endif";
    }
}

namespace
{

string
sequenceTypeToString(const SequencePtr& seq, const StringList& metaData, int typeCtx)
{
    string seqType = findMetaData(metaData, typeCtx);
    if(!seqType.empty())
    {
        if(seqType == "array" || seqType == "range:array")
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
            if(typeCtx & TypeContextAMIPrivateEnd)
            {
                if(!builtin || builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy)
                {
                    seqType = findMetaData(seq->getMetaData());
                    if(seqType.empty())
                    {
                        return typeToString(seq);
                    }
                    else
                    {
                        return "::std::vector< " + typeToString(seq->type()) + ">";
                    }
                }
                else if(builtin->kind() == Builtin::KindByte)
                {
                    string s = typeToString(seq->type());
                    return "::std::pair<const " + s + "*, const " + s + "*>";
                }
                else if(builtin->kind() != Builtin::KindString &&
                        builtin->kind() != Builtin::KindObject &&
                        builtin->kind() != Builtin::KindObjectProxy)
                {
                    string s = typeToString(builtin);
                    if(s[0] == ':')
                    {
                        s = " " + s;
                    }
                    return "::std::pair< ::IceUtil::ScopedArray<" + s + ">," +
                        " ::std::pair<const " + s + "*, const " + s + "*> >";
                }
                else
                {
                    return "::std::vector< " + typeToString(seq->type(), 
                                                            seq->typeMetaData(), 
                                                            inWstringModule(seq) ? TypeContextUseWstring : 0) + ">";
                }
            }
            string s = typeToString(seq->type(), seq->typeMetaData(), inWstringModule(seq) ? TypeContextUseWstring : 0);
            return "::std::pair<const " + s + "*, const " + s + "*>";
        }
        else if(seqType.find("range") == 0)
        {
            string s;
            if(seqType.find("range:") == 0)
            {
                s = seqType.substr(strlen("range:"));
            }
            else
            {
                s = fixKwd(seq->scoped());
            }
            if(typeCtx & TypeContextAMIPrivateEnd)
            {
                return s;
            }
            if(s[0] == ':')
            {
                s = " " + s;
            }
            return "::std::pair<" + s + "::const_iterator, " + s + "::const_iterator>";
        }
        else
        {
            return seqType;
        }
    }
    else
    {
        // Get the metadata associated at the point of definition.
        bool protobuf;
        seqType = findMetaData(seq, seq->getMetaData(), protobuf, typeCtx);
        if(protobuf && !seqType.empty())
        {
            return seqType;
        }
        return fixKwd(seq->scoped());
    }
}

}

string
Slice::typeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    static const char* builtinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "::Ice::Short",
        "::Ice::Int",
        "::Ice::Long",
        "::Ice::Float",
        "::Ice::Double",
        "::std::string",
        "::Ice::ObjectPtr",
        "::Ice::ObjectPrx",
        "::Ice::LocalObjectPtr"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            string strType = findMetaData(metaData);
            if(strType != "string" && (typeCtx & TypeContextUseWstring || strType == "wstring"))
            {
                if(featureProfile == IceE)
                {
                    return "::Ice::Wstring";
                }
                else
                {
                    return "::std::wstring";
                }
            }
        }
        return builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return fixKwd(cl->scoped() + "Ptr");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "class")
        {
            return fixKwd(st->scoped() + "Ptr");
        }
        return fixKwd(st->scoped());
    }
            
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixKwd(proxy->_class()->scoped() + "Prx");
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return sequenceTypeToString(seq, metaData, typeCtx);
    }
            
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixKwd(contained->scoped());
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixKwd(en->scoped());
    }
            
    return "???";
}

string
Slice::returnTypeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    if(!type)
    {
        return "void";
    }

    return typeToString(type, metaData, typeCtx);
}

string
Slice::inputTypeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    static const char* inputBuiltinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "::Ice::Short",
        "::Ice::Int",
        "::Ice::Long",
        "::Ice::Float",
        "::Ice::Double",
        "const ::std::string&",
        "const ::Ice::ObjectPtr&",
        "const ::Ice::ObjectPrx&",
        "const ::Ice::LocalObjectPtr&"
    };

    typeCtx |= TypeContextInParam;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            string strType = findMetaData(metaData);
            if(strType != "string" && (typeCtx & TypeContextUseWstring || strType == "wstring"))
            {
                if(featureProfile == IceE)
                {
                    return "const ::Ice::Wstring&";
                }
                else
                {
                    return "const ::std::wstring&";
                }
            }
        }
        return inputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return "const " + fixKwd(cl->scoped() + "Ptr&");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "class")
        {
            return "const " + fixKwd(st->scoped() + "Ptr&");
        }
        return "const " + fixKwd(st->scoped()) + "&";
    }
            
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return "const " + fixKwd(proxy->_class()->scoped() + "Prx&");
    }
            
    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixKwd(en->scoped());
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return "const " + sequenceTypeToString(seq, metaData, typeCtx) + "&";
    }
            
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return "const " + fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

string
Slice::outputTypeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    static const char* outputBuiltinTable[] =
    {
        "::Ice::Byte&",
        "bool&",
        "::Ice::Short&",
        "::Ice::Int&",
        "::Ice::Long&",
        "::Ice::Float&",
        "::Ice::Double&",
        "::std::string&",
        "::Ice::ObjectPtr&",
        "::Ice::ObjectPrx&",
        "::Ice::LocalObjectPtr&"
    };
    
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            string strType = findMetaData(metaData);
            if(strType != "string" && (typeCtx & TypeContextUseWstring || strType == "wstring"))
            {
                if(featureProfile == IceE)
                {
                    return "::Ice::Wstring&";
                }
                else
                {
                    return "::std::wstring&";
                }
            }
        }
        return outputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return fixKwd(cl->scoped() + "Ptr&");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "class")
        {
            return fixKwd(st->scoped() + "Ptr&");
        }
        return fixKwd(st->scoped()) + "&";
    }
            
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixKwd(proxy->_class()->scoped() + "Prx&");
    }
            
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return sequenceTypeToString(seq, metaData, typeCtx) + "&";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

string
Slice::operationModeToString(Operation::Mode mode)
{
    switch(mode)
    {
        case Operation::Normal:
        {
            return "::Ice::Normal";
        }
            
        case Operation::Nonmutating:
        {
            return "::Ice::Nonmutating";
        }

        case Operation::Idempotent:
        {
            return "::Ice::Idempotent";
        }

        default:
        {
            assert(false);
        }
    }
    
    return "???"; 
}

//
// If the passed name is a keyword, return the name with a "_cxx_" prefix;
// otherwise, return the name unchanged.
//

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    // Note that this keyword list unnecessarily contains C++ keywords
    // that are illegal Slice identifiers -- namely identifiers that
    // are Slice keywords (class, int, etc.). They have not been removed
    // so that the keyword list is kept complete.
    //
    static const string keywordList[] = 
    {       
        "and", "and_eq", "asm", "auto", "bit_and", "bit_or", "bool", "break", "case", "catch", "char",
        "class", "compl", "const", "const_cast", "continue", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for",
        "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "not", "not_eq",
        "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
        "return", "short", "signed", "sizeof", "static", "static_cast", "struct", "switch", "template",
        "this", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_cpp_" + name : name;
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
// but with all components that are C++ keywords replaced by
// their "_cxx_"-prefixed version; otherwise, if the passed name is
// not scoped, but a C++ keyword, return the "_cxx_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::fixKwd(const string& name)
{
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& fixedParam, bool marshal,
                                 const string& str, bool pointer, const StringList& metaData, int typeCtx)
{
    string stream;
    if(str.empty())
    {
        stream = marshal ? "__os" : "__is";
    }
    else
    {
        stream = str;
    }
    
    string deref;
    if(pointer)
    {
        deref = "->";
    }
    else
    {
        deref = '.';
    }
    
    string func = marshal ? "write(" : "read(";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObject)
        {
            if(marshal)
            {
                out << nl << stream << deref << func << fixedParam << ");";
            }
            else
            {
                out << nl << stream << deref << func << "::Ice::__patch__ObjectPtr, &" << fixedParam << ");";
            }
            return;
        }
        else
        {
            out << nl << stream << deref << func << fixedParam << ");";
            return;
        }
    }
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        string scope = fixKwd(cl->scope());
        if(marshal)
        {
            out << nl << stream << deref << "write(::Ice::ObjectPtr(" << scope << "upCast(" << fixedParam 
                << ".get())));";
        }
        else
        {
            out << nl << stream << deref << "read("
                << scope << "__patch__" << cl->name() << "Ptr, &" << fixedParam << ");";
        }

        return;
    }

    ProxyPtr px = ProxyPtr::dynamicCast(type);
    if(px)
    {
        string scope = fixKwd(px->_class()->scope());
        if(marshal)
        {
            out << nl << stream << deref << "write(::Ice::ObjectPrx(::IceProxy" << scope << "upCast(" << fixedParam 
                << ".get())));";
        }
        else
        {
            out << nl << scope << "__read(" << (pointer ? "" : "&") << stream << ", " << fixedParam << ");";
        }
        return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        string deref;
        if(findMetaData(st->getMetaData()) == "class")
        {
            if(!marshal)
            {
                out << nl << fixedParam << " = new " << fixKwd(st->scoped()) << ";";
            }
            out << nl << fixedParam << "->__" << func << (pointer ? "" : "&") << stream << ");";
        }
        else
        {
            out << nl << fixedParam << ".__" << func << (pointer ? "" : "&") << stream << ");";
        }
        return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        builtin = BuiltinPtr::dynamicCast(seq->type());
        string funcSeq = (marshal ? "write" : "read") + fixKwd(seq->name()) + "(";

        if(marshal)
        {
            string seqType = findMetaData(metaData, typeCtx);
            string scope = fixKwd(seq->scope());

            if(seqType == "array" || seqType == "range:array")
            {
                //
                // Use array (pair<const TYPE*, const TYPE*>). In parameters only.
                //
                if(!builtin || builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy)
                {
                    //
                    // Sequence type in not handled by BasicStream functions. If the sequence is the
                    // default vector than we can use the sequences generated write function. Otherwise
                    // we need to generate marshal code to write each element.
                    //
                    seqType = findMetaData(seq->getMetaData());
                    if(seqType.empty())
                    {
                        out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", "
                            << fixedParam << ".first, " << fixedParam << ".second);";
                    }
                    else
                    {
                        out << nl << "::Ice::Int __sz_" << fixedParam << " = static_cast< ::Ice::Int>(" << fixedParam
                            << ".second - " << fixedParam << ".first);";
                        out << nl << stream << deref << "writeSize(__sz_" << fixedParam << ");";
                        out << nl << "for(int __i_" << fixedParam << " = 0; __i_" << fixedParam << " < __sz_" 
                            << fixedParam << "; ++__i_" << fixedParam << ")";
                        out << sb;
                        writeMarshalUnmarshalCode(out, seq->type(), fixedParam + ".first[__i_" + fixedParam + "]",
                                                  true);
                        out << eb;
                    }
                }
                else
                {
                    //
                    // Use BasicStream write functions.
                    //
                    out << nl << stream << deref << func << fixedParam << ".first, " << fixedParam << ".second);";
                }
            }
            else if(seqType.find("range") == 0)
            {
                //
                // Use range (pair<TYPE::const_iterator, TYPE::const_iterator). Only for in paramaters.
                // Need to check if the range defines an iterator type other than the actual sequence
                // type.
                //
                StringList l;
                if(seqType.find("range:") == 0)
                {
                    seqType = seqType.substr(strlen("range:"));
                    l.push_back("cpp:" + seqType);
                }
                else
                {
                    seqType = fixKwd(seq->scoped());
                }
                out << nl << stream << deref << "writeSize(static_cast< ::Ice::Int>(IceUtilInternal::distance(" 
                    << fixedParam << ".first, " << fixedParam << ".second)));"; 
                out << nl << "for(" << seqType << "::const_iterator ___" << fixedParam << " = "
                    << fixedParam << ".first; ___" << fixedParam << " != " << fixedParam << ".second; ++___"
                    << fixedParam << ")";
                out << sb;
                writeMarshalUnmarshalCode(out, seq->type(), "(*___" + fixedParam + ")", true, "", true, l);
                out << eb;
            }
            else if(!seqType.empty())
            {
                out << nl << stream << deref << "writeSize(static_cast< ::Ice::Int>(" << fixedParam << ".size()));";
                out << nl << seqType << "::const_iterator __p_" << fixedParam << ";";
                out << nl << "for(__p_" << fixedParam << " = " << fixedParam << ".begin(); __p_" << fixedParam 
                    << " != " << fixedParam << ".end(); ++__p_" << fixedParam << ")";
                out << sb;
                writeMarshalUnmarshalCode(out, seq->type(), "(*__p_" + fixedParam + ")", true);
                out << eb;
            }
            else
            {
                //
                // No modifying metadata specified. Use appropriate write methods for type.
                //
                bool protobuf;
                seqType = findMetaData(seq, seq->getMetaData(), protobuf);
                if(protobuf || !seqType.empty())
                {
                    out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", " << fixedParam 
                        << ");";
                }
                else if(!builtin || 
                        builtin->kind() == Builtin::KindObject ||
                        builtin->kind() == Builtin::KindObjectProxy)
                {
                    out << nl << "if(" << fixedParam << ".size() == 0)";
                    out << sb;
                    out << nl << stream << deref << "writeSize(0);";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", &"
                        << fixedParam << "[0], &" << fixedParam << "[0] + " << fixedParam << ".size());";
                    out << eb;
                }
                else if(builtin->kind() == Builtin::KindBool)
                {
                    out << nl << stream << deref << func << fixedParam << ");";
                }
                else
                {
                    out << nl << "if(" << fixedParam << ".size() == 0)";
                    out << sb;
                    out << nl << stream << deref << "writeSize(0);";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << stream << deref << func << "&" << fixedParam << "[0], &" << fixedParam 
                        << "[0] + " << fixedParam << ".size());";
                    out << eb;
                }
            }
        }
        else
        {
            string seqType = findMetaData(metaData, typeCtx);
            string scope = fixKwd(seq->scope());
            if(seqType == "array" || seqType == "range:array")
            {
                //
                // Use array (pair<const TYPE*, const TYPE*>). In parameters only.
                //
                if(!builtin || builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy)
                {
                    seqType = findMetaData(seq->getMetaData());
                    if(seqType.empty())
                    {
                        out << nl << typeToString(type) << " ___" << fixedParam << ";";
                        out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", ___"
                            << fixedParam << ");";
                    }
                    else
                    {
                        seqType = "::std::vector< " + typeToString(seq->type()) + ">";
                        StringList l;
                        l.push_back("cpp:type:" + seqType);
                        out << nl << seqType << " ___" << fixedParam << ";";
                        writeMarshalUnmarshalCode(out, seq, "___" + fixedParam, false, "", true, l);
                    }
                }
                else if(builtin->kind() == Builtin::KindByte)
                {
                    out << nl << stream << deref << func << fixedParam << ");";
                }
                else if(builtin->kind() != Builtin::KindString && builtin->kind() != Builtin::KindObject &&
                        builtin->kind() != Builtin::KindObjectProxy)
                {
                    string s = typeToString(builtin);
                    if(s[0] == ':')
                    {
                       s = " " + s;
                    }

                    if(typeCtx & TypeContextAMIPrivateEnd)
                    {
                        out << nl << fixedParam << ".first.reset(" << stream << deref << func << fixedParam
                            << ".second));";
                    }
                    else
                    {
                        out << nl << "::IceUtil::ScopedArray<" << s << "> ___" << fixedParam << '('
                            << stream << deref << func << fixedParam << "));";
                    }
                }
                else
                {
                    out << nl << "::std::vector< " 
                        << typeToString(seq->type(), seq->typeMetaData(), 
                                        inWstringModule(seq) ? TypeContextUseWstring : 0) << "> ___" 
                        << fixedParam << ";";
                    out << nl << stream << deref << func << "___" << fixedParam << ");";
                }

                if(!builtin || builtin->kind() == Builtin::KindString || builtin->kind() == Builtin::KindObject ||
                        builtin->kind() == Builtin::KindObjectProxy)
                {
                    if(typeCtx & TypeContextAMIPrivateEnd)
                    {
                        out << nl << fixedParam << ".swap(" << "___" << fixedParam << ");";
                    }
                    else
                    {
                        out << nl << fixedParam << ".first" << " = &___" << fixedParam << "[0];";
                        out << nl << fixedParam << ".second" << " = " << fixedParam << ".first + " << "___" 
                            << fixedParam << ".size();";
                    }
                }
            }
            else if(seqType.find("range") == 0)
            {
                //
                // Use range (pair<TYPE::const_iterator, TYPE::const_iterator>). In paramters only.
                // Need to check if iterator type other than default is specified.
                //
                StringList md;
                if(seqType.find("range:") == 0)
                {
                    md.push_back("cpp:type:" + seqType.substr(strlen("range:")));
                }
                if(typeCtx & TypeContextAMIPrivateEnd)
                {
                    writeMarshalUnmarshalCode(out, seq, fixedParam, false, "", true, md);
                }
                else
                {
                    writeMarshalUnmarshalCode(out, seq, "___" + fixedParam, false, "", true, md);
                    out << nl << fixedParam << ".first = ___" << fixedParam << ".begin();";
                    out << nl << fixedParam << ".second = ___" << fixedParam << ".end();";
                }
            }
            else if(!seqType.empty())
            {
                //
                // Using alternate sequence type. 
                //
                out << nl << "::Ice::Int __sz_" << fixedParam << ";";
                out << nl << stream << deref << "readAndCheckSeqSize(" << seq->type()->minWireSize() << ", __sz_" 
                    << fixedParam << ");";
                out << nl << seqType << "(__sz_" << fixedParam << ").swap(" << fixedParam << ");";
                out << nl << seqType << "::iterator __p_" << fixedParam << ";";
                out << nl << "for(__p_" << fixedParam << " = " << fixedParam << ".begin(); __p_" << fixedParam 
                   << " != " << fixedParam << ".end(); ++__p_" << fixedParam << ")";
                out << sb;
                writeMarshalUnmarshalCode(out, seq->type(), "(*__p_" + fixedParam + ")", false);
                out << eb;
            }
            else
            {
                //
                // No modifying metadata supplied. Just use
                // appropriate read function.
                //
                bool protobuf;
                seqType = findMetaData(seq, seq->getMetaData(), protobuf);
                if(protobuf || 
                   !seqType.empty() ||
                   !builtin || 
                   builtin->kind() == Builtin::KindObject ||
                   builtin->kind() == Builtin::KindObjectProxy)
                {
                    out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", "
                        << fixedParam << ");";
                }
                else if(builtin->kind() == Builtin::KindByte)
                {
                    StringList md;
                    md.push_back("cpp:array");
                    string tmpParam = "___";

                    //
                    // Catch some case in which it is not possible to just prepend
                    // underscores to param to use as temporary variable.
                    //
                    if(fixedParam.find("(*") == 0)
                    {
                        tmpParam += fixedParam.substr(2, fixedParam.length() - 3);
                    }
                    else if(fixedParam.find("[i]") != string::npos)
                    {
                        tmpParam += fixedParam.substr(0, fixedParam.length() - 3);
                    }
                    else if(fixedParam.find("->second") != string::npos)
                    {
                        tmpParam += fixedParam.substr(0, fixedParam.length() - 8);
                    }
                    else
                    {
                        tmpParam += fixedParam;
                    }
                    out << nl << typeToString(type, md, TypeContextInParam) << " " << tmpParam << ";";
                    out << nl << stream << deref << func << tmpParam << ");";
                    out << nl << "::std::vector< ::Ice::Byte>(" << tmpParam << ".first, " << tmpParam 
                        << ".second).swap(" << fixedParam << ");";
                }
                else
                {
                    out << nl << stream << deref << func << fixedParam << ");";
                }
            }
        }
        return;
    }
    
    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        string scope = fixKwd(dict->scope());
        string funcDict = (marshal ? "write" : "read") + fixKwd(dict->name()) + "(";
        out << nl << scope << "__" << funcDict << (pointer ? "" : "&") << stream << ", " << fixedParam << ");";
        return;
    }
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);

    out << nl << fixKwd(constructed->scope()) << "__" << func << (pointer ? "" : "&") << stream << ", "
        << fixedParam << ");";
}

void
Slice::writeMarshalCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData,
                        int typeCtx)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeMarshalUnmarshalCode(out, (*p)->type(), fixKwd((*p)->name()), true, "", true, (*p)->getMetaData(),
                                  typeCtx);
    }
    if(ret)
    {
        writeMarshalUnmarshalCode(out, ret, "__ret", true, "", true, metaData);
    }
}

void
Slice::writeUnmarshalCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData,
                          int typeCtx)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeMarshalUnmarshalCode(out, (*p)->type(), fixKwd((*p)->name()), false, "", true, (*p)->getMetaData(), 
                                  typeCtx);
    }
    if(ret)
    {
        writeMarshalUnmarshalCode(out, ret, "__ret", false, "", true, metaData, typeCtx);
    }
}

namespace
{

void
writeParamAllocateCode(Output& out, const TypePtr& type, const string& fixedName, const StringList& metaData,
                       int typeCtx)
{
    out << nl << typeToString(type, metaData, typeCtx) << ' ' << fixedName;
    StructPtr st = StructPtr::dynamicCast(type);
    if(st && findMetaData(st->getMetaData()) == "class")
    {
        out << " = new " << fixKwd(st->scoped());
    }
    out << ';';

    if(!(typeCtx & TypeContextInParam))
    {
        return; // We're done.
    }

    //
    // If using a range or array we need to allocate the range container, or 
    // array as well now to ensure they are always in the same scope.
    //
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        bool protobuf;
        string seqType = findMetaData(seq, metaData, protobuf, typeCtx);
        if(!protobuf && seqType.empty())
        {
            seqType = findMetaData(seq, seq->getMetaData(), protobuf, typeCtx);
        }
        if(protobuf)
        {
            return;
        }
        if(seqType == "array" || seqType == "range:array")
        {
            if(typeCtx & TypeContextAMICallPrivateEnd)
            {
                out << nl << typeToString(seq, metaData, TypeContextAMIPrivateEnd) << " ___" << fixedName << ";";
//                 BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
//                 if(builtin && 
//                    builtin->kind() != Builtin::KindByte &&
//                    builtin->kind() != Builtin::KindString &&
//                    builtin->kind() != Builtin::KindObject &&
//                    builtin->kind() != Builtin::KindObjectProxy)
//                 {
//                     out << nl << typeToString(seq, metaData, TypeContextAMIPrivateEnd) << " ___" << fixedName << ";";
//                 }
//                 else if(!builtin || builtin->kind() != Builtin::KindByte)
//                 {
//                     out << nl << typeToString(seq, metaData) << " ___" << fixedName << ";";
//                 }
            }
        }
        else if(seqType.find("range") == 0)
        {
            StringList md;
            if(seqType.find("range:") == 0)
            {
                md.push_back("cpp:type:" + seqType.substr(strlen("range:")));
            }
            out << nl << typeToString(seq, md) << " ___" << fixedName << ";";
        }
    }
}

}

void
Slice::writeAllocateCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData, 
                         int typeCtx)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamAllocateCode(out, (*p)->type(), fixKwd((*p)->name()), (*p)->getMetaData(), typeCtx);
    }
    if(ret)
    {
        writeParamAllocateCode(out, ret, "__ret", metaData, typeCtx);
    }
}

void
Slice::writeStreamMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal,
                                       const string& str, const StringList& metaData, int typeCtx)
{
    string fixedParam = fixKwd(param);

    string stream;
    if(str.empty())
    {
        stream = marshal ? "__outS" : "__inS";
    }
    else
    {
        stream = str;
    }

    if(marshal)
    {
        out << nl << stream << "->write(" << fixedParam << ");";
    }
    else
    {
        out << nl << stream << "->read(" << fixedParam << ");";
    }
}

string
Slice::getEndArg(const TypePtr& type, const StringList& metaData, const string& arg)
{
    string endArg = arg;
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        bool protobuf;
        string seqType = findMetaData(seq, metaData, protobuf, TypeContextInParam);
        if(!protobuf && seqType.empty())
        {
            seqType = findMetaData(seq, seq->getMetaData(), protobuf, TypeContextInParam);
        }
        if(!protobuf)
        {
            if(seqType == "array" || seqType == "range:array")
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin && 
                   builtin->kind() != Builtin::KindByte &&
                   builtin->kind() != Builtin::KindString &&
                   builtin->kind() != Builtin::KindObject &&
                   builtin->kind() != Builtin::KindObjectProxy)
                {
                    endArg = "___" + endArg;
                }
                else if(!builtin || builtin->kind() != Builtin::KindByte)
                {
                    endArg = "___" + endArg;
                }
            }
            else if(seqType.find("range") == 0)
            {
                StringList md;
                if(seqType.find("range:") == 0)
                {
                    md.push_back("cpp:type:" + seqType.substr(strlen("range:")));
                }
                endArg = "___" + endArg;
            }
        }
    }
    return endArg;
}

namespace
{

void
writeParamEndCode(Output& out, const TypePtr& type, const string& fixedName, const StringList& metaData)
{
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        bool protobuf;
        string seqType = findMetaData(seq, metaData, protobuf, TypeContextInParam);
        if(!protobuf && seqType.empty())
        {
            seqType = findMetaData(seq, seq->getMetaData(), protobuf, TypeContextInParam);
        }
        if(!protobuf)
        {
            if(seqType == "array" || seqType == "range:array")
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin && 
                   builtin->kind() != Builtin::KindByte &&
                   builtin->kind() != Builtin::KindString &&
                   builtin->kind() != Builtin::KindObject &&
                   builtin->kind() != Builtin::KindObjectProxy)
                {
                    out << nl << fixedName << " = ___" << fixedName << ".second;";
                }
                else if(!builtin || builtin->kind() == Builtin::KindString || builtin->kind() == Builtin::KindObject ||
                        builtin->kind() == Builtin::KindObjectProxy)
                {
                    out << nl << fixedName << ".first" << " = &___" << fixedName << "[0];";
                    out << nl << fixedName << ".second" << " = " << fixedName << ".first + " << "___" 
                        << fixedName << ".size();";
                }
            }
            else if(seqType.find("range") == 0)
            {
                out << nl << fixedName << ".first = ___" << fixedName << ".begin();";
                out << nl << fixedName << ".second = ___" << fixedName << ".end();";
            }
        }
    }
}

}

void
Slice::writeEndCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamEndCode(out, (*p)->type(), fixKwd((*p)->name()), (*p)->getMetaData());
    }
    if(ret)
    {
        writeParamEndCode(out, ret, "__ret", metaData);
    }
}

// Accepted metadata.
//
// cpp:type:<typename>
// cpp:const
// cpp:array
// cpp:range:<typename>
// cpp:protobuf<:typename>
//
// For the new AMI mapping, we ignore the array and range directives because they don't apply.
//
// This form is for sequences definitions only.
//
string
Slice::findMetaData(const SequencePtr& seq, const StringList& metaData, bool& isProtobuf, int typeCtx)
{
    isProtobuf = false;
    static const string prefix = "cpp:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if(str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());
            string ss = str.substr(prefix.size());

            //
            // If the form is cpp:type:<...> the data after cpp:type:
            // is returned.  If the form is cpp:range:<...> (and this
            // is an inParam) the data after cpp: is returned.
            //
            if(ss.find("protobuf") == 0 || pos != string::npos)
            {
                if(ss.find("type:") == 0)
                {
                    return str.substr(pos + 1);
                }
                else if(ss.find("protobuf:") == 0)
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                    if(!builtin || builtin->kind() != Builtin::KindByte)
                    {
                        continue;
                    }
                    isProtobuf = true;
                    if(pos != string::npos)
                    {
                        return str.substr(pos + 1);
                    }
                    return "";
                }
                else if((typeCtx & (TypeContextInParam  | TypeContextAMIPrivateEnd)) &&
                        !(typeCtx & TypeContextAMIEnd) && ss.find("range:") == 0)
                {
                    return str.substr(prefix.size());
                }
                else if((typeCtx & TypeContextAMIPrivateEnd) && ss == "range:array")
                {
                    return str.substr(prefix.size());
                }
            }
            //
            // If the data is an inParam and the metadata is cpp:array
            // or cpp:range then array or range is returned.
            //
            else if(typeCtx & (TypeContextInParam | TypeContextAMIPrivateEnd) && !(typeCtx & TypeContextAMIEnd))
            {
                if(ss == "array")
                {
                    return ss;
                }
                else if((typeCtx & TypeContextInParam) && ss == "range")
                {
                    return ss;
                }
            }
            //
            // Otherwise if the data is "class" it is returned.
            //
            else
            {
                if(ss == "class")
                {
                    return ss;
                }
            }
        }
    }

    return "";
}

// Does not handle cpp:protobuf
string
Slice::findMetaData(const StringList& metaData, int typeCtx)
{
    static const string prefix = "cpp:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if(str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());
            //
            // If the form is cpp:type:<...> the data after cpp:type:
            // is returned.  If the form is cpp:range:<...> the data
            // after cpp: is returned.
            //
            if(pos != string::npos)
            {
                string ss = str.substr(prefix.size());
                if(ss.find("type:") == 0)
                {
                    return str.substr(pos + 1);
                }
                else if((typeCtx & (TypeContextInParam  | TypeContextAMIPrivateEnd)) && 
                        !(typeCtx & TypeContextAMIEnd) && ss.find("range:") == 0)
                {
                    return str.substr(prefix.size());
                }
                else if((typeCtx & TypeContextAMIPrivateEnd) && ss == "range:array")
                {
                    return str.substr(prefix.size());
                }
            }
            else if(typeCtx & (TypeContextInParam | TypeContextAMIPrivateEnd) && !(typeCtx & TypeContextAMIEnd))
            {
                string ss = str.substr(prefix.size());
                if(ss == "array")
                {
                    return ss;
                }
                else if((typeCtx & TypeContextInParam) && ss == "range")
                {
                    return ss;
                }
            }
            //
            // Otherwise if the data is "class" it is returned.
            //
            else
            {
                string ss = str.substr(prefix.size());
                if(ss == "class")
                {
                    return ss;
                }
            }
        }
    }

    return "";
}

bool
Slice::inWstringModule(const SequencePtr& seq)
{
    ContainerPtr cont = seq->container();
    while(cont)
    {
        ModulePtr mod = ModulePtr::dynamicCast(cont);
        if(!mod)
        {
            break;
        }
        StringList metaData = mod->getMetaData();
        if(find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
        {
            return true;
        }
        else if(find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
        {
            return false;
        }
        cont = mod->container();
    }
    return false;
}
