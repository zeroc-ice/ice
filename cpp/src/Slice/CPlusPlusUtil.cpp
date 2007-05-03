// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/CPlusPlusUtil.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

Slice::FeatureProfile Slice::featureProfile = Slice::Ice;

char
Slice::ToIfdef::operator()(char c)
{
    if(!isalnum(c))
    {
        return '_';
    }
    else
    {
        return c;
    }
}

string
Slice::normalizePath(const string& path)
{
    string result = path;
    replace(result.begin(), result.end(), '\\', '/');
    string::size_type pos;
    while((pos = result.find("//")) != string::npos)
    {
        result.replace(pos, 2, "/");
    }
    if(result.size() > 1 && isalpha(result[0]) && result[1] == ':')
    {
        result = result.substr(2);
    }
    return result;
}

string
Slice::changeInclude(const string& orig, const vector<string>& includePaths)
{
    string file = normalizePath(orig);
    string::size_type pos;

    //
    // Compare each include path against the included file and select
    // the path that produces the shortest relative filename.
    //
    string result = file;
    for(vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
        string includePath = normalizePath(*p);

        if(file.compare(0, includePath.length(), includePath) == 0)
        {
            string s = file.substr(includePath.length());
            if(s.size() < result.size())
            {
                result = s;
            }
        }
    }

    if((pos = result.rfind('.')) != string::npos)
    {
        result.erase(pos);
    }

    return result;
}

void
Slice::printHeader(Output& out)
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    out << header;
    out << "\n// Ice version " << ICE_STRING_VERSION;
}

void
Slice::printVersionCheck(Output& out)
{
    out << "\n";
    out << "\n#ifndef ICE_IGNORE_VERSION";
    if(ICE_INT_VERSION % 100 > 50)
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

string
Slice::typeToString(const TypePtr& type, bool useWstring, const StringList& metaData, bool inParam)
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
            string strType = findMetaData(metaData, true);
            if(strType != "string" && (useWstring || strType == "wstring"))
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
        if(findMetaData(st->getMetaData(), false) == "class")
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
        string seqType = findMetaData(metaData, true);
        if(!seqType.empty())
        {
            if(seqType == "array" || seqType == "range:array")
            {
                if(inParam)
                {
                    TypePtr elemType = seq->type();
                    string s = typeToString(elemType, inWstringModule(seq), seq->typeMetaData());
                    return "::std::pair<const " + s + "*, const " + s + "*>";
                }
                else
                {
                    return fixKwd(seq->scoped());
                }
            }
            else if(seqType.find("range") == 0)
            {
                if(inParam)
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
                    if(s[0] == ':')
                    {
                        s = " " + s;
                    }
                    return "::std::pair<" + s + "::const_iterator, " + s + "::const_iterator>";
                }
                else
                {
                    return fixKwd(seq->scoped());
                }
            }
            else
            {
                return seqType;
            }
        }
        else
        {
            return fixKwd(seq->scoped());
        }
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
Slice::returnTypeToString(const TypePtr& type, bool useWstring, const StringList& metaData)
{
    if(!type)
    {
        return "void";
    }

    return typeToString(type, useWstring, metaData, false);
}

string
Slice::inputTypeToString(const TypePtr& type, bool useWstring, const StringList& metaData)
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

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            string strType = findMetaData(metaData, true);
            if(strType != "string" && (useWstring || strType == "wstring"))
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
        return "const " + fixKwd(cl->scoped()) + "Ptr&";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData(), false) == "class")
        {
            return "const " + fixKwd(st->scoped()) + "Ptr&";
        }
        return "const " + fixKwd(st->scoped()) + "&";
    }
            
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return "const " + fixKwd(proxy->_class()->scoped()) + "Prx&";
    }
            
    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixKwd(en->scoped());
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, true);
        if(!seqType.empty())
        {
            if(seqType == "array" || seqType == "range:array")
            {
                TypePtr elemType = seq->type();
                string s = typeToString(elemType, inWstringModule(seq), seq->typeMetaData());
                return "const ::std::pair<const " + s + "*, const " + s + "*>&";
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
                if(s[0] == ':')
                {
                    s = " " + s;
                }
                return "const ::std::pair<" + s + "::const_iterator, " + s + "::const_iterator>&";
            }
            else
            {
                return "const " + seqType + "&";
            }
        }
        else
        {
            return "const " + fixKwd(seq->scoped()) + "&";
        }
    }
            
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return "const " + fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

string
Slice::outputTypeToString(const TypePtr& type, bool useWstring, const StringList& metaData)
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
            string strType = findMetaData(metaData, true);
            if(strType != "string" && (useWstring || strType == "wstring"))
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
        return fixKwd(cl->scoped()) + "Ptr&";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData(), false) == "class")
        {
            return fixKwd(st->scoped()) + "Ptr&";
        }
        return fixKwd(st->scoped()) + "&";
    }
            
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixKwd(proxy->_class()->scoped()) + "Prx&";
    }
            
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, false);
        if(!seqType.empty())
        {
            return seqType + "&";
        }
        else
        {
            return fixKwd(seq->scoped()) + "&";
        }
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
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal,
                                 const string& str, bool pointer, const StringList& metaData, bool inParam)
{
    string fixedParam = fixKwd(param);

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
            out << nl << stream << deref << "write(::Ice::ObjectPtr(::IceInternal::upCast(" << fixedParam 
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
            out << nl << stream << deref << "write(::Ice::ObjectPrx(::IceInternal::upCast(" << fixedParam 
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
        if(findMetaData(st->getMetaData(), false) == "class")
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
        string seqType = findMetaData(metaData, inParam);
        builtin = BuiltinPtr::dynamicCast(seq->type());
        string funcSeq = (marshal ? "write" : "read") + fixKwd(seq->name()) + "(";

        if(marshal)
        {
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
                    StringList l = seq->getMetaData();
                    seqType = findMetaData(l, false);
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
                out << nl << stream << deref << "writeSize(static_cast< ::Ice::Int>(ice_distance(" 
                    << fixedParam << ".first, " << fixedParam << ".second)));"; 
                out << nl << "for(" << seqType << "::const_iterator ___" << fixedParam << " = "
                    << fixedParam << ".first; ___" << fixedParam << " != " << fixedParam << ".second; ++___"
                    << fixedParam << ")";
                out << sb;
                writeMarshalUnmarshalCode(out, seq->type(), "(*___" + fixedParam + ")", true, "", true, l, false);
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
                StringList l = seq->getMetaData();
                seqType = findMetaData(l, false);
                if(!seqType.empty())
                {
                    out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", " << fixedParam 
                        << ");";
                }
                else if(!builtin || builtin->kind() == Builtin::KindObject ||
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
            string scope = fixKwd(seq->scope());
            if(seqType == "array" || seqType == "range:array")
            {
                //
                // Use array (pair<const TYPE*, const TYPE*>). In paramters only.
                //
                if(!builtin || builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy)
                {
                    StringList l = seq->getMetaData();
                    seqType = findMetaData(l, false);
                    if(seqType.empty())
                    {
                        out << nl << typeToString(type, false) << " ___" << fixedParam << ";";
                        out << nl << scope << "__" << funcSeq << (pointer ? "" : "&") << stream << ", ___"
                            << fixedParam << ");";
                    }
                    else
                    {
                        seqType = "::std::vector< " + typeToString(seq->type(), false) + ">";
                        StringList l;
                        l.push_back("cpp:type:" + seqType);
                        out << nl << seqType << " ___" << fixedParam << ";";
                        writeMarshalUnmarshalCode(out, seq, "___" + fixedParam, false, "", true, l, false);
                    }
                }
                else if(builtin->kind() == Builtin::KindByte)
                {
                    out << nl << stream << deref << func << fixedParam << ");";
                }
                else if(builtin->kind() != Builtin::KindString && builtin->kind() != Builtin::KindObject &&
                        builtin->kind() != Builtin::KindObjectProxy)
                {
                    string s = typeToString(builtin, false);
                    if(s[0] == ':')
                    {
                       s = " " + s;
                    }
                    out << nl << "::IceUtil::ScopedArray<" << s << "> ___" << fixedParam << '('
                        << stream << deref << func << fixedParam << "));";
                }
                else
                {
                    out << nl << "::std::vector< " 
                        << typeToString(seq->type(), inWstringModule(seq), seq->typeMetaData()) << "> ___" 
                        << fixedParam << ";";
                    out << nl << stream << deref << func << "___" << fixedParam << ");";
                }

                if(!builtin || builtin->kind() == Builtin::KindString || builtin->kind() == Builtin::KindObject ||
                   builtin->kind() == Builtin::KindObjectProxy)
                {
                    out << nl << fixedParam << ".first" << " = &___" << fixedParam << "[0];";
                    out << nl << fixedParam << ".second" << " = " << fixedParam << ".first + " << "___" 
                        << fixedParam << ".size();";
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
                writeMarshalUnmarshalCode(out, seq, "___" + fixedParam, false, "", true, md, false);
                out << nl << fixedParam << ".first = ___" << fixedParam << ".begin();";
                out << nl << fixedParam << ".second = ___" << fixedParam << ".end();";
            }
            else if(!seqType.empty())
            {
                //
                // Using alternate sequence type. 
                //
                out << nl << "::Ice::Int __sz_" << fixedParam << ";";
                out << nl << stream << deref << "readSize(__sz_" << fixedParam << ");";
                out << nl << seqType << "(__sz_" << fixedParam << ").swap(" << fixedParam << ");";
                if(seq->type()->isVariableLength())
                {
                    out << nl << stream << deref << "startSeq(__sz_" << fixedParam << ", " 
                        << seq->type()->minWireSize() << ");";
                }
                else
                {
                    out << nl << stream << deref << "checkFixedSeq(__sz_" << fixedParam << ", " 
                        << seq->type()->minWireSize() << ");";
                }
                out << nl << seqType << "::iterator __p_" << fixedParam << ";";
                out << nl << "for(__p_" << fixedParam << " = " << fixedParam << ".begin(); __p_" << fixedParam 
                   << " != " << fixedParam << ".end(); ++__p_" << fixedParam << ")";
                out << sb;
                writeMarshalUnmarshalCode(out, seq->type(), "(*__p_" + fixedParam + ")", false);
                if(seq->type()->isVariableLength())
                {
                    if(!SequencePtr::dynamicCast(seq->type()))
                    {
                        out << nl << stream << deref << "checkSeq();";
                    }
                    out << nl << stream << deref << "endElement();";
                }
                out << eb;
                if(seq->type()->isVariableLength())
                {
                    out << nl << stream << deref << "endSeq(__sz_" << fixedParam << ");";
                }
            }
            else
            {
                //
                // No modifying metadata supplied. Just use appropriate read function.
                //
                StringList l = seq->getMetaData();
                seqType = findMetaData(l, false);
                if(!seqType.empty() || !builtin || builtin->kind() == Builtin::KindObject ||
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
                    out << nl << typeToString(type, false, md) << " " << tmpParam << ";";
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
                        bool inParam)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeMarshalUnmarshalCode(out, (*p)->type(), fixKwd((*p)->name()), true, "", true, (*p)->getMetaData(),
                                  inParam);
    }
    if(ret)
    {
        writeMarshalUnmarshalCode(out, ret, "__ret", true, "", true, metaData, false);
    }
}

void
Slice::writeUnmarshalCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData,
                          bool inParam)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeMarshalUnmarshalCode(out, (*p)->type(), fixKwd((*p)->name()), false, "", true, (*p)->getMetaData(),
                                  inParam);
    }
    if(ret)
    {
        writeMarshalUnmarshalCode(out, ret, "__ret", false, "", true, metaData, false);
    }
}

static void
writeRangeAllocateCode(Output& out, const TypePtr& type, const string& fixedName, const StringList& metaData,
                              bool inParam)
{
    if(!inParam)
    {
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, true);
        if(seqType.find("range") == 0 && seqType != "range:array")
        {
            StringList md;
            if(seqType.find("range:") == 0)
            {
                md.push_back("cpp:type:" + seqType.substr(strlen("range:")));
            }
            out << nl << typeToString(seq, false, md, false) << " ___" << fixedName << ";";
        }
    }
}

void
Slice::writeAllocateCode(Output& out, const ParamDeclList& params, const TypePtr& ret, const StringList& metaData,
                         bool useWstring, bool inParam)
{
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        out << nl << typeToString((*p)->type(), useWstring, (*p)->getMetaData(), inParam) << ' '
            << fixKwd((*p)->name());
        StructPtr st = StructPtr::dynamicCast((*p)->type());
        if(st && findMetaData(st->getMetaData(), false) == "class")
        {
            out << " = new " << fixKwd(st->scoped());
        }
        out << ';';

        //
        // If using a range we need to allocate the range container as well now to ensure they
        // are always in the same scope.
        //
        writeRangeAllocateCode(out, (*p)->type(), fixKwd((*p)->name()), (*p)->getMetaData(), inParam);
    }

    if(ret)
    {
        out << nl << typeToString(ret, useWstring, metaData, inParam) << " __ret";
        StructPtr st = StructPtr::dynamicCast(ret);
        if(st && findMetaData(st->getMetaData(), false) == "class")
        {
            out << " = new " << fixKwd(st->scoped());
        }
        out << ";";

        //
        // If using a range we need to allocate the range container as well now to ensure they
        // are always in the same scope.
        //
        writeRangeAllocateCode(out, ret, "__ret", metaData, inParam);
    }
}

void
Slice::writeStreamMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal,
                                       const string& str, bool useWstring, const StringList& metaData)
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

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeByte(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeBool(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeShort(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeInt(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeLong(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeFloat(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << "->writeDouble(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                string strType = findMetaData(metaData, true);
                if(strType != "string" && (useWstring || strType == "wstring"))
                {
                    if(marshal)
                    {
                        out << nl << stream << "->writeWstring(" << fixedParam << ");";
                    }
                    else
                    {
                        out << nl << fixedParam << " = " << stream << "->readWstring();";
                    }
                }
                else
                {
                    if(marshal)
                    {
                        out << nl << stream << "->writeString(" << fixedParam << ");";
                    }
                    else
                    {
                        out << nl << fixedParam << " = " << stream << "->readString();";
                    }
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << "::Ice::ice_writeObject(" << stream << ", " << fixedParam << ");";
                }
                else
                {
                    out << nl << "::Ice::ice_readObject(" << stream << ", " << fixedParam << ");";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                // TODO
                if(marshal)
                {
                    out << nl << stream << "->writeProxy(" << fixedParam << ");";
                }
                else
                {
                    out << nl << fixedParam << " = " << stream << "->readProxy();";
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
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        string scope = fixKwd(cl->scope());
        if(marshal)
        {
            out << nl << scope << "ice_write" << cl->name() << "(" << stream << ", " << fixedParam << ");";
        }
        else
        {
            out << nl << scope << "ice_read" << cl->name() << "(" << stream << ", " << fixedParam << ");";
        }

        return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        string scope = fixKwd(st->scope());
        if(marshal)
        {
            out << nl << scope << "ice_write" << st->name() << "(" << stream << ", " << fixedParam << ");";
        }
        else
        {
            if(findMetaData(st->getMetaData(), false) == "class")
            {
                out << nl << fixedParam << " = new " << fixKwd(st->scoped()) << ";";
            }
            out << nl << scope << "ice_read" << st->name() << "(" << stream << ", " << fixedParam << ");";
        }

        return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, false);
        if(!seqType.empty())
        {
            if(marshal)
            {
                out << nl << stream << "->writeSize(static_cast< ::Ice::Int>(" << fixedParam << ".size()));";
                out << nl << seqType << "::const_iterator ___" << fixedParam << ";";
                out << nl << "for(___" << fixedParam << " = " << fixedParam << ".begin(); ___" << fixedParam << " != "
                    << fixedParam << ".end(); ++___" << fixedParam << ")";
                out << sb;
                writeStreamMarshalUnmarshalCode(out, seq->type(), "(*___" + fixedParam + ")", true);
                out << eb;
            }
            else
            {
                out << nl << seqType << "(static_cast< ::Ice::Int>(" << stream << "->readSize())).swap("
                    << fixedParam << ");";
                out << nl << seqType << "::iterator ___" << fixedParam << ";";
                out << nl << "for(___" << fixedParam << " = " << fixedParam << ".begin(); ___" << fixedParam << " != "
                    << fixedParam << ".end(); ++___" << fixedParam << ")";
                out << sb;
                writeStreamMarshalUnmarshalCode(out, seq->type(), "(*___" + fixedParam + ")", false);
                out << eb;
            }
        }
        else
        {
            seqType = findMetaData(seq->getMetaData(), false);
            builtin = BuiltinPtr::dynamicCast(seq->type());
            if(!seqType.empty() || !builtin || (builtin->kind() == Builtin::KindObject || 
               builtin->kind() == Builtin::KindObjectProxy))
            {
                string scope = fixKwd(seq->scope());
                if(marshal)
                {
                    out << nl << scope << "ice_write" << seq->name() << '(' << stream << ", " << fixedParam << ");";
                }
                else
                {
                    out << nl << scope << "ice_read" << seq->name() << '(' << stream << ", " << fixedParam << ");";
                }
            }
            else
            {
                switch(builtin->kind())
                {
                    case Builtin::KindByte:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeByteSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readByteSeq();";
                        }
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeBoolSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readBoolSeq();";
                        }
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeShortSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readShortSeq();";
                        }
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeIntSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readIntSeq();";
                        }
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeLongSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readLongSeq();";
                        }
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeFloatSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readFloatSeq();";
                        }
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        if(marshal)
                        {
                            out << nl << stream << "->writeDoubleSeq(" << fixedParam << ");";
                        }
                        else
                        {
                            out << nl << fixedParam << " = " << stream << "->readDoubleSeq();";
                        }
                        break;
                    }
                    case Builtin::KindString:
                    {
                        string strType = findMetaData(seq->typeMetaData(), true);
                        if(strType != "string" && (useWstring || strType == "wstring"))
                        {
                            if(marshal)
                            {
                                out << nl << stream << "->writeWstringSeq(" << fixedParam << ");";
                            }
                            else
                            {
                                out << nl << fixedParam << " = " << stream << "->readWstringSeq();";
                            }
                        }
                        else
                        {
                            if(marshal)
                            {
                                out << nl << stream << "->writeStringSeq(" << fixedParam << ");";
                            }
                            else
                            {
                                out << nl << fixedParam << " = " << stream << "->readStringSeq();";
                            }
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
        }

        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        string scope = fixKwd(dict->scope());
        if(marshal)
        {
            out << nl << scope << "ice_write" << dict->name() << "(" << stream << ", " << fixedParam << ");";
        }
        else
        {
            out << nl << scope << "ice_read" << dict->name() << "(" << stream << ", " << fixedParam << ");";
        }

        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        string scope = fixKwd(en->scope());
        if(marshal)
        {
            out << nl << scope << "ice_write" << en->name() << "(" << stream << ", " << fixedParam << ");";
        }
        else
        {
            out << nl << scope << "ice_read" << en->name() << "(" << stream << ", " << fixedParam << ");";
        }

        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        ClassDeclPtr cls = prx->_class();
        string scope = fixKwd(cls->scope());
        if(marshal)
        {
            out << nl << scope << "ice_write" << cls->name() << "Prx(" << stream << ", " << fixedParam << ");";
        }
        else
        {
            out << nl << scope << "ice_read" << cls->name() << "Prx(" << stream << ", " << fixedParam << ");";
        }

        return;
    }
    
    assert(false);
}

string
Slice::findMetaData(const StringList& metaData, bool inParam)
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
            // is returned.  If the form is cpp:range:<...> (and this
            // is an inParam) the data after cpp: is returned.
            //
            if(pos != string::npos)
            {
                string ss = str.substr(prefix.size(), pos - prefix.size());
                if(ss == "type")
                {
                    return str.substr(pos + 1);
                }
                else if(inParam && ss == "range")
                {
                    return str.substr(prefix.size());
                }
            }
            //
            // If the data is an inParam and the metadata is cpp:array
            // or cpp:range then array or range is returned.
            //
            else if(inParam)
            {
                string ss = str.substr(prefix.size());
                if(ss == "array" || ss == "range")
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
